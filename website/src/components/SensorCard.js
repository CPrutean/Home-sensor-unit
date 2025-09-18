import React from 'react';

// A helper function to get the right color based on status
const getStatusColor = (status) => {
    switch (status.toLowerCase()) {
        case 'ok':
            return '#28a745'; // Green
        case 'warning':
            return '#ffc107'; // Yellow
        case 'critical':
            return '#dc3545'; // Red
        default:
            return '#6c757d'; // Gray
    }
};

const SensorCard = ({ name, value, values, unit, status, timestamp }) => {
    const cardStyle = {
        borderLeft: `5px solid ${getStatusColor(status)}`,
        padding: '16px',
        margin: '8px',
        backgroundColor: '#1e1e1e',
        borderRadius: '8px',
        boxShadow: '0 4px 8px rgba(0,0,0,0.2)',
    };

    const formattedTimestamp = new Date(timestamp).toLocaleTimeString();

    // Format value to 3 decimal places unless it's a location/GPS sensor
    const isLocationSensor = name.toLowerCase().includes('gps') || name.toLowerCase().includes('location');

    // Helper function to format a single value
    const formatValue = (val) => {
        return isLocationSensor ? val : (typeof parseFloat(val) === 'number' && !isNaN(parseFloat(val)) ? parseFloat(val).toFixed(3) : val);
    };

    return (
        <div style={cardStyle}>
            <h3>{name}</h3>
            <p className="sensor-value">
                {values && Array.isArray(values) && values.length > 0 ? (
                    values.map((val, index) => (
                        <span key={index}>
                            {formatValue(val)}
                            {index < values.length - 1 ? ', ' : ''}
                        </span>
                    ))
                ) : (
                    formatValue(value)
                )}
                <span className="sensor-unit"> {unit}</span>
            </p>
            <small>Last updated: {formattedTimestamp}</small>
        </div>
    );
};

export default SensorCard;
