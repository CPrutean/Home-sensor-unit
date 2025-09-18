import React from 'react';
import SensorCard from './SensorCard';

const Cluster = ({ clusterName, sensors }) => {
    return (
        <section className="sensor-cluster">
            <div className="cluster-header">
                <h2>{clusterName}</h2>
            </div>
            <div className="dashboard-grid">
                {sensors.map((sensor) => (
                    <SensorCard
                        key={sensor.id}
                        name={sensor.name}
                        value={sensor.value}
                        unit={sensor.unit}
                        status={sensor.status}
                        timestamp={sensor.timestamp}
                    />
                ))}
            </div>
        </section>
    );
};

export default Cluster;