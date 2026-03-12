import { useEffect, useState } from 'react';
import {Route,Routes} from 'react-router-dom';
import NavBar from './common_components/navbar';
import Home from './components/Home';
import Login from './components/Login';
import User from './components/User';
import Footer from './common_components/footer';

function App(){
  const [theme, setTheme] = useState(() => localStorage.getItem('theme') || 'light');

  useEffect(() => {
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);
  }, [theme]);

  const toggleTheme = () => {
    setTheme((prevTheme) => (prevTheme === 'light' ? 'dark' : 'light'));
  };

  return(
    <div className="App">
      <NavBar theme={theme} onToggleTheme={toggleTheme} />
      <Routes>
        <Route path="/" element={<Home/>} />
        <Route path="/login" element={<Login/>} />
        <Route path="/user" element={<User/>} />
      </Routes>
      <Footer/>
    </div>
  );
}

export default App;