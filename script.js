const http = require('http');
const fs = require('fs');
const { exec } = require('child_process');

const MAX_REQUESTS_PER_MINUTE = 100;
const requestCounts = {};

const server = http.createServer((req, res) => {
    try {
        const ip = req.socket.remoteAddress;
        requestCounts[ip] = (requestCounts[ip] || 0) + 1;

        if (requestCounts[ip] > MAX_REQUESTS_PER_MINUTE) {
            res.writeHead(429, { 'Content-Type': 'text/plain' });
            res.end('Too Many Requests');
            return;
        }

        if (req.method === 'POST') {
            if (req.headers['content-type'] !== 'application/json') {
                res.writeHead(400, { 'Content-Type': 'text/plain' });
                res.end('Bad Request');
                return;
            }

            let requestBody = '';
            req.on('data', (chunk) => {
                requestBody += chunk.toString();
            });

            req.on('end', () => {
                try {
                    const requestData = JSON.parse(requestBody);

                    exec('./stuManagement.exe', (error, stdout, stderr) => {
                        if (error) {
                            console.error('Error executing C++ program:', error);
                            res.writeHead(500, { 'Content-Type': 'text/plain' });
                            res.end('Internal Server Error');
                        } else {
                            res.writeHead(200, { 'Content-Type': 'application/json' });
                            res.end(JSON.stringify({ output: stdout }));
                        }
                    });
                } catch (err) {
                    console.error('Error parsing JSON:', err);
                    res.writeHead(400, { 'Content-Type': 'text/plain' });
                    res.end('Bad Request');
                }
            });
        } else if (req.method === 'GET') {
            fs.readFile('index.html', (err, data) => {
                if (err) {
                    console.error('Error reading index.html:', err);
                    res.writeHead(404, { 'Content-Type': 'text/plain' });
                    res.end('Page not found.');
                } else {
                    res.writeHead(200, { 'Content-Type': 'text/html' });
                    res.end(data);
                }
            });
        } else {
            res.writeHead(405, { 'Content-Type': 'text/plain' });
            res.end('Method Not Allowed');
        }
    } catch (err) {
        console.error('Error processing request:', err);
        res.writeHead(500, { 'Content-Type': 'text/plain' });
        res.end('Internal Server Error');
    }
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});
