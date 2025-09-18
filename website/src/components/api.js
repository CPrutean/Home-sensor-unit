// This file handles API requests to fetch sensor data from the server.
// Make sure the API server is running at the URL specified in the .env file
// Run the msg-handler-server with `python main.py` to start the API server

// Main function to fetch all data (communication units, sensor units, sensors, and readings)
export const fetchAllData = async () => {
    console.log('Fetching all sensor data...');
    console.log('Environment variables:', process.env);

    try {
        // Use the environment variable with the correct protocol
        const apiUrl = process.env.REACT_APP_API_ADDR;
        console.log('Using API URL:', apiUrl);

        // Fetch communication units
        const commUnitsResponse = await fetch(`${apiUrl}/communication_units/`);
        if (!commUnitsResponse.ok) {
            throw new Error(`HTTP error fetching communication units! Status: ${commUnitsResponse.status}`);
        }
        const commUnitsData = await commUnitsResponse.json();
        console.log('Communication units data fetched successfully:', commUnitsData);

        // Fetch sensor units
        const sensorUnitsResponse = await fetch(`${apiUrl}/sensor_units`);
        if (!sensorUnitsResponse.ok) {
            throw new Error(`HTTP error fetching sensor units! Status: ${sensorUnitsResponse.status}`);
        }
        const sensorUnitsData = await sensorUnitsResponse.json();
        console.log('Sensor units data fetched successfully:', sensorUnitsData);

        // Fetch sensors
        const sensorsResponse = await fetch(`${apiUrl}/sensors`);
        if (!sensorsResponse.ok) {
            throw new Error(`HTTP error fetching sensors! Status: ${sensorsResponse.status}`);
        }
        const sensorsData = await sensorsResponse.json();
        console.log('Sensors data fetched successfully:', sensorsData);

        // Fetch readings
        const readingsResponse = await fetch(`${apiUrl}/readings`);
        if (!readingsResponse.ok) {
            throw new Error(`HTTP error fetching readings! Status: ${readingsResponse.status}`);
        }
        const readingsData = await readingsResponse.json();
        console.log('Readings data fetched successfully:', readingsData);

        // Transform the data to match the expected format for the website
        const transformedData = transformAllData(commUnitsData, sensorUnitsData, sensorsData, readingsData);
        return transformedData;
    } catch (error) {
        console.error('Failed to fetch data:', error);
        console.error('Error details:', error.message);
        if (error.name === 'TypeError' && error.message.includes('Failed to fetch')) {
            console.error('Network error: Check if the API server is running and accessible');
        }
        // Throw the error to be handled by the component
        throw error;
    }
};

// For backward compatibility with existing code
export const fetchSensorData = async () => {
    console.log('Fetching sensor cluster data (legacy function)...');

    try {
        const allData = await fetchAllData();
        // Convert the hierarchical data to the flat structure expected by existing code
        const legacyData = allData.flatMap(commUnit => 
            commUnit.sensorUnits.map(sensorUnit => ({
                clusterId: sensorUnit.id,
                clusterName: sensorUnit.name,
                location: {
                    city: 'Unknown',
                    latitude: 0,
                    longitude: 0
                },
                sensors: sensorUnit.sensors
            }))
        );
        return legacyData;
    } catch (error) {
        console.error('Failed to fetch sensor data (legacy):', error);
        // Throw the error to be handled by the component
        throw error;
    }
};

// Helper function to transform all data into a hierarchical structure
// This function follows the structure defined by the toDict methods in main.py
const transformAllData = (commUnitsData, sensorUnitsData, sensorsData, readingsData) => {
    console.log('Transforming all data:');
    console.log('- Communication units:', commUnitsData);
    console.log('- Sensor units:', sensorUnitsData);
    console.log('- Sensors:', sensorsData);
    console.log('- Readings:', readingsData);

    // Check if we have the expected data structures
    if (!commUnitsData || !commUnitsData['Communication units']) {
        console.error('Unexpected communication units data format:', commUnitsData);
        throw new Error('Invalid data format: Communication units data is missing or malformed');
    }

    if (!sensorUnitsData || !sensorUnitsData['Sensor units']) {
        console.error('Unexpected sensor units data format:', sensorUnitsData);
        throw new Error('Invalid data format: Sensor units data is missing or malformed');
    }

    // Transform communication units
    // CommunicationUnit.to_dict() returns: { "ID": id, "Name": name, "Sensor units": [sensor_unit_ids] }
    return commUnitsData['Communication units'].map(commUnit => {
        // Find sensor units for this communication unit
        const commUnitSensorUnits = sensorUnitsData['Sensor units'].filter(
            sensorUnit => commUnit['Sensor units'].includes(sensorUnit.ID)
        );

        // Transform sensor units
        // SensorUnit.to_dict() returns: { "ID": id, "Name": name, "Status": status, "Error Code": error_code, "Sensors": [sensor_names] }
        const transformedSensorUnits = commUnitSensorUnits.map(sensorUnit => {
            // Find sensors for this sensor unit
            const unitSensors = [];

            // If the sensor unit has sensors, process them
            if (sensorUnit.Sensors && Array.isArray(sensorUnit.Sensors)) {
                sensorUnit.Sensors.forEach(sensorName => {
                    // Find the sensor details from the sensors data
                    const sensorDetails = sensorsData.Sensors ? 
                        sensorsData.Sensors.find(s => s.Name === sensorName) : null;

                    // Find readings for this sensor
                    const sensorReadings = readingsData && Array.isArray(readingsData) 
                        ? readingsData.filter(reading => reading.Sensor === sensorName)
                        : [];

                    if (sensorReadings.length > 0) {
                        // Use the most recent reading
                        const latestReading = sensorReadings.reduce((latest, current) => {
                            return new Date(current.Timestamp) > new Date(latest.Timestamp) ? current : latest;
                        }, sensorReadings[0]);

                        // Determine status based on reading value (this is a simple example)
                        let status = 'OK';
                        if (latestReading.Values && latestReading.Values.length > 0) {
                            const value = parseFloat(latestReading.Values[0]);
                            // This is a simple example - you might want to adjust thresholds based on sensor type
                            if (value > 80) status = 'Critical';
                            else if (value > 60) status = 'Warning';
                        }

                        // Create a sensor object that matches the website's expected format
                        // but includes all relevant information from the server
                        unitSensors.push({
                            id: `${sensorUnit.ID}-${sensorName}`,
                            name: sensorName,
                            value: latestReading.Values && latestReading.Values.length > 0 
                                ? parseFloat(latestReading.Values[0]) 
                                : 0,
                            values: latestReading.Values,
                            unit: latestReading.Value || latestReading.Reading || '',
                            status: status,
                            timestamp: latestReading.Timestamp || new Date().toISOString(),
                            // Include additional information from the sensor if available
                            commands: sensorDetails ? sensorDetails.Commands : [],
                            responses: sensorDetails ? sensorDetails.Responses : []
                        });
                    }
                });
            }

            return {
                id: `unit-${sensorUnit.ID}`,
                name: sensorUnit.Name || `Sensor Unit ${sensorUnit.ID}`,
                status: sensorUnit.Status || 'Unknown',
                errorCode: sensorUnit['Error Code'] || '',
                sensors: unitSensors
            };
        });

        return {
            id: `comm-${commUnit.ID}`,
            name: commUnit.Name || `Communication Unit ${commUnit.ID}`,
            sensorUnits: transformedSensorUnits
        };
    });
};

// For backward compatibility - transform sensor units data to the old format
const transformSensorData = (sensorUnitsData, readingsData) => {
    console.log('Transforming sensor data (legacy function):', sensorUnitsData);
    console.log('Readings data:', readingsData);

    // Check if we have the expected data structure
    if (!sensorUnitsData || !sensorUnitsData['Sensor units']) {
        console.error('Unexpected sensor units data format:', sensorUnitsData);
        throw new Error('Invalid data format: Sensor units data is missing or malformed');
    }

    // Create clusters from sensor units
    return sensorUnitsData['Sensor units'].map(unit => {
        // Find readings for this unit's sensors
        const unitSensors = [];

        // If the unit has sensors, process them
        if (unit.Sensors && Array.isArray(unit.Sensors)) {
            unit.Sensors.forEach(sensorName => {
                // Find readings for this sensor
                const sensorReadings = readingsData && Array.isArray(readingsData) 
                    ? readingsData.filter(reading => reading.Sensor === sensorName)
                    : [];

                if (sensorReadings.length > 0) {
                    // Use the most recent reading
                    const latestReading = sensorReadings.reduce((latest, current) => {
                        return new Date(current.Timestamp) > new Date(latest.Timestamp) ? current : latest;
                    }, sensorReadings[0]);

                    // Determine status based on reading value (this is a simple example)
                    let status = 'OK';
                    if (latestReading.Values && latestReading.Values.length > 0) {
                        const value = parseFloat(latestReading.Values[0]);
                        // This is a simple example - you might want to adjust thresholds based on sensor type
                        if (value > 80) status = 'Critical';
                        else if (value > 60) status = 'Warning';
                    }

                    unitSensors.push({
                        id: `${unit.ID}-${sensorName}`,
                        name: sensorName,
                        value: latestReading.Values && latestReading.Values.length > 0 
                            ? parseFloat(latestReading.Values[0]) 
                            : 0,
                        values: latestReading.Values,
                        unit: latestReading.Value || latestReading.Reading || '',
                        status: status,
                        timestamp: latestReading.Timestamp || new Date().toISOString()
                    });
                }
            });
        }

        return {
            clusterId: `cluster-${unit.ID}`,
            clusterName: unit.Name || `Sensor Unit ${unit.ID}`,
            location: {
                city: 'Unknown',
                latitude: 0,
                longitude: 0
            },
            sensors: unitSensors
        };
    });
};

export const fetchMostRecentReadings = async () => {
  try {
      // Use the environment variable with the correct protocol
      const apiUrl = process.env.REACT_APP_API_ADDR;
      console.log('Using API URL for readings:', apiUrl);
      const response = await fetch(`${apiUrl}/readings`);
      if (!response.ok) {
          throw new Error(`HTTP error! Status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Most recent readings fetched successfully:', data);

      // Transform the readings data to match the expected format for the website
      const transformedReadings = transformReadingsData(data);
      return transformedReadings;
  } catch (error) {
      console.error('Failed to fetch most recent readings:', error);
      console.error('Error details:', error.message);
      if (error.name === 'TypeError' && error.message.includes('Failed to fetch')) {
          console.error('Network error: Check if the API server is running and accessible');
      }
      // Instead of throwing the error, return an empty array to prevent UI disruption
      console.log('Unable to fetch recent readings, returning empty array.');
      return [];
  }
};

// Helper function to transform readings data to the format expected by the website
// This function follows the structure defined by the RecentReading.to_dict method in main.py
const transformReadingsData = (readingsData) => {
    console.log('Transforming readings data:', readingsData);

    if (!readingsData || !Array.isArray(readingsData)) {
        console.error('Unexpected readings data format:', readingsData);
        return [];
    }

    // RecentReading.to_dict() returns: { "Sensor": sensor, "Reading": reading, "Values": values, "Timestamp": timestamp, "Value": value }
    return readingsData.map(reading => {
        // Determine status based on reading value (simple example)
        let status = 'OK';
        if (reading.Values && reading.Values.length > 0) {
            const value = parseFloat(reading.Values[0]);
            // Adjust thresholds based on your requirements
            if (value > 80) status = 'Critical';
            else if (value > 60) status = 'Warning';
        }

        // Create a reading object that matches the website's expected format
        // but includes all relevant information from the server
        return {
            sensorId: `${reading.Sensor}-${reading.Reading}`,
            value: reading.Values && reading.Values.length > 0 ? parseFloat(reading.Values[0]) : 0,
            status: status,
            timestamp: reading.Timestamp || new Date().toISOString(),
            // Include the original fields from the server for reference
            sensor: reading.Sensor,
            reading: reading.Reading,
            values: reading.Values,
            // Include the unit from the Value field
            unit: reading.Value || reading.Reading
        };
    });
};
