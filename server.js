const express = require('express');
const path = require('path');
const app = express();
const PORT = 3000;

// Increase body size limit to handle large route paths (long distances)
app.use(express.json({ limit: '10mb' }));
app.use(express.static('public'));

// In-memory state
let journeyState = {
    active: false,
    currentDistance: 0,
    targetDistance: 0,
    path: [],
    pathIndex: 0,
    stepSize: 1,  // How many path points to skip per tick (scales with path length)
    vibration: false,
    reached: false,
    coordinates: { lat: 0, lng: 0 }
};

// Simulation loop — runs every 300ms
setInterval(() => {
    if (journeyState.active && !journeyState.reached && journeyState.path.length > 0) {
        // Move forward by stepSize (proportional to path length for any distance)
        journeyState.pathIndex += journeyState.stepSize;

        if (journeyState.pathIndex >= journeyState.path.length) {
            journeyState.pathIndex = journeyState.path.length - 1;
            journeyState.reached = true;
            journeyState.vibration = false;
            journeyState.currentDistance = journeyState.targetDistance;
        } else {
            const currentPoint = journeyState.path[journeyState.pathIndex];
            journeyState.coordinates.lat = currentPoint.lat;
            journeyState.coordinates.lng = currentPoint.lng;

            // Progress ratio
            journeyState.currentDistance = (journeyState.pathIndex / journeyState.path.length) * journeyState.targetDistance;

            // Vibrate alert when within last 10%
            const remainingRatio = 1 - (journeyState.pathIndex / journeyState.path.length);
            if (remainingRatio <= 0.1 && remainingRatio > 0) {
                journeyState.vibration = true;
            }
        }
    }
}, 300);

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
        // ~200 ticks at 300ms = 60 seconds total simulation. stepSize covers all points in that time.
        stepSize: Math.max(1, Math.ceil(path.length / 200)),
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
