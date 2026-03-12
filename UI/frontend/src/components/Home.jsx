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
                        <p className="intro-para-1">
                            This project presents a prototype for an automated library management system using ESP32 microcontrollers and RFID technology.
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
                        This project is built around a simple idea — a library should run itself as much as possible.
                        From the moment a book is picked up to the moment it is returned, every action should be tracked,
                        verified, and logged without manual effort.
                    </p>
                    <p className="about-para">
                        Designed as a working prototype, the system targets three core problems faced by traditional libraries:
                        unreliable lending records, missed return deadlines, and untracked book movement.
                        By automating these checkpoints, the system reduces dependency on manual entry and human oversight.
                    </p>
                    <p className="about-para">
                        The result is a faster, more accountable circulation process — one where staff spend less time
                        chasing records and students experience a smoother, more reliable borrowing routine.
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
                            <h3>Scan Tag</h3>
                            <p>The RFID scanner reads the unique tag attached to the book or user card at the service point.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">02</div>
                        <div className="step-content">
                            <h3>Verify Identity</h3>
                            <p>The system cross-checks the scanned ID against the database to confirm user and book validity.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">03</div>
                        <div className="step-content">
                            <h3>Check Status</h3>
                            <p>Lending status, issue date, and return deadline are fetched and evaluated in real time.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">04</div>
                        <div className="step-content">
                            <h3>Authorize or Flag</h3>
                            <p>If all checks pass, the transaction is approved. Otherwise, the system raises an alert for overdue or unauthorized movement.</p>
                        </div>
                    </div>

                    <div className="workflow-arrow">&#8594;</div>

                    <div className="workflow-step">
                        <div className="step-number">05</div>
                        <div className="step-content">
                            <h3>Update Records</h3>
                            <p>The database is updated instantly — issue, return, or flag status is logged with timestamp and user details.</p>
                        </div>
                    </div>

                </div>
            </div>
        </div>
    </>
    )
}

export default Home;