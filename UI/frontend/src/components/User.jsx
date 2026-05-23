import {useLocation, useNavigate} from "react-router-dom";
import axios from 'axios';
import {useEffect, useState} from 'react';
import "../css/user.css";

const FINE_PER_DAY = 10;

function User(){
    const location = useLocation();
    const navigate = useNavigate();
    const initialData = location.state?.data || null;
    const [data, setData] = useState(initialData);
    const [loading, setLoading] = useState(initialData === null);

    useEffect(()=>{
        let mounted = true;

        async function loadData(){
            if(initialData !== null){
                setLoading(false);
                return;
            }

            try{
                const res = await axios.get('/api/auth-check');
                if(!mounted) return;
                setData(res.data?.data || []);
            }catch(err){
                navigate('/login');
            }finally{
                if(mounted) setLoading(false);
            }
        }

        loadData();
        return ()=>{ mounted = false; };
    }, [initialData, navigate]);

    function formatDate(dateStr){
        if(!dateStr) return "-";
        return new Date(dateStr).toLocaleDateString("en-IN", {day:"2-digit", month:"short", year:"numeric"});
    }

    function getFineInfo(lastDate){
        if(!lastDate) return { daysOverdue: 0, fineAmount: 0, status: 'No Fine' };
        const dueDate = new Date(lastDate);
        const today = new Date();
        dueDate.setHours(0, 0, 0, 0);
        today.setHours(0, 0, 0, 0);
        const daysOverdue = Math.max(Math.ceil((today - dueDate) / (1000 * 60 * 60 * 24)), 0);
        const fineAmount = daysOverdue * FINE_PER_DAY;
        return {
            daysOverdue,
            fineAmount,
            status: fineAmount > 0 ? 'Fine Due' : 'No Fine'
        };
    }

    if(loading) return <div style={{padding:20}}>Loading user data...</div>;

    return(
        <div className="user">
            <div className="user-card">
                <div className="user-card-header">
                    <div>
                        <h2>User Dashboard</h2>
                        <p>Track borrowed books, due dates, and pending actions from a single place.</p>
                    </div>
                    <div className="user-actions">
                        <button className="fine-btn" onClick={()=>navigate('/fine-page', {state:{data: data || []}})}>
                            Download Status Fine
                        </button>
                        <button className="logout-btn" onClick={async ()=>{
                            try{
                                await axios.post('/api/logout');
                            }catch(e){
                                console.error('Logout error', e);
                            }
                            navigate("/login");
                        }}>
                            Logout
                        </button>
                    </div>
                </div>

                {(!data || data.length === 0) ? (
                    <div className="no-records">No books currently issued.</div>
                ) : (
                    <div className="book-table-wrapper">
                        <table className="book-table">
                            <thead>
                                <tr>
                                    <th>SRN / ID</th>
                                    <th>Book Name</th>
                                    <th>Date Issued</th>
                                    <th>Due Date</th>
                                    <th>Fine</th>
                                    <th>Status</th>
                                </tr>
                            </thead>
                            <tbody>
                                {data.map((indi)=>{
                                    const fineInfo = getFineInfo(indi.lastDate);
                                    return (
                                        <tr key={indi._id}>
                                            <td>{indi.id}</td>
                                            <td>{indi.bookName}</td>
                                            <td>{formatDate(indi.dateIssued)}</td>
                                            <td>{formatDate(indi.lastDate)}</td>
                                            <td>{fineInfo.fineAmount > 0 ? `₹${fineInfo.fineAmount}` : '-'}</td>
                                            <td>
                                                <span className={fineInfo.fineAmount > 0 ? "badge overdue" : "badge active"}>
                                                    {fineInfo.status}
                                                </span>
                                            </td>
                                        </tr>
                                    );
                                })}
                            </tbody>
                        </table>
                    </div>
                )}
            </div>
        </div>
    )
}

export default User;