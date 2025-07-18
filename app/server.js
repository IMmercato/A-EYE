const express = require('express');
const http = require('http');
const apiRoutes = require('./routes/routes')

const app = express();
app.use(express.static('www'));

const server = http.createServer(app);

app.get('/', async (req, res) => {
    res.sendFile(__dirname + '/www/index.html');
});

app.use('/api', apiRoutes);

app.use((req, res, next) => {
    res.status(404).sendFile(__dirname + '/www/404.html');
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});