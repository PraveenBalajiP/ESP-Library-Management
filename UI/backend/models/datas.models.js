import mongoose from "mongoose"

const userSchema=new mongoose.Schema({
    id:{
        type:String,
        required:true
    },
    bookName:{
        type:String,
        required:true
    },
    dateIssued:{
        type:String,
        required:true
    },
    lastDate:{
        type:String,
        required:true
    }
},{timestamps:true});

const Data=mongoose.model("data",userSchema);

export default Data;