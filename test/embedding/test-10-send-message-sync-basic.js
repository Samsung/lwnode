const lwnode = process.lwnode;
const port = process.lwnode.port;

port.onmessage = (event) => {
    console.log(`${event.data}`);
    if (event.data == "ping") {
        port.postMessage("pong");
    }
};

function printMessage() {
    console.log("printMessage called--------------------------------------------");
    const name = lwnode.sendMessageSync('name');
    console.log(`Hello, ${name}!`);

    const age = lwnode.sendMessageSync('age');
    console.log(`I am ${age} years old.`);

    const gender = lwnode.sendMessageSync('gender');
    console.log(`My gender is ${gender}.`);
}

let count = 10;
let loop = setInterval(() => {
    if (count-- <= 0) {
        clearInterval(loop);
    }
    printMessage();
}, 1000);
