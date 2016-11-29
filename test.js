const http = require('http');

var server = http.createServer((req, res) => {
  res.writeHead(200, {"Content-Type": 'text/plain'});
  res.end("Hello, World!");
});


console.log("Node.js server listening on 3002.");
server.listen(3002);

