const lwnode = process.lwnode;
const port = process.lwnode.port;

lwnode.ref();


let count = 0;

function test(timeout = 1000) {
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      resolve(`test: ${count++} abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz`);
      }, timeout);
  });
}
  
port.onmessage = async (event) => {
  console.log(`js: ${event.data}`);
  if (event.data == "sync") {
    let data = await test();
    event.setResult(data);
  } else if (event.data == "async") {
    port.postMessage("async result from js");
  } else if (event.data == "delay-timeout") {
    let data = await test(5000); // long delay.
    event.setResult(data);
  } else if (event.data == "sync-timeout") {
    // Do not intentionally set the result value to test the timeout.
  } else if (event.data == "exit") {
    port.postMessage("exit javascript");
    lwnode.unref();
  }
};

