import React, { useState, useEffect, useRef } from 'react';
import { fetchMostRecentReadings } from './api';
import './GraphComponent.css';

const GraphComponent = () => {
    const [readings, setReadings] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const [selectedSensor, setSelectedSensor] = useState(null);
    const canvasRef = useRef(null);

    useEffect(() => {
        const getReadings = async () => {
            try {
                const data = await fetchMostRecentReadings();
                setReadings(data);
                
                // Set the first sensor as selected by default
                if (data.length > 0 && !selectedSensor) {
                    const sensors = [...new Set(data.map(reading => reading.sensor))];
                    if (sensors.length > 0) {
                        setSelectedSensor(sensors[0]);
                    }
                }
                
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
    }, [selectedSensor]);

    // Draw the graph when readings or selected sensor changes
    useEffect(() => {
        if (readings.length === 0 || !selectedSensor || !canvasRef.current) return;

        const canvas = canvasRef.current;
        const ctx = canvas.getContext('2d');
        const width = canvas.width;
        const height = canvas.height;

        // Clear the canvas
        ctx.clearRect(0, 0, width, height);

        // Filter readings for the selected sensor
        const sensorReadings = readings.filter(reading => reading.sensor === selectedSensor);

        // Sort readings by timestamp
        sensorReadings.sort((a, b) => new Date(a.timestamp) - new Date(b.timestamp));

        // If no readings for the selected sensor, show a message
        if (sensorReadings.length === 0) {
            ctx.fillStyle = '#aaa';
            ctx.font = '14px Arial';
            ctx.textAlign = 'center';
            ctx.fillText('No data available for this sensor', width / 2, height / 2);
            return;
        }

        // Group readings by reading type
        const readingTypes = [...new Set(sensorReadings.map(reading => reading.reading))];
        const readingsByType = {};
        
        readingTypes.forEach(type => {
            readingsByType[type] = sensorReadings.filter(reading => reading.reading === type);
        });

        // Define colors for different reading types
        const colors = ['#4e79a7', '#f28e2c', '#e15759', '#76b7b2', '#59a14f', '#edc949', '#af7aa1', '#ff9da7', '#9c755f', '#bab0ab'];

        // Draw the graph for each reading type
        Object.keys(readingsByType).forEach((type, typeIndex) => {
            const typeReadings = readingsByType[type];
            
            // Extract values (assuming the first value is the one we want to graph)
            const values = typeReadings.map(reading => 
                reading.values && reading.values.length > 0 
                    ? parseFloat(reading.values[0]) 
                    : parseFloat(reading.value) || 0
            );

            // Find min and max values for scaling
            const minValue = Math.min(...values);
            const maxValue = Math.max(...values);
            const valueRange = maxValue - minValue;

            // Draw the line
            ctx.beginPath();
            ctx.strokeStyle = colors[typeIndex % colors.length];
            ctx.lineWidth = 2;

            typeReadings.forEach((reading, index) => {
                const value = reading.values && reading.values.length > 0 
                    ? parseFloat(reading.values[0]) 
                    : parseFloat(reading.value) || 0;
                
                // Scale x and y to fit the canvas
                const x = (index / (typeReadings.length - 1)) * width;
                const y = height - ((value - minValue) / (valueRange || 1)) * (height - 40) - 20;
                
                if (index === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            });

            ctx.stroke();

            // Draw the legend
            ctx.fillStyle = colors[typeIndex % colors.length];
            ctx.fillRect(10, 10 + typeIndex * 20, 15, 15);
            ctx.fillStyle = '#f0f0f0';
            ctx.font = '12px Arial';
            ctx.textAlign = 'left';
            ctx.fillText(type, 30, 22 + typeIndex * 20);
        });

    }, [readings, selectedSensor]);

    // Get unique sensor names for the dropdown
    const sensorNames = [...new Set(readings.map(reading => reading.sensor))];

    if (loading) {
        return <div className="loading-message">Loading sensor data...</div>;
    }

    if (error) {
        return <div className="error-message">Error: {error}</div>;
    }

    return (
        <div className="graph-container">
            <h2>Sensor Readings Graph</h2>
            <div className="sensor-selector">
                <label htmlFor="sensor-select">Select Sensor:</label>
                <select 
                    id="sensor-select"
                    value={selectedSensor || ''}
                    onChange={(e) => setSelectedSensor(e.target.value)}
                >
                    {sensorNames.map(sensor => (
                        <option key={sensor} value={sensor}>{sensor}</option>
                    ))}
                </select>
            </div>
            <div className="graph-wrapper">
                <canvas 
                    ref={canvasRef} 
                    width={800} 
                    height={400}
                    className="graph-canvas"
                ></canvas>
            </div>
        </div>
    );
};

export default GraphComponent;