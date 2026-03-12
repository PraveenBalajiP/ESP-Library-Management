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

app.post("/api/addinfo",async (req,res)=>{
    const {id,bookName}=req.body;
    const dateIssued=new Date();
    const lastDate=new Date(dateIssued);
    lastDate.setDate(lastDate.getDate()+15);

    const alreadyExist=await Data.findOne({id:id,bookName:bookName});
    if(alreadyExist){
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

app.listen(PORT,()=>{
    connectDB();
    console.log(`Server is running on port ${PORT}`);
})