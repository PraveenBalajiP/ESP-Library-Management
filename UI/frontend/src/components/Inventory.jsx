import { useEffect, useState } from "react";
import axios from "axios";
import "../css/inventory.css";

function Inventory(){
    const [inventory,setInventory]=useState([]);
    const [subjectStock,setSubjectStock]=useState(5);

    useEffect(()=>{
        let active=true;

        async function fetchInventory(){
            try{
                const response=await axios.get("/api/inventory");
                if(active){
                    setInventory(response.data.inventory || []);
                    setSubjectStock(response.data.subjectStock || 5);
                }
            }
            catch(error){
                if(active){
                    setInventory([]);
                    setSubjectStock(5);
                }
            }
        }

        fetchInventory();
        const intervalId=setInterval(fetchInventory,3000);

        return ()=>{
            active=false;
            clearInterval(intervalId);
        };
    },[]);

    return(
        <div className="inventory-page">
            <div className="inventory-card">
                <h2>Subject Inventory</h2>
                <p>Each subject has {subjectStock} books in stock. Repeated issue scans reduce available inventory.</p>

                {inventory.length===0 ? (
                    <div className="inventory-empty">No issued subjects yet. Inventory appears after the first scan for a subject.</div>
                ) : (
                    <div className="inventory-table-wrapper">
                        <table className="inventory-table">
                            <thead>
                                <tr>
                                    <th>Subject</th>
                                    <th>Total</th>
                                    <th>Issued</th>
                                    <th>Available</th>
                                </tr>
                            </thead>
                            <tbody>
                                {inventory.map((item)=>(
                                    <tr key={item.subject}>
                                        <td>{item.subject}</td>
                                        <td>{item.totalBooks}</td>
                                        <td>{item.issuedCount}</td>
                                        <td>
                                            <span className={item.availableBooks===0?"stock-badge out":"stock-badge in"}>
                                                {item.availableBooks}
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

export default Inventory;
