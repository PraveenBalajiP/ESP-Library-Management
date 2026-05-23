import { useEffect, useState } from 'react';
import {Route,Routes,Link} from 'react-router-dom';
import { ToastContainer } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import axios from 'axios';
import NavBar from './common_components/navbar';
import Home from './components/Home';
import Login from './components/Login';
import User from './components/User';
import FinePage from './components/FinePage';
import ProtectedRoute from './common_components/ProtectedRoute';
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
      const response=await axios.get("/api/addinfo");
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
            <Link to="/inventory" className="nav-btn" onClick={closeMenu}><i className="fa-solid fa-book"></i>Inventory</Link>
            <Link to="/login" className="nav-btn" onClick={closeMenu}><i className="fa-solid fa-user"></i>Login</Link>
        </nav>
      </div>
      <Routes>
        <Route path="/" element={<Home/>} />
        <Route path="/login" element={<Login/>} />
        <Route path="/user" element={
          <ProtectedRoute>
            <User />
          </ProtectedRoute>
        } />
        <Route path="/fine-page" element={
          <ProtectedRoute>
            <FinePage />
          </ProtectedRoute>
        } />
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
      <ToastContainer
        position="bottom-center"
        autoClose={2200}
        pauseOnHover={false}
        pauseOnFocusLoss={false}
        closeOnClick
        draggable
        newestOnTop={false}
        theme="dark"
        toastClassName={({type})=>{
          if(type === 'success') return 'toast-success';
          if(type === 'error') return 'toast-error';
          if(type === 'info') return 'toast-default';
          if(type === 'warning') return 'toast-default';
          if(type === 'default') return 'toast-default';
          if(type === 'loading') return 'toast-loading';
          return 'toast-default';
        }}
      />
      <Footer/>
    </div>
  );
}

export default App;