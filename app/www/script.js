// A-EYE Control Panel JavaScript

// Update system status from API
async function updateStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();
        
        document.getElementById('system-status').textContent = data.status;
        document.getElementById('uptime').textContent = formatUptime(data.uptime);
        document.getElementById('memory').textContent = data.memory;
        document.getElementById('last-update').textContent = new Date().toLocaleString();
        
        // Update status color
        const statusElement = document.getElementById('system-status');
        statusElement.className = `status-value ${data.status.toLowerCase()}`;
        
    } catch (error) {
        console.error('Failed to update status:', error);
        document.getElementById('system-status').textContent = 'Error';
        document.getElementById('system-status').className = 'status-value offline';
    }
}

// Format uptime in seconds to readable format
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    
    if (days > 0) return `${days}d ${hours}h ${minutes}m`;
    if (hours > 0) return `${hours}h ${minutes}m`;
    return `${minutes}m`;
}

// Add log message
function addLogMessage(message) {
    const logOutput = document.getElementById('log-output');
    const timestamp = new Date().toISOString().slice(0, 19).replace('T', ' ');
    const logEntry = document.createElement('p');
    logEntry.textContent = `[${timestamp}] ${message}`;
    logOutput.appendChild(logEntry);
    logOutput.scrollTop = logOutput.scrollHeight;
}

// Control functions
function startMonitoring() {
    addLogMessage('Starting monitoring system...');
    // Add your monitoring logic here
    setTimeout(() => {
        addLogMessage('Monitoring system started successfully');
    }, 1000);
}

function openSettings() {
    addLogMessage('Opening settings panel...');
    // Add your settings logic here
    alert('Settings panel - Feature coming soon!');
}

function stopSystem() {
    if (confirm('Are you sure you want to stop the system?')) {
        addLogMessage('Stopping system...');
        // Add your stop logic here
        setTimeout(() => {
            addLogMessage('System stopped');
        }, 1000);
    }
}

// Initialize page
document.addEventListener('DOMContentLoaded', function() {
    addLogMessage('Web interface loaded');
    updateStatus();
    
    // Update status every 5 seconds
    setInterval(updateStatus, 5000);
});

// Add some interactive effects
document.querySelectorAll('.btn').forEach(button => {
    button.addEventListener('click', function() {
        this.style.transform = 'scale(0.95)';
        setTimeout(() => {
            this.style.transform = '';
        }, 150);
    });
});