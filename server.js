const express = require('express');
const path = require('path');
const app = express();
const PORT = 3000;

app.use(express.json());
app.use(express.static('public'));

// In-memory state
let journeyState = {
    active: false,
    currentDistance: 0,
    targetDistance: 0,
    path: [], // Array of {lat, lng}
    pathIndex: 0,
    vibration: false,
    reached: false,
    coordinates: { lat: 0, lng: 0 } 
};

// Simulation loop
setInterval(() => {
    if (journeyState.active && !journeyState.reached && journeyState.path.length > 0) {
        // Increment speed (move through the path array)
        // Adjust index based on simulated speed
        journeyState.pathIndex += 1; 

        if (journeyState.pathIndex >= journeyState.path.length) {
            journeyState.pathIndex = journeyState.path.length - 1;
            journeyState.reached = true;
            journeyState.vibration = false;
            journeyState.currentDistance = journeyState.targetDistance;
        } else {
            const currentPoint = journeyState.path[journeyState.pathIndex];
            journeyState.coordinates.lat = currentPoint.lat;
            journeyState.coordinates.lng = currentPoint.lng;

            // Calculate current distance from start along the path
            // (Simple approximation: ratio of index to length)
            journeyState.currentDistance = (journeyState.pathIndex / journeyState.path.length) * journeyState.targetDistance;

            // Alert when near end (e.g., last 10% or last few points)
            const remainingRatio = 1 - (journeyState.pathIndex / journeyState.path.length);
            if (remainingRatio <= 0.1 && remainingRatio > 0) {
                journeyState.vibration = true;
            }
        }
    }
}, 300); // Slightly faster tick for smoother movement on paths

app.post('/api/start-journey', (req, res) => {
    const { startCoords, endCoords, path } = req.body;
    
    if (!startCoords || !endCoords || !path) {
        return res.status(400).json({ error: 'Start, End coordinates and path required' });
    }

    // Helper to calculate total path distance
    const getPathDistance = (pts) => {
        let dist = 0;
        const R = 6371;
        for (let i = 0; i < pts.length - 1; i++) {
            const lat1 = pts[i].lat, lon1 = pts[i].lng;
            const lat2 = pts[i+1].lat, lon2 = pts[i+1].lng;
            const dLat = (lat2 - lat1) * Math.PI / 180;
            const dLon = (lon2 - lon1) * Math.PI / 180;
            const a = Math.sin(dLat/2) * Math.sin(dLat/2) +
                      Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) * 
                      Math.sin(dLon/2) * Math.sin(dLon/2);
            dist += R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
        }
        return dist;
    };

    journeyState = {
        active: true,
        currentDistance: 0,
        targetDistance: getPathDistance(path),
        path: path,
        pathIndex: 0,
        vibration: false,
        reached: false,
        coordinates: { ...startCoords }
    };

    res.json({ message: 'Journey started', state: journeyState });
});

app.get('/api/location', (req, res) => {
    res.json(journeyState);
});

app.post('/api/reset', (req, res) => {
    journeyState.active = false;
    journeyState.reached = false;
    res.json({ message: 'Reset successful' });
});

app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});
