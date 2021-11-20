/**
 * This file start the server. esp32 will connect to this server and download the ota file to 
 */


const https = require('https');
const fs = require('fs');
var path = require('path');

const options = {
  key: fs.readFileSync('ca_key.pem'),
  cert: fs.readFileSync('ca_cert.pem')
};

https.createServer(options, function (req, res) {
    var filePath = path.join(__dirname, 'build/native_ota.bin');
    // var stat = fs.statSync(filePath);
    // console.dir(stat);
  res.writeHead(200);
  var readStream = fs.createReadStream(filePath);
  readStream.pipe(res);
}).listen(8070);