# Deployment Guide: Vercel (Frontend + Backend) + MongoDB Atlas

This guide explains how to deploy your **React frontend** and **Node backend** to **Vercel**, with **MongoDB Atlas** as database.

---

## 1) MongoDB Atlas Setup (Database)

1. Go to [mongodb.com/cloud/atlas](https://www.mongodb.com/cloud/atlas) and sign up (free tier available).
2. Create a new cluster (Free tier: M0).
3. Create a database user:
   - Go to **Security > Database Access**
   - Click **Add New Database User**
   - Username: any name
   - Password: strong password (copy it)
   - Click **Add User**
4. Whitelist IP:
   - Go to **Security > Network Access**
   - Click **Add IP Address**
   - Select **Allow Access from Anywhere** (0.0.0.0/0)
   - Click **Confirm**
5. Get connection string:
   - Go to **Deployment > Database**
   - Click **Connect** on your cluster
   - Choose **Drivers**
   - Copy the connection string (MongoDB+Srv URL)
   - Replace `<username>` and `<password>` with your DB credentials
   - Example: `mongodb+srv://user:pass@cluster0.abc.mongodb.net/library?retryWrites=true&w=majority`

---

## 2) Backend Deployment (Vercel)

1. Push your code to GitHub first:
   ```bash
   git add .
   git commit -m "Ready for deployment"
   git push
   ```

2. Go to [vercel.com](https://vercel.com) and sign up with GitHub.

3. Create a new **Project**:
   - Click **Add New** → **Project**
   - Connect your GitHub repo
   - Framework preset: **Other**
   - Root Directory: `UI/backend`
   - Click **Deploy**

4. Set environment variables:
   - In Vercel dashboard, go to your backend project
   - Click **Environment**
   - Add variables:
     - `MONGO_URI`: Your MongoDB connection string (from step 1.5)
     - `TOTAL_BOOKS`: `6`
     - `FRONTEND_URL`: Your Vercel frontend URL (e.g., `https://esp-library.vercel.app`)
   - Click **Save**

5. Confirm `UI/backend/vercel.json` exists (already added in repo) and routes all traffic to `server.js`.

6. Redeploy after env vars are added. Note your backend URL (e.g., `https://esp-library-backend.vercel.app`).

7. Test the backend:
   - Open `https://esp-library-backend.vercel.app/api/stats` in browser
   - Should see JSON response

---

## 3) Frontend Deployment (Vercel)

1. Update `vercel.json` in `UI/frontend/` folder:
   - Replace `https://your-backend-vercel-domain.vercel.app` with your actual backend Vercel URL
   - Example:
     ```json
     {
       "rewrites": [
         {
           "source": "/api/:path*",
           "destination": "https://esp-library-backend.vercel.app/api/:path*"
         },
         ...
       ]
     }
     ```

2. Push updated code to GitHub.

3. Go to [vercel.com](https://vercel.com) and sign up with GitHub.

4. Create new project:
   - Click **Add New** → **Project**
   - Import your GitHub repo
   - Framework: **Vite**
   - Root Directory: `UI/frontend`
   - Build Command: `npm run build`
   - Output Directory: `dist`
   - Click **Deploy**

5. Wait for deployment (1-2 min).

6. Once live, test:
   - Open your Vercel frontend URL
   - Try logging in or checking stats
   - Check browser console (F12) for any CORS/API errors

---

## 4) ESP32 Configuration for Production

Update your ESP32 sketch with the live backend URL:

```cpp
String addinfoURL = "https://esp-library-backend.vercel.app/api/addinfo";
String exitVerifyURL = "https://esp-library-backend.vercel.app/api/exit-verify";
String peopleCountURL = "https://esp-library-backend.vercel.app/api/people-count";
```

Since URLs are HTTPS, also add to ESP32 setup (after WiFi connect):
```cpp
// After connectWiFi() in setup():
// For HTTPS only:
// You may need: #include <WiFiClientSecure.h>
// Then use: HTTPSClient client; instead of HTTPClient
// Or disable cert verification (less safe):
// client.setInsecure();
```

For simplicity, **keep HTTP** while developing; upgrade to HTTPS with proper cert handling later.

---

## 5) Environment Variables Summary

**Backend (Vercel Environment Variables):**
```env
MONGO_URI=mongodb+srv://user:pass@cluster.abc.mongodb.net/library?retryWrites=true&w=majority
PORT=5000
TOTAL_BOOKS=6
FRONTEND_URL=https://esp-library.vercel.app
```

**Frontend (vercel.json):**
```json
{
  "rewrites": [
    {
      "source": "/api/:path*",
         "destination": "https://esp-library-backend.vercel.app/api/:path*"
    }
  ]
}
```

---

## 6) Troubleshooting

### A) Frontend shows "Cannot GET /api/..." 
- Verify `vercel.json` exists and has correct backend URL
- Check Vercel logs: **Settings > Function Logs**

### B) CORS error in browser console
- Ensure `FRONTEND_URL` env var on backend Vercel project matches frontend domain
- Or manually add your Vercel domain to `allowedOrigins` in `server.js`

### C) MongoDB connection fails
- Verify `MONGO_URI` is correct and doesn't have URL encoding issues
- Ensure IP whitelist on MongoDB Atlas includes your deployment access (or 0.0.0.0/0)
- Test locally first: `mongo "mongodb+srv://..." ` with MongoDB CLI

### D) Backend project returns 500 on Vercel
- Check backend **Runtime Logs** in Vercel
- Verify all env variables are set
- Ensure `npm install` runs without errors

---

## 7) Costs & Limits

- **Vercel**: Free tier (10 Git deployments/month, 100GB bandwidth)
- **Vercel Backend Functions**: subject to serverless limits/timeouts on free tier
- **MongoDB Atlas**: Free tier (512MB storage, 100 max connections)

For production: Upgrade to paid plans as needed.

---

## 8) Quick Checklist Before Going Live

- [ ] Backend URL live and `/api/stats` returns JSON
- [ ] Frontend URL live and loads without CORS errors
- [ ] User can login and see books issued/available/people count
- [ ] Entry RFID scan logs book in
- [ ] Exit RFID scan verifies theft detection
- [ ] People counter (IR sensors) updates on dashboard
- [ ] All green/red LEDs and buzzer trigger correctly
- [ ] Update ESP32 sketch with production URLs
- [ ] MongoDB Atlas whitelist configured + connection tested
- [ ] Tighten CORS to disallow `*` origin (use specific domains)

---

Done! Your system is now production-ready on Vercel + MongoDB.