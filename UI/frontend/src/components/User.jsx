import {useLocation, useNavigate} from "react-router-dom";
import "../css/user.css";

function User(){
    const location=useLocation();
    const navigate=useNavigate();
    const data=location.state?.data || [];

    function formatDate(dateStr){
        if(!dateStr) return "-";
        return new Date(dateStr).toLocaleDateString("en-IN",{day:"2-digit",month:"short",year:"numeric"});
    }

    function isOverdue(lastDate){
        return new Date(lastDate) < new Date();
    }

    return(
        <div className="user">
            <div className="user-card">
                <div className="user-card-header">
                    <div>
                        <h2>User Dashboard</h2>
                        <p>Track borrowed books, due dates, and pending actions from a single place.</p>
                    </div>
                    <button className="logout-btn" onClick={()=>navigate("/login")}>Logout</button>
                </div>

                {data.length===0 ? (
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
                                    <th>Status</th>
                                </tr>
                            </thead>
                            <tbody>
                                {data.map((indi)=>(
                                    <tr key={indi._id}>
                                        <td>{indi.id}</td>
                                        <td>{indi.bookName}</td>
                                        <td>{formatDate(indi.dateIssued)}</td>
                                        <td>{formatDate(indi.lastDate)}</td>
                                        <td>
                                            <span className={isOverdue(indi.lastDate)?"badge overdue":"badge active"}>
                                                {isOverdue(indi.lastDate)?"Overdue":"Active"}
                                            </span>
                                        </td>
                                    </tr>
                                ))}
                            </tbody>
                        </table>
                    </div>
                )}
            </div>
        </div>
    )
}

export default User;