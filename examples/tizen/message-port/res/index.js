console.log('[JS] started...');

const lwnode = process.lwnode;
const port = process.lwnode.port;

// Keep the process alive until we receive a message.
// lwnode is not terminated until lwnode.unref() is called, which decreases the reference count.
lwnode.ref();

// Listen for messages from the native app.
port.onmessage = (event) => {
    console.log(`[JS] receive ${event.data}`);
    if (event.data == "ping") {

        // send a response back to the native app. (c -> js)
        port.postMessage("pong");

        // Unref the process. If you want to keep it alive, remove this line.
        lwnode.unref();
    }
};
