import {useEffect, useState} from 'react';
import {useNavigate} from 'react-router-dom';
import axios from 'axios';

// Wraps a single child and ensures the user is authenticated via /api/auth-check
export default function ProtectedRoute({children}){
  const [loading,setLoading]=useState(true);
  const navigate=useNavigate();

  useEffect(()=>{
    let mounted=true;
    (async ()=>{
      try{
        const res = await axios.get('/api/auth-check');
        if(!mounted) return;
        if(res.data && res.data.authenticated){
          setLoading(false);
        }else{
          navigate('/login');
        }
      }catch(err){
        navigate('/login');
      }
    })();
    return ()=>{ mounted=false };
  },[navigate]);

  if(loading) return <div style={{padding:20}}>Checking authentication...</div>;
  return children;
}
