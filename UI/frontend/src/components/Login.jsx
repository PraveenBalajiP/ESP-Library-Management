import {useState,useEffect} from "react";
import { toast } from "react-toastify";
import {useNavigate} from "react-router-dom";
import axios from "axios";
import "../css/login.css";

function Login(){
    const [placeholder,setPlaceholder]=useState("Enter your SRN..");
    const navigate=useNavigate();
    const [input,setInput]=useState();

    async function handleSubmit(event){
        const data={
            user:input
        };
        try{
            const response=await axios.post("/api/login",data);
            toast.success(`${response.data.message}`,{className:"toast-success"});
            navigate("/user",{state:{data:response.data.data}});
        }
        catch(error){
            toast.error(`${error.response.data.message}`,{className:"toast-error"});
        }
    }

    return(
        <div className="login">
            <div className="login-card">
                <h2>Member Sign In</h2>
                <p>Access issue history, return reminders, and account activity.</p>
                <form onSubmit={(event)=>{event.preventDefault()}}>
                    <p>Whose Logging In?</p>
                    <select onChange={(e)=>setPlaceholder(e.target.value==="student"?"Enter your SRN..":"Enter your Username")}>
                        <option value="student">Student</option>
                        <option value="librarian">Librarian</option>
                    </select>
                    <select onChange={(e)=>setInput(e.target.value)}>
                        <option value="">Select User</option>
                        <option value="PES2UG24CS001">PES2UG24CS001</option>
                        <option value="PES24G24CS002">PES2UG24CS002</option>
                    </select>
                    <button type="submit" onClick={handleSubmit}>Login</button>
                </form>
            </div>
        </div>
    )
}

export default Login;