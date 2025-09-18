import React from 'react';
import SensorCard from './SensorCard';

const SensorUnit = ({ id, name, status, errorCode, sensors }) => {
    // Determine the status color for the sensor unit
    const getStatusColor = (status) => {
        switch (status.toLowerCase()) {
            case 'ok':
                return '#28a745'; // Green
            case 'warning':
                return '#ffc107'; // Yellow
            case 'critical':
            case 'offline':
                return '#dc3545'; // Red
            default:
                return '#6c757d'; // Gray
        }
    };

    const sensorUnitStyle = {
        borderLeft: `5px solid ${getStatusColor(status)}`,
        padding: '20px',
        margin: '15px 0',
        backgroundColor: '#2a2a2a',
        borderRadius: '8px',
        boxShadow: '0 4px 8px rgba(0,0,0,0.2)',
    };

    return (
        <section className="sensor-unit" style={sensorUnitStyle}>
            <div className="sensor-unit-header">
                <h2>{name}</h2>
                <div className="sensor-unit-status">
                    Status: {status}
                    {errorCode && <span className="error-code"> (Error: {errorCode})</span>}
                </div>
                <div className="sensor-unit-id">ID: {id}</div>
            </div>
            <div className="sensors-grid">
                {sensors.map((sensor) => (
                    <SensorCard
                        key={sensor.id}
                        name={sensor.name}
                        value={sensor.value}
                        values={sensor.values}
                        unit={sensor.unit}
                        status={sensor.status}
                        timestamp={sensor.timestamp}
                    />
                ))}
            </div>
        </section>
    );
};

export default SensorUnit;
