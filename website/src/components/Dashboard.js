import React, { useState, useEffect } from 'react';
import CommunicationUnit from './CommunicationUnit';
import RecentReadings from './RecentReadings';
import MapComponent from './MapComponent';
import GraphComponent from './GraphComponent';
import { fetchAllData, fetchMostRecentReadings } from './api';
import './Dashboard.css';

const Dashboard = () => {
    const [commUnits, setCommUnits] = useState([]);
    const [selectedCommUnitId, setSelectedCommUnitId] = useState(null);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);

    useEffect(() => {
        const getData = async () => {
            try {
                const data = await fetchAllData();
                // Filter out duplicate communication units by ID
                const uniqueCommUnits = [];
                const commUnitIds = new Set();

                data.forEach(commUnit => {
                    // Extract the numeric ID from the string (e.g., "comm-1" -> 1)
                    const idMatch = commUnit.id.match(/comm-(\d+)/);
                    if (idMatch && idMatch[1]) {
                        const numericId = idMatch[1];
                        if (!commUnitIds.has(numericId)) {
                            commUnitIds.add(numericId);
                            uniqueCommUnits.push(commUnit);
                        }
                    } else {
                        // If we can't extract a numeric ID, just use the full ID
                        if (!commUnitIds.has(commUnit.id)) {
                            commUnitIds.add(commUnit.id);
                            uniqueCommUnits.push(commUnit);
                        }
                    }
                });

                setCommUnits(uniqueCommUnits);

                // Set the first communication unit as selected by default
                if (uniqueCommUnits.length > 0) {
                    setSelectedCommUnitId(uniqueCommUnits[0].id);
                }

                setError(null);
            } catch (err) {
                setError(err.message);
                setCommUnits([]);
            } finally {
                setLoading(false);
            }
        };

        getData();
    }, []); // The empty dependency array [] means this effect runs once on mount

    // Set up polling for most recent readings
    useEffect(() => {
        // Don't start polling until initial data is loaded
        if (commUnits.length === 0) return;

        const updateReadings = async () => {
            try {
                const recentReadings = await fetchMostRecentReadings();

                // Update the communication units with the most recent readings
                setCommUnits(prevCommUnits => {
                    // Create a deep copy of the communication units
                    const updatedCommUnits = JSON.parse(JSON.stringify(prevCommUnits));

                    // Update each sensor with its most recent reading if available
                    recentReadings.forEach(reading => {
                        // Find the sensor unit and sensor to update
                        updatedCommUnits.forEach(commUnit => {
                            commUnit.sensorUnits.forEach(sensorUnit => {
                                const sensorToUpdate = sensorUnit.sensors.find(sensor => sensor.id === reading.sensorId);
                                if (sensorToUpdate) {
                                    // Update the sensor with the new reading
                                    sensorToUpdate.value = reading.value;
                                    sensorToUpdate.status = reading.status;
                                    sensorToUpdate.timestamp = reading.timestamp;
                                }
                            });
                        });
                    });

                    return updatedCommUnits;
                });
            } catch (err) {
                console.error('Error updating sensor readings:', err);
                // Don't set error state here to avoid disrupting the UI
            }
        };

        // Update readings every 30 seconds
        const intervalId = setInterval(updateReadings, 30000);

        // Clean up the interval when the component unmounts
        return () => clearInterval(intervalId);
    }, [commUnits.length]); // Only re-run when commUnits.length changes (i.e., when initial data is loaded)

    if (loading) {
        return <div className="loading-message">Loading sensor data...</div>;
    }

    if (error) {
        return <div className="error-message">Error: {error}</div>;
    }

    // Find the selected communication unit
    const selectedCommUnit = commUnits.find(unit => unit.id === selectedCommUnitId);

    // Handle change in dropdown selection
    const handleCommUnitChange = (e) => {
        setSelectedCommUnitId(e.target.value);
    };

    return (
        <div className="dashboard-container">
            {/* Display all recent readings */}
            <RecentReadings />

            {/* Display GPS location on a map */}
            <MapComponent />

            {/* Display graph of sensor readings */}
            <GraphComponent />

            {/* Dropdown selector for communication units */}
            <div className="comm-unit-selector">
                <label htmlFor="comm-unit-select">Select Communication Unit:</label>
                <select 
                    id="comm-unit-select"
                    value={selectedCommUnitId || ''}
                    onChange={handleCommUnitChange}
                >
                    {commUnits.map(unit => (
                        <option key={unit.id} value={unit.id}>
                            {unit.name}
                        </option>
                    ))}
                </select>
            </div>

            {/* Display only the selected communication unit */}
            {selectedCommUnit && (
                <CommunicationUnit
                    key={selectedCommUnit.id}
                    id={selectedCommUnit.id}
                    name={selectedCommUnit.name}
                    sensorUnits={selectedCommUnit.sensorUnits}
                />
            )}
        </div>
    );
};

export default Dashboard;
