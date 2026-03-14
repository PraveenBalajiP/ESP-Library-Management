import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import connectDB from './mongoConnect/connectDB.js';
import Data from './models/datas.models.js';

dotenv.config();
const PORT=process.env.PORT || 5000;

const app=express();
app.use(cors());
app.use(express.json());

let popup=false;
let popupData=null;

app.post("/api/addinfo",async (req,res)=>{
    const {id,bookName}=req.body;
    const dateIssued=new Date();
    const lastDate=new Date(dateIssued);
    lastDate.setDate(lastDate.getDate()+15);

    const alreadyExist=await Data.findOne({id:id,bookName:bookName});
    if(alreadyExist){
        popup=true;
        popupData={
            id:alreadyExist.id,
            bookName:alreadyExist.bookName,
            dateIssued:alreadyExist.dateIssued,
            lastDate:alreadyExist.lastDate
        };
        res.status(400).json({message:"Data already exists"});
        return;
    }
    try{
        const newData=new Data({
            id,
            bookName,
            dateIssued,
            lastDate
        })
        console.log(newData);
        await newData.save();
        res.status(200).json({message:"Data added successfully"});
    } 
    catch(error){
        res.status(500).json({message:`Error adding data, ${error}`});
    }
})

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
            res.status(400).json({message:"⚠ Alert ⚠ Data does not exist"});
            return;
        }
        else if(exist){
            return res.status(200).json({message:"Data checked out successfully"});
        }
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

app.listen(PORT,()=>{
    connectDB();
    console.log(`Server is running on port ${PORT}`);
})