const http = require('http');
const fs = require('fs');
const { exec } = require('child_process');

const server = http.createServer((req, res) => {
    if (req.method === 'POST') {
        exec('./stuManagement.exe', (error, stdout, stderr) => {
            if (error) {
                res.end('Error running the C++ program.');
            } else {
                res.end(stdout);
            }
        });
    } else {
        fs.readFile('index.html', (err, data) => {
            if (err) {
                res.writeHead(404);
                res.end('Page not found.');
            } else {
                res.writeHead(200, { 'Content-Type': 'text/html' });
                res.end(data);
            }
        });
    }
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});