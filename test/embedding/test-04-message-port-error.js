const lwnode = process.lwnode;
const port = process.lwnode.port;

lwnode.ref();

port.onmessage = (event) => {
  if (event.data == "ping") {
    port.postMessage("pong");
    lwnode.unref();
  }
};

error
