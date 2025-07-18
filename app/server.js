const express = require('express');
const cors = require('cors');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json({ limit: '10mb' })); // Increase limit for base64 images
app.use(express.static('public'));

// Create uploads directory if it doesn't exist
const uploadsDir = path.join(__dirname, 'uploads');
if (!fs.existsSync(uploadsDir)) {
    fs.mkdirSync(uploadsDir);
}

// Mock data for responses
const mockFaces = [
    { name: "Alice", confidence: 0.95 },
    { name: "Bob", confidence: 0.87 },
    { name: "Charlie", confidence: 0.92 },
    { name: "Diana", confidence: 0.89 }
];

const mockObjects = [
    { name: "coffee cup", confidence: 0.85 },
    { name: "laptop", confidence: 0.78 },
    { name: "phone", confidence: 0.92 },
    { name: "book", confidence: 0.76 },
    { name: "water bottle", confidence: 0.83 },
    { name: "pen", confidence: 0.71 },
    { name: "chair", confidence: 0.89 },
    { name: "table", confidence: 0.94 }
];

const mockContexts = [
    "You're in an office environment",
    "Looks like a coffee break",
    "Study session in progress",
    "Meeting room setting",
    "Working from home setup",
    "Outdoor environment detected",
    "Kitchen area identified",
    "Living room space"
];

// Helper function to get random items from array
function getRandomItems(array, count) {
    const shuffled = [...array].sort(() => 0.5 - Math.random());
    return shuffled.slice(0, count);
}

// Helper function to save image (optional for debugging)
function saveImage(base64Data, filename) {
    try {
        const buffer = Buffer.from(base64Data, 'base64');
        const filepath = path.join(uploadsDir, filename);
        fs.writeFileSync(filepath, buffer);
        console.log(`Image saved: ${filepath}`);
    } catch (error) {
        console.error('Error saving image:', error);
    }
}

// Main analysis endpoint
app.post('/analyze', (req, res) => {
    console.log('Received analysis request');
    
    try {
        const { image, timestamp, device_id } = req.body;
        
        if (!image) {
            return res.status(400).json({ error: 'No image data provided' });
        }
        
        console.log(`Processing image from device: ${device_id} at ${timestamp}`);
        
        // Optionally save the image for debugging
        if (process.env.SAVE_IMAGES === 'true') {
            const filename = `${device_id}_${timestamp}.jpg`;
            saveImage(image, filename);
        }
        
        // Simulate processing time
        setTimeout(() => {
            // Generate random mock response
            const shouldDetectFaces = Math.random() > 0.3; // 70% chance of faces
            const shouldDetectObjects = Math.random() > 0.2; // 80% chance of objects
            const shouldHaveUnknown = Math.random() > 0.7; // 30% chance of unknown faces
            
            const response = {
                status: 'success',
                timestamp: Date.now(),
                device_id: device_id,
                processing_time: Math.round(Math.random() * 2000 + 500), // 500-2500ms
                recognized_faces: [],
                unknown_faces: 0,
                objects: [],
                context: null
            };
            
            // Add recognized faces
            if (shouldDetectFaces) {
                const numFaces = Math.floor(Math.random() * 3) + 1; // 1-3 faces
                response.recognized_faces = getRandomItems(mockFaces, numFaces);
            }
            
            // Add unknown faces
            if (shouldHaveUnknown) {
                response.unknown_faces = Math.floor(Math.random() * 2) + 1; // 1-2 unknown
            }
            
            // Add objects
            if (shouldDetectObjects) {
                const numObjects = Math.floor(Math.random() * 4) + 1; // 1-4 objects
                response.objects = getRandomItems(mockObjects, numObjects);
            }
            
            // Add context
            if (Math.random() > 0.4) { // 60% chance of context
                response.context = mockContexts[Math.floor(Math.random() * mockContexts.length)];
            }
            
            console.log('Sending response:', JSON.stringify(response, null, 2));
            res.json(response);
            
        }, Math.random() * 1000 + 200); // 200-1200ms delay
        
    } catch (error) {
        console.error('Error processing request:', error);
        res.status(500).json({ 
            error: 'Internal server error',
            message: error.message 
        });
    }
});

// Health check endpoint
app.get('/health', (req, res) => {
    res.json({ 
        status: 'healthy', 
        timestamp: Date.now(),
        server: 'Mock AI Analysis Server'
    });
});

// Root endpoint with info
app.get('/', (req, res) => {
    res.json({
        message: 'Mock AI Analysis Server for ESP32 Glasses',
        version: '1.0.0',
        endpoints: {
            analyze: 'POST /analyze - Send base64 image for analysis',
            health: 'GET /health - Check server status'
        },
        usage: {
            image_format: 'base64 encoded JPEG',
            max_size: '10MB',
            response_format: 'JSON with faces, objects, and context'
        }
    });
});

// Error handling middleware
app.use((error, req, res, next) => {
    console.error('Unhandled error:', error);
    res.status(500).json({ 
        error: 'Internal server error',
        message: error.message 
    });
});

// Start server
app.listen(PORT, () => {
    console.log(`🚀 Mock AI Analysis Server running on port ${PORT}`);
    console.log(`📡 Ready to receive images from ESP32`);
    console.log(`🔗 Analyze endpoint: http://localhost:${PORT}/analyze`);
    console.log(`❤️  Health check: http://localhost:${PORT}/health`);
    
    if (process.env.SAVE_IMAGES === 'true') {
        console.log(`💾 Images will be saved to: ${uploadsDir}`);
    }
});

// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('Received SIGTERM, shutting down gracefully');
    process.exit(0);
});

process.on('SIGINT', () => {
    console.log('Received SIGINT, shutting down gracefully');
    process.exit(0);
});