import {useEffect, useMemo, useState} from 'react';
import {useLocation, useNavigate} from 'react-router-dom';
import axios from 'axios';
import {toast} from 'react-toastify';
import '../css/fine.css';

const FINE_PER_DAY = 10;

function FinePage(){
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
				const response = await axios.get('/api/auth-check');
				if(!mounted) return;
				setData(response.data?.data || []);
			}catch(err){
				navigate('/login');
			}finally{
				if(mounted) setLoading(false);
			}
		}

		loadData();
		return ()=>{ mounted = false; };
	}, [initialData, navigate]);

	const fineRows = useMemo(()=>{
		const now = new Date();
		now.setHours(0,0,0,0);

		return (data || []).map((item)=>{
			const dueDate = new Date(item.lastDate);
			dueDate.setHours(0,0,0,0);
			const daysOverdue = Math.max(Math.ceil((now - dueDate) / (1000 * 60 * 60 * 24)), 0);
			const fineAmount = daysOverdue * FINE_PER_DAY;
			return {
				...item,
				daysOverdue,
				fineAmount,
				status: fineAmount > 0 ? 'Fine Due' : 'No Fine'
			};
		});
	}, [data]);

	function formatDate(dateStr){
		if(!dateStr) return '-';
		return new Date(dateStr).toLocaleDateString('en-IN', {day:'2-digit', month:'short', year:'numeric'});
	}

	function downloadCSV(){
		const header = ['SRN', 'Book Name', 'Date Issued', 'Due Date', 'Days Overdue', 'Fine', 'Status'];
		const rows = fineRows.map((row)=>[
			row.id,
			row.bookName,
			formatDate(row.dateIssued),
			formatDate(row.lastDate),
			row.daysOverdue,
			row.fineAmount,
			row.status,
		]);

		const csv = [header, ...rows]
			.map((line)=>line.map((value)=>`"${String(value ?? '').replaceAll('"', '""')}"`).join(','))
			.join('\n');

		const blob = new Blob([csv], {type: 'text/csv;charset=utf-8;'});
		const url = URL.createObjectURL(blob);
		const link = document.createElement('a');
		link.href = url;
		link.download = 'fine-status.csv';
		link.click();
		URL.revokeObjectURL(url);
		toast.success('Fine status downloaded');
	}

	if(loading) return <div style={{padding:20}}>Loading fine details...</div>;

	return (
		<div className="fine-page">
			<div className="fine-card">
				<div className="fine-header">
					<div>
						<h2>Fine Status</h2>
						<p>View overdue details and download the fine summary for the current user.</p>
					</div>
					<div className="fine-actions">
						<button className="fine-secondary-btn" onClick={()=>navigate('/user', {state: {data: data || []}})}>Back</button>
						<button className="fine-primary-btn" onClick={downloadCSV} disabled={fineRows.length===0}>Download Fine Status</button>
					</div>
				</div>

				{fineRows.length === 0 ? (
					<div className="fine-empty">No issued books found for the current user.</div>
				) : (
					<div className="fine-table-wrap">
						<table className="fine-table">
							<thead>
								<tr>
									<th>SRN / ID</th>
									<th>Book Name</th>
									<th>Date Issued</th>
									<th>Due Date</th>
									<th>Days Overdue</th>
									<th>Fine</th>
									<th>Status</th>
								</tr>
							</thead>
							<tbody>
								{fineRows.map((item)=> (
									<tr key={item._id}>
										<td>{item.id}</td>
										<td>{item.bookName}</td>
										<td>{formatDate(item.dateIssued)}</td>
										<td>{formatDate(item.lastDate)}</td>
										<td>{item.daysOverdue}</td>
										<td>{item.fineAmount > 0 ? `₹${item.fineAmount}` : '-'}</td>
										<td>
											<span className={item.fineAmount > 0 ? 'fine-badge overdue' : 'fine-badge clear'}>{item.status}</span>
										</td>
									</tr>
								))}
							</tbody>
						</table>
					</div>
				)}
			</div>
		</div>
	);
}

export default FinePage;