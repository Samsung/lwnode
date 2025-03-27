const lwnode = process.lwnode;
const port = process.lwnode.port;

lwnode.ref();

port.onmessage = (event) => {
    console.log(`${event.data}`);
    if (event.data == "ping") {
        port.postMessage("pong");
        lwnode.unref();
    }
};

function printMessage() {
    console.log("printMessage called--------------------------------------------");
    const name = lwnode.binding('name');
    console.log(`Hello, ${name}!`);

    const age = lwnode.binding('age');
    console.log(`I am ${age} years old.`);

    const gender = lwnode.binding('gender');
    console.log(`My gender is ${gender}.`);
}

let count = 10;
let loop = setInterval(() => {
    if (count-- <= 0) {
        clearInterval(loop);
    }
    printMessage();
}, 1000);
