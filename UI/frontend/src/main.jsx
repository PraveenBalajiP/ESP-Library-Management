import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import {BrowserRouter} from "react-router-dom"
import axios from 'axios'
import './index.css'
import App from './App.jsx'

axios.defaults.baseURL=import.meta.env.VITE_API_BASE_URL || ''
// Send cookies/credentials by default so httpOnly auth cookie is included
axios.defaults.withCredentials = true;

createRoot(document.getElementById('root')).render(
  <StrictMode>
      <BrowserRouter>
        <App />
      </BrowserRouter>
  </StrictMode>,
)
