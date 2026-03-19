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
        type:Date,
        required:true,
        default:Date.now
    },
    lastDate:{
        type:Date,
        required:true
    }
},{timestamps:true});

// Add unique index to prevent duplicate issues
userSchema.index({ id: 1, bookName: 1 }, { unique: true });

const Data=mongoose.model("data",userSchema);

export default Data;