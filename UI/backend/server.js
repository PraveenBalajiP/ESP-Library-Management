import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import cookieParser from 'cookie-parser';
import connectDB from './mongoConnect/connectDB.js';
import Data from './models/datas.models.js';
import jwt from 'jsonwebtoken';

dotenv.config();
const PORT=process.env.PORT || 5000;

const app=express();
const allowedOrigins = [
    "http://localhost:3000",
    "http://localhost:5173",
    "http://localhost:5000",
    process.env.FRONTEND_URL || ""
].filter(Boolean);

const isLocalOrigin=(origin)=>/^https?:\/\/(localhost|127\.0\.0\.1)(:\d+)?$/i.test(origin);

app.use(cors({
    origin: (origin, callback) => {
        if (!origin || allowedOrigins.includes(origin) || isLocalOrigin(origin) || origin.includes("vercel.app")) {
            callback(null, true);
        } else {
            callback(new Error("CORS not allowed"));
        }
    },
    methods:["GET","POST","OPTIONS"],
    credentials: true
}));
app.use(express.json());
app.use(cookieParser());

// Initialize database before starting server
await connectDB();

const JWT_SECRET = process.env.JWT_SECRET || 'change_this_secret';

function generateToken(user, isAdmin=false){
    return jwt.sign({user,isAdmin}, JWT_SECRET, {expiresIn: '8h'});
}

function authenticateToken(req, res, next){
    const authHeader = req.headers['authorization'] || req.headers['Authorization'];
    let token = authHeader && authHeader.split(' ')[1];
    // if no Authorization header, try cookie
    if(!token && req.cookies && req.cookies.token){
        token = req.cookies.token;
    }
    if(!token) return res.status(401).json({message: 'No token provided'});
    try{
        const payload = jwt.verify(token, JWT_SECRET);
        req.user = payload;
        next();
    }catch(err){
        return res.status(403).json({message: 'Invalid or expired token'});
    }
}

let popup=false;
let popupData=null;
let peopleInLibrary=0;
const SUBJECT_STOCK=5;

app.get("/",(req,res)=>{
    res.send("Library Management Backend is running");
});

app.post("/api/addinfo", authenticateToken, async (req,res)=>{
    const {id,bookName}=req.body;
    const dateIssued=new Date();
    const lastDate=new Date(dateIssued);
    lastDate.setDate(lastDate.getDate()+15);
    try{
        const alreadyExist=await Data.findOne({id,bookName});
        if(alreadyExist){
            popup=true;
            popupData={
                id:alreadyExist.id,
                bookName:alreadyExist.bookName,
                dateIssued:alreadyExist.dateIssued,
                lastDate:alreadyExist.lastDate
            };
            return res.status(200).json({
                duplicate:true
            });
        }
        const subjectIssuedCount=await Data.countDocuments({bookName});
        if(subjectIssuedCount>=SUBJECT_STOCK){
            return res.status(400).json({
                message:`No books left in inventory for ${bookName}`,
                inventoryAvailable:0
            });
        }
        const newData=new Data({
            id,
            bookName,
            dateIssued,
            lastDate
        });
        console.log("NEW ENTRY:",newData);
        await newData.save();
        return res.status(200).json({
            duplicate:false,
            message:"Data added successfully"
        });
    }
    catch(error){
        return res.status(500).json({
            message:`Error adding data, ${error}`
        });
    }
});

app.get("/api/addinfo",(req,res)=>{
    const current=popup;
    const data=popupData;
    popup=false;
    popupData=null;
    res.json({popup:current,...(data||{})});
});

app.post("/api/checkout", authenticateToken, async (req,res)=>{
    const {id,bookName}=req.body;
    try{
        const exist=await Data.findOne({id:id,bookName:bookName});
       if(!exist){
            return res.status(200).json({
            valid:false,
            message:"Book was not issued"
        });
    }
    // Actually delete the book record on checkout
    await Data.deleteOne({id:id,bookName:bookName});
    return res.status(200).json({
        valid:true,
        message:"Checkout successful"
    });
    }
    catch(error){
        res.status(500).json({message:`Error checking out data, ${error}`});
    }
})

app.post("/api/login", async (req,res)=>{
    const {user} = req.body;
    try{
        const isAdmin = user === "pesu@123";
        let sendData;
        if(isAdmin){
            sendData = {
                data: await Data.find(),
                message: "Login Successful"
            };
        } else {
            sendData = {
                data: await Data.find({id: user}),
                message: "Login Successful"
            };
        }

        const token = generateToken(user, isAdmin);
        // set cookie
        res.cookie('token', token, {
            httpOnly: true,
            secure: process.env.NODE_ENV === 'production',
            sameSite: 'lax',
            maxAge: 8 * 60 * 60 * 1000 // 8 hours
        });

        return res.status(200).json({...sendData});
    }
    catch(err){
        return res.status(500).json({message:`Login error: ${err}`});
    }
})

// Auth check endpoint (used by frontend to verify cookie/session)
app.get('/api/auth-check', authenticateToken, async (req,res)=>{
    try{
        const user = req.user?.user || null;
        const isAdmin = req.user?.isAdmin || false;
        if(isAdmin){
            return res.status(200).json({authenticated:true, isAdmin:true, data: await Data.find()});
        }
        return res.status(200).json({authenticated:true, isAdmin:false, data: await Data.find({id:user})});
    }
    catch(err){
        return res.status(500).json({message:`Auth check error: ${err}`});
    }
});

// Logout clears the cookie
app.post('/api/logout', (req,res)=>{
    res.clearCookie('token', { httpOnly: true, sameSite: 'lax' });
    return res.status(200).json({message: 'Logged out'});
});

app.get("/api/stats",async (req,res)=>{
    try{
        const TOTAL_BOOKS=Number(process.env.TOTAL_BOOKS || 6);
        const issuedDocs=await Data.find({},"lastDate");
        const issuedCount=issuedDocs.length;
        const availableBooks=Math.max(TOTAL_BOOKS-issuedCount,0);
        res.status(200).json({
            issuedCount,
            availableBooks,
            peopleInLibrary,
            totalBooks:TOTAL_BOOKS
        });
    }
    catch(error){
        res.status(500).json({message:`Error fetching stats, ${error}`});
    }
})

app.get("/api/inventory",async (req,res)=>{
    try{
        const issuedBySubject=await Data.aggregate([
            {
                $group:{
                    _id:"$bookName",
                    issuedCount:{$sum:1}
                }
            },
            {
                $project:{
                    _id:0,
                    subject:"$_id",
                    issuedCount:1
                }
            },
            {
                $sort:{subject:1}
            }
        ]);

        const inventory=issuedBySubject.map((item)=>({
            subject:item.subject,
            totalBooks:SUBJECT_STOCK,
            issuedCount:item.issuedCount,
            availableBooks:Math.max(SUBJECT_STOCK-item.issuedCount,0)
        }));

        return res.status(200).json({
            inventory,
            subjectStock:SUBJECT_STOCK
        });
    }
    catch(error){
        return res.status(500).json({message:`Error fetching inventory, ${error}`});
    }
})

app.post("/api/people-count",(req,res)=>{
    const {count}=req.body;
    const numericCount=Number(count);
    if(Number.isNaN(numericCount) || numericCount<0){
        return res.status(400).json({message:"Invalid people count"});
    }
    peopleInLibrary=Math.floor(numericCount);
    return res.status(200).json({
        message:"People count updated",
        peopleInLibrary
    });
})

app.post("/api/renew", authenticateToken, async (req,res)=>{
    const {id,bookName}=req.body;
    try{
        const exist=await Data.findOne({id:id,bookName:bookName});
        if(!exist){
            res.status(400).json({message:"⚠ Alert ⚠ Data does not exist"});
            return;
        }
        else if(exist){
            const newLastDate=new Date(exist.lastDate);
            newLastDate.setDate(newLastDate.getDate()+15);
            await Data.updateOne({id:id,bookName:bookName},{$set:{lastDate:newLastDate}});
            res.status(200).json({message:"Book renewed successfully"});
        }
    }
    catch(error){
        res.status(500).json({message:`Error renewing book, ${error}`});
    }
})

app.post("/api/withdraw", authenticateToken, async (req,res)=>{
    const {id,bookName}=req.body;
    try{
        const exist=await Data.findOne({id:id,bookName:bookName});
        if(!exist){
            res.status(400).json({message:"⚠ Alert ⚠ Data does not exist"});
            return;
        }
        else if(exist){
            await Data.deleteOne({id:id,bookName:bookName});
            res.status(200).json({message:"Book withdrawn successfully"});
        }
    }
    catch(error){
        res.status(500).json({message:`Error withdrawing book, ${error}`});
    }
})

app.post("/api/exit-verify", authenticateToken, async (req,res)=>{
    const {id,bookName}=req.body;
    try{
        const exist=await Data.findOne({id:id,bookName:bookName});
        if(!exist){
            return res.status(400).json({
                valid:false,
                message:"Theft detected: book was not issued"
            });
        }
        return res.status(200).json({
            valid:true,
            message:"Exit verified: no theft detected"
        });
    }
    catch(error){
        return res.status(500).json({message:`Error verifying exit, ${error}`});
    }
})

if(process.env.VERCEL !== "1"){
    app.listen(PORT,"0.0.0.0",()=>{
        console.log(`Server is running on port ${PORT}`);
    })
}

export default app;