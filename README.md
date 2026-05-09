# ESP Library Management — Full Project Details & Setup

_Previous updation: 19-03-2026_

_Last updated: 21-04-2026_

## Overview

This project combines:
- **ESP32 hardware workflow** (RFID issue/exit detection + IR people counting)
- **React frontend** for dashboard/login
- **Express + MongoDB backend** for APIs and records

---

## 1) Repository Structure

```text
ESP-Library_Management/
├─ main-logic.ino                  # Primary ESP32 firmware (RFID + IR + API calls)
├─ uid-detection.ino               # Utility sketch to read RFID UID values
├─ wifi-connection.ino             # Utility sketch to validate Wi-Fi connectivity
├─ urls-config.txt                 # Active backend API URLs used by device
├─ urls.example.txt                # URL template placeholders
└─ UI/
   ├─ backend/
   │  ├─ server.js                 # Express API server
   │  ├─ package.json              # Backend dependencies & scripts
   │  ├─ vercel.json               # Backend Vercel deployment config
   │  ├─ mongoConnect/connectDB.js # Mongo connection bootstrap
   │  └─ models/datas.models.js    # Mongoose schema/model
   └─ frontend/
      ├─ src/                      # React app source
      ├─ package.json              # Frontend dependencies & scripts
      ├─ vite.config.js            # Dev proxy (/api -> localhost:5000)
      └─ vercel.json               # Frontend rewrites to backend API
```

---

## 2) Architecture

### Hardware Layer (ESP32)
- Reads RFID UIDs at entry and exit readers.
- Maps UIDs to known student IDs and book names.
- Tracks people count through IR entry/exit sensors.
- Sends HTTPS payloads to backend endpoints.

### Backend Layer (Express + MongoDB)
- Receives issue/verify/count events from ESP32.
- Stores issued-book records in MongoDB.
- Exposes login and stats APIs for frontend.
- Supports duplicate-issue popup workflow.

### Frontend Layer (React + Vite)
- Displays issued count, available books, and people count.
- Provides login and user dashboard pages.
- Polls duplicate popup endpoint periodically.
- Uses Vite proxy locally and Vercel rewrites in deployment.

---

## 3) Backend Setup

### 3.1 Prerequisites
- Node.js 18+ (latest LTS recommended)
- MongoDB connection URI (Atlas or local)

### 3.2 Install & Run
```bash
cd UI/backend
npm install
npm run dev   # nodemon server.js
# or
npm start     # node server.js
```

### 3.3 Environment Variables (`UI/backend/.env`)
```env
PORT=5000
MONGO_URI=your_mongodb_connection_string
FRONTEND_URL=http://localhost:5173
TOTAL_BOOKS=6
```

### 3.4 CORS Behavior
Allowed origins include:
- `http://localhost:3000`
- `http://localhost:5173`
- `http://localhost:5000`
- `FRONTEND_URL` (if set)
- Any origin containing `vercel.app`

---

## 4) Backend API Endpoints

| Method | Route | Purpose | Notes |
|---|---|---|---|
| GET | `/` | Health response | Basic backend check |
| POST | `/api/addinfo` | Issue/entry record | Returns duplicate status if same `id + bookName` exists |
| GET | `/api/addinfo` | Read/reset popup payload | Polled by frontend every 2s |
| POST | `/api/checkout` | Checkout (delete issue record) | Validates record exists |
| POST | `/api/login` | Fetch records per user | `pesu@123` returns all records |
| GET | `/api/stats` | Issued/available/people stats | Uses `TOTAL_BOOKS` |
| POST | `/api/people-count` | Update in-memory people count | Requires non-negative `count` |
| POST | `/api/renew` | Extend due date by 15 days | Record must exist |
| POST | `/api/withdraw` | Delete issued record | Record must exist |
| POST | `/api/exit-verify` | Theft detection check | 400 if not issued, 200 if valid |

---

## 5) Frontend Setup

### 5.1 Install & Run
```bash
cd UI/frontend
npm install
npm run dev
```

### 5.2 Build & Preview
```bash
npm run build
npm run preview
```

### 5.3 API Routing
- Local dev: `vite.config.js` proxies `/api` to `http://localhost:5000`
- Frontend requests are made with relative paths like:
  - `/api/stats`
  - `/api/login`
  - `/api/addinfo`

---

## 6) ESP32 Firmware Setup

### 6.1 Required Arduino Libraries
- `MFRC522`
- ESP32 core libraries (`WiFi.h`, `WiFiClientSecure.h`)
- Built-in `HTTPClient`, `SPI`

### 6.2 Pin Mapping

| Component | Pin |
|---|---|
| RFID Entry SS | GPIO 5 |
| RFID Exit SS | GPIO 27 |
| RFID Reset | GPIO 22 |
| SPI SCK | GPIO 18 |
| SPI MISO | GPIO 19 |
| SPI MOSI | GPIO 23 |
| Buzzer | GPIO 26 |
| Green LED | GPIO 25 |
| Red LED | GPIO 33 |
| IR Entry Sensor | GPIO 32 |
| IR Exit Sensor | GPIO 34 |

### 6.3 Firmware Variables to Update
In `main-logic.ino`:
- Wi-Fi credentials: `ssid`, `password`
- Backend URLs: `addinfoURL`, `exitVerifyURL`, `peopleCountURL`
- RFID mappings in:
  - `getStudent()`
  - `getBook()`

### 6.4 Utility Sketches
- `wifi-connection.ino`: tests Wi-Fi and prints IP
- `uid-detection.ino`: prints RFID UIDs to serial monitor

---

## 7) URL Config Files

- `urls.example.txt` has placeholder endpoint templates.
- `urls-config.txt` contains active deployed endpoint URLs.
- Keep URL files and firmware URL constants aligned per environment.

## 8) Future Updates

- Direct fine calculation upon overdue based on certain credentials given by librarian
- Seperate page that is accessible by librarian that shows status of only overdue books alongwith fine amount accumulated
- Warning sent to the user upon his login for his overdue books
- Direct payment of fines through RazorPay
