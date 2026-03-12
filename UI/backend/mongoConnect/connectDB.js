import mongoose from 'mongoose';
import dotenv from 'dotenv';

dotenv.config();

const connectDB=async ()=>{
    try{
        const connection=await mongoose.connect(process.env.MONGO_URI);
        console.log(`MongoDB connected successfully: ${connection.connection.host}`);
    } catch (error) {
        console.error('Error connecting to MongoDB:', error);
    }
}

export default connectDB;