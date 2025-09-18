import React, { useState, useEffect } from 'react';
import { fetchMostRecentReadings } from './api';
import './RecentReadings.css';

const RecentReadings = () => {
    const [readings, setReadings] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);

    // Helper function to format values to 3 decimal places unless it's a location/GPS sensor
    const formatValue = (value, sensorName) => {
        const isLocationSensor = sensorName.toLowerCase().includes('gps') || sensorName.toLowerCase().includes('location');
        return isLocationSensor ? value : (typeof parseFloat(value) === 'number' && !isNaN(parseFloat(value)) ? parseFloat(value).toFixed(3) : value);
    };

    useEffect(() => {
        const getReadings = async () => {
            try {
                const data = await fetchMostRecentReadings();
                setReadings(data);
                setError(null);
            } catch (err) {
                setError(err.message);
                setReadings([]);
            } finally {
                setLoading(false);
            }
        };

        getReadings();

        // Set up polling to update readings every 30 seconds
        const intervalId = setInterval(getReadings, 30000);

        // Clean up the interval when the component unmounts
        return () => clearInterval(intervalId);
    }, []);

    if (loading) {
        return <div className="loading-message">Loading recent readings...</div>;
    }

    if (error) {
        return <div className="error-message">Error: {error}</div>;
    }

    return (
        <div className="recent-readings-container">
            <h2>Most Recent Readings</h2>
            <div className="readings-grid">
                {readings.map((reading) => (
                    <div key={`${reading.sensor}-${reading.reading}-${reading.timestamp}`} className="reading-card">
                        <h3>{reading.sensor}</h3>
                        <p className="reading-type">{reading.reading}</p>
                        <p className="reading-value">
                            {Array.isArray(reading.values) 
                                ? reading.values.map((value, index) => (
                                    <span key={index}>{formatValue(value, reading.sensor)}{index < reading.values.length - 1 ? ', ' : ''}</span>
                                  ))
                                : formatValue(reading.value, reading.sensor)}
                            <span className="reading-unit"> {reading.unit}</span>
                        </p>
                        <p className="reading-timestamp">
                            {new Date(reading.timestamp).toLocaleString()}
                        </p>
                    </div>
                ))}
            </div>
        </div>
    );
};

export default RecentReadings;
