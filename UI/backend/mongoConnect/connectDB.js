import mongoose from 'mongoose';
import dotenv from 'dotenv';

dotenv.config();

const connectDB=async ()=>{
    try{
        if(!process.env.MONGO_URI){
            throw new Error('MONGO_URI is not defined in environment variables');
        }
        const connection=await mongoose.connect(process.env.MONGO_URI);
        console.log(`MongoDB connected successfully: ${connection.connection.host}`);
    } catch (error) {
        console.error('Error connecting to MongoDB:', error.message);
        process.exit(1);
    }
}

export default connectDB;