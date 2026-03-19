import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import connectDB from './mongoConnect/connectDB.js';
import Data from './models/datas.models.js';

dotenv.config();
const PORT=process.env.PORT || 5000;

const app=express();
const allowedOrigins = [
    "http://localhost:3000",
    "http://localhost:5173",
    "http://localhost:5000",
    process.env.FRONTEND_URL || ""
].filter(Boolean);

app.use(cors({
    origin: (origin, callback) => {
        if (!origin || allowedOrigins.includes(origin) || origin.includes("vercel.app")) {
            callback(null, true);
        } else {
            callback(new Error("CORS not allowed"));
        }
    },
    methods:["GET","POST"]
}));
app.use(express.json());

connectDB();

let popup=false;
let popupData=null;
let peopleInLibrary=0;

app.get("/",(req,res)=>{
    res.send("Library Management Backend is running");
});

app.post("/api/addinfo",async (req,res)=>{
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

app.post("/api/checkout",async (req,res)=>{
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

app.post("/api/login",async (req,res)=>{
    const {user}=req.body;
    if(user==="pesu@123"){
        const sendData={
            data:await Data.find(),
            message:"Login Successful"
        }
        res.status(200).json(sendData);
    }
    else{
        const sendData={
            data:await Data.find({id:user}),
            message:"Login Successful"
        }
        res.status(200).json(sendData);
    }  
})

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

app.post("/api/renew",async (req,res)=>{
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

app.post("/api/withdraw",async (req,res)=>{
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

app.post("/api/exit-verify",async (req,res)=>{
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