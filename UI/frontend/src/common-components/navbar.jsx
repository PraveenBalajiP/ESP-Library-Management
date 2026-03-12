import "../css/navbar.css";
import { Link } from "react-router-dom";

function Navbar({ theme, onToggleTheme }){
    return(
        <div className="navbar">
            <div className="logo">ESP Library</div>
            <div className="nav-options">
                <button className="theme-toggle" onClick={onToggleTheme}>
                    {theme === "light" ? "Dark Mode" : "Light Mode"}
                </button>
                <nav className="nav-links">
                    <Link to="/" className="nav-btn">Home</Link>
                    <Link to="/login" className="nav-btn">Login</Link>
                    <Link to="/user" className="nav-btn">User</Link>
                </nav>
            </div>
        </div>
    )
}

export default Navbar;