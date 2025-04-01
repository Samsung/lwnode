const lwnode = process.lwnode;
const http = require('http');

if (process.lwnode.hasSystemInfo('tizen')) {
    console.log('[JS] start gmain-loop')
    require("./gmain-loop").init();
}

console.log('[JS] start app')

console.log('[JS] create server')
const server = http.createServer();
server.listen(1234);

lwnode.ref();

setInterval(() => {
    console.log('[JS]loop');
}, 1000);
