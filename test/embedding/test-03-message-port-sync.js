const lwnode = process.lwnode;
const port = process.lwnode.port;

lwnode.ref();

let count = 0;

port.onmessage = (event) => {
  console.log(`js: ${event.data}`);
  if (event.data == "sync") {
    event.setResult(`sync test: ${count++}`);
  } else if (event.data == "exit") {
    port.postMessage("exit javascript");
    lwnode.unref();
  } else if (event.data == "sync-timeout") {
    // Do not intentionally set the result value to test the timeout.
  }
};
