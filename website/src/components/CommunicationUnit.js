import React from 'react';
import SensorUnit from './SensorUnit'; // We'll rename Cluster.js to SensorUnit.js later

const CommunicationUnit = ({ id, name, sensorUnits }) => {
    return (
        <div className="communication-unit">
            <div className="communication-unit-header">
                <h1>{name}</h1>
                <div className="communication-unit-id">ID: {id}</div>
            </div>
            <div className="sensor-units-container">
                {sensorUnits.map((sensorUnit) => (
                    <SensorUnit
                        key={sensorUnit.id}
                        id={sensorUnit.id}
                        name={sensorUnit.name}
                        status={sensorUnit.status}
                        errorCode={sensorUnit.errorCode}
                        sensors={sensorUnit.sensors}
                    />
                ))}
            </div>
        </div>
    );
};

export default CommunicationUnit;