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
