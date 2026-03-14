import { useEffect, useRef, useState } from 'react';
import axios from 'axios';
import { toast } from 'react-toastify';
import '../css/popup.css';

const DURATION=15;

function PopUp({id,bookName,dateIssued,lastDate,onClose}){
    const [timeLeft,setTimeLeft]=useState(DURATION);
    const timerRef=useRef(null);

    useEffect(()=>{
        timerRef.current=setInterval(()=>{
            setTimeLeft(prev=>{
                if(prev<=1){
                    clearInterval(timerRef.current);
                    onClose();
                    return 0;
                }
                return prev-1;
            });
        },1000);
        return ()=>clearInterval(timerRef.current);
    },[]);

    async function withdrawBook(){
        try{
            const response=await axios.post("http://localhost:5000/api/withdraw",{id,bookName});
            if(response.status===200){
                toast.success("Book withdrawn successfully");
            }
        }catch(err){
            toast.error("Error withdrawing book");
        }
        onClose();
    }

    async function renewBook(){
        try{
            const response=await axios.post("http://localhost:5000/api/renew",{id,bookName});
            if(response.status===200){
                toast.success("Book renewed successfully");
            }
        }catch(err){
            toast.error("Error renewing book");
        }
        onClose();
    }

    function formatDate(val){
        if(!val) return '-';
        return new Date(val).toLocaleDateString('en-IN',{day:'2-digit',month:'short',year:'numeric'});
    }

    return(
        <div className="popup-overlay" onClick={onClose}>
            <div className="popup-modal" onClick={(e)=>e.stopPropagation()}>
                <div className="popup-timer-bar">
                    <div className="popup-timer-fill" style={{width:`${(timeLeft/DURATION)*100}%`}}></div>
                </div>
                <h2>Book Already Issued</h2>
                <div className="popup-grid">
                    <span>SRN</span><strong>{id||'-'}</strong>
                    <span>Book Name</span><strong>{bookName||'-'}</strong>
                    <span>Date Issued</span><strong>{formatDate(dateIssued)}</strong>
                    <span>Due Date</span><strong>{formatDate(lastDate)}</strong>
                </div>
                <p className="popup-countdown">Closing in {timeLeft}s</p>
                <div className="popup-actions">
                    <button className="popup-btn-withdraw" onClick={renewBook}>Renew</button>
                    <button className="popup-btn-withdraw" onClick={withdrawBook}>Withdraw</button>
                    <button className="popup-btn-close" onClick={onClose}>Close</button>
                </div>
            </div>
        </div>
    );
}

export default PopUp;