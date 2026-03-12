import "../css/navbar.css";
import { Link } from "react-router-dom";

function Navbar({theme,onToggleTheme,slideMenu}){
    return(
        <div className="navbar">
            <div className="logo">ESP Library</div>
            <div className="nav-options">
                <button className="theme-toggle" onClick={onToggleTheme}>
                    {theme==="light"?<i className="fa-solid fa-moon"></i>:<i className="fa-solid fa-sun"></i>}
                </button>
                <nav className="nav-links">
                    <Link to="/" className="nav-btn"><i className="fa-solid fa-house"></i>Home</Link>
                    <Link to="/login" className="nav-btn"><i className="fa-solid fa-user"></i>Login</Link>
                </nav>
                <button className="hamburger" onClick={slideMenu}><i className="fa-solid fa-bars"></i></button>
            </div>
        </div>
    )
}

export default Navbar;