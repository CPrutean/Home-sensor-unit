import React, { useState, useEffect } from 'react';
import { fetchMostRecentReadings } from './api';
import './MapComponent.css';

const MapComponent = () => {
    const [gpsReadings, setGpsReadings] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);

    useEffect(() => {
        const getGpsReadings = async () => {
            try {
                const data = await fetchMostRecentReadings();
                // Filter readings to only include GPS readings
                // Assuming GPS readings have "GPS" in the reading type or sensor name
                const gpsData = data.filter(reading => 
                    reading.sensor.toLowerCase().includes('gps') || 
                    reading.reading.toLowerCase().includes('gps') ||
                    (reading.values && reading.values.length >= 2)
                );
                setGpsReadings(gpsData);
                setError(null);
            } catch (err) {
                setError(err.message);
                setGpsReadings([]);
            } finally {
                setLoading(false);
            }
        };

        getGpsReadings();

        // Set up polling to update readings every 30 seconds
        const intervalId = setInterval(getGpsReadings, 30000);

        // Clean up the interval when the component unmounts
        return () => clearInterval(intervalId);
    }, []);

    if (loading) {
        return <div className="loading-message">Loading GPS data...</div>;
    }

    if (error) {
        return <div className="error-message">Error: {error}</div>;
    }

    if (gpsReadings.length === 0) {
        return <div className="no-data-message">No GPS data available</div>;
    }

    // For simplicity, we'll just use the first GPS reading
    // In a real application, you might want to show multiple markers
    const latestGpsReading = gpsReadings[0];
    
    // Extract latitude and longitude from the values array
    // Assuming the first value is latitude and the second is longitude
    const latitude = latestGpsReading.values && latestGpsReading.values.length > 0 
        ? latestGpsReading.values[0] 
        : 0;
    const longitude = latestGpsReading.values && latestGpsReading.values.length > 1 
        ? latestGpsReading.values[1] 
        : 0;

    // Create a Google Maps URL with the latitude and longitude
    const apiKey = process.env.REACT_APP_GOOGLE_MAPS_API_KEY;
    const mapUrl = `https://www.google.com/maps/embed/v1/place?key=${apiKey}&q=${latitude},${longitude}&zoom=15`;

    return (
        <div className="map-container">
            <h2>GPS Location</h2>
            <div className="map-wrapper">
                {/* 
                    Note: In a production environment, you would need to:
                    1. Get a Google Maps API key
                    2. Replace YOUR_API_KEY in the URL with your actual API key
                    3. Consider using a proper map library like react-leaflet or google-maps-react
                */}
                <iframe
                    title="GPS Location"
                    width="100%"
                    height="400"
                    frameBorder="0"
                    src={mapUrl}
                    allowFullScreen
                ></iframe>
            </div>
            <div className="gps-info">
                <p>Latitude: {latitude}</p>
                <p>Longitude: {longitude}</p>
                <p>Last updated: {new Date(latestGpsReading.timestamp).toLocaleString()}</p>
            </div>
        </div>
    );
};

export default MapComponent;