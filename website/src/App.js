import React from 'react';
import Dashboard from './components/Dashboard';
import './App.css';

function App() {
  return (
      <div className="App">
        <header className="App-header">
          <div className="App-logo-container">
            <img src={process.env.PUBLIC_URL + '/favicon.ico'} alt="Logo" className="App-logo" />
            <h1>IoT Sensor Dashboard</h1>
          </div>
        </header>
        <main>
          <Dashboard />
        </main>
      </div>
  );
}

export default App;
