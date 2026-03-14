import { useEffect, useState } from 'react';
import {Route,Routes,Link} from 'react-router-dom';
import { ToastContainer } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import axios from 'axios';
import NavBar from './common_components/navbar';
import Home from './components/Home';
import Login from './components/Login';
import User from './components/User';
import PopUp from './common_components/popup';
import Footer from './common_components/footer';
import './App.css';
import './css/toast.css';

function App(){
  const [theme,setTheme]=useState(()=>localStorage.getItem('theme') || 'light');
  const [menu,setMenu]=useState(false);
  const [popupData,setPopupData]=useState(null);

  useEffect(()=>{
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);
  }, [theme]);

  const toggleTheme=()=>{
    setTheme((prevTheme)=>(prevTheme==='light'?'dark':'light'));
  };

  const slideMenu=()=>{
    setMenu(prevMenu=>!prevMenu);
  };

  const closeMenu=()=>{
    setMenu(false);
  };

  useEffect(()=>{
  const interval=setInterval(async ()=>{
    try{
      const response=await axios.get("http://192.168.137.1:5000/api/addinfo");
      if(response.data.popup){
        setPopupData(response.data);
      }
      }catch(err){
        console.log(err);
      }
    },2000);
    return ()=>clearInterval(interval);
  },[]);

  return(
    <div className="App">
      <NavBar theme={theme} onToggleTheme={toggleTheme} slideMenu={slideMenu}/>
      <div className={`slide-backdrop ${menu ? 'open' : ''}`} onClick={closeMenu}></div>
      <div className={`slide-bar ${menu ? 'open' : ''}`}>
        <button onClick={closeMenu}>Close</button>
        <nav className="nav-links">
            <Link to="/" className="nav-btn" onClick={closeMenu}><i className="fa-solid fa-house"></i>Home</Link>
            <Link to="/login" className="nav-btn" onClick={closeMenu}><i className="fa-solid fa-user"></i>Login</Link>
        </nav>
      </div>
      <Routes>
        <Route path="/" element={<Home/>} />
        <Route path="/login" element={<Login/>} />
        <Route path="/user" element={<User/>} />
      </Routes>
      {popupData && (
        <PopUp
          id={popupData.id}
          bookName={popupData.bookName}
          dateIssued={popupData.dateIssued}
          lastDate={popupData.lastDate}
          onClose={()=>setPopupData(null)}
        />
      )}
      <ToastContainer position="top-right" autoClose={3000} />
      <Footer/>
    </div>
  );
}

export default App;