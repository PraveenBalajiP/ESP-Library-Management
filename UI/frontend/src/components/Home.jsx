import {Link} from "react-router-dom";
import "../css/home.css";

function Home(){
    return(
        <>
        <div className="home">
            <div className="home-intro">
                <div className="intro-b1">
                    <div className="intro-text">
                        <h1 className="intro-title">Library Management Prototype</h1>
                        <p className="intro-para-0">
                            Read. Return. Repeat.
                        </p>
                        <p>
                            In an era of digital transformation, our mission is to bring speed, accuracy, and automation to library management, making book tracking and access simpler than ever before.
                        </p>
                    </div>
                    <div className="login-sec">
                        
                    </div>                  
                </div>
                <div className="intro-b2">
                    <div className="info-1">
                        <h2 className="info-title">Books Issued Till Now</h2>
                        <p className="info-para">1000</p>
                    </div>
                    <div className="info-2">
                        <h2 className="info-title">Books Available</h2>
                        <p className="info-para">500</p> 
                    </div>
                    <div className="info-3">
                        <h2 className="info-title">Pending Returns</h2>
                        <p className="info-para">50</p>   
                    </div>
                </div>
            </div>
            <div className="about">
                <div className="about-text">
                    <span className="about-tag">About the Project</span>
                    <h2 className="about-title">Bringing Automation to Every Library Shelf</h2>
                    <p className="about-para">
                        This project focuses on the development of a smart library management system using RFID technology and web technologies. The main objective is to modernize traditional library operations by introducing automation for book identification and tracking. By using RFID tags attached to books and an RFID reader connected to a microcontroller, the system enables quick and contactless identification of library items. This approach helps reduce manual work, improves accuracy, and speeds up common library processes such as issuing and returning books.   
                    </p>
                </div>
                <div className="about-highlights">
                    <div className="highlight-card">
                        <span className="highlight-icon">&#128218;</span>
                        <h4>Accurate Records</h4>
                        <p>Every transaction is logged instantly, eliminating manual entry errors.</p>
                    </div>
                    <div className="highlight-card">
                        <span className="highlight-icon">&#9200;</span>
                        <h4>Timely Returns</h4>
                        <p>Due-date tracking keeps borrowers accountable and shelves stocked.</p>
                    </div>
                    <div className="highlight-card">
                        <span className="highlight-icon">&#128274;</span>
                        <h4>Theft Prevention</h4>
                        <p>Unauthorized book movement is flagged before it ever leaves the building.</p>
                    </div>
                    <div className="highlight-card">
                        <span className="highlight-icon">&#9889;</span>
                        <h4>Real-Time Updates</h4>
                        <p>Status changes reflect across the system the moment they happen.</p>
                    </div>
                </div>
            </div>

            <div className="workflow">
                <h2 className="workflow-title">How It Works</h2>
                <div className="workflow-steps">

                    <div className="workflow-step">
                        <div className="step-number">01</div>
                        <div className="step-content">
                            <h3>RFID Tag Assignment</h3>
                            <p>Each book in the library is attached with a unique RFID tag that stores identification information about the book.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">02</div>
                        <div className="step-content">
                            <h3>Book Scanning</h3>
                            <p>When a book is placed near the RFID reader, the reader scans the RFID tag and captures the book’s unique ID.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">03</div>
                        <div className="step-content">
                            <h3>Data Transmission</h3>
                            <p>The RFID reader connected to the microcontroller sends the scanned data to the backend server through the network</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">04</div>
                        <div className="step-content">
                            <h3>Data Processing</h3>
                            <p>The backend application receives the data, processes it, and checks or updates the book information in the database.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">05</div>
                        <div className="step-content">
                            <h3>System Response</h3>
                            <p>Based on the data, the system records the transaction (such as issuing or returning a book) and updates the library records.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">06</div>
                        <div className="step-content">
                            <h3>User Interface Update</h3>
                            <p>The web application displays the updated information to the user, allowing librarians to monitor and manage library activities easily.</p>
                        </div>
                    </div>

                </div>
            </div>
        </div>
    </>
    )
}

export default Home;