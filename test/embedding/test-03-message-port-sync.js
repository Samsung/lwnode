const lwnode = process.lwnode;
const port = process.lwnode.port;

lwnode.ref();


let count = 0;

function test() {
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      resolve(`sync test: ${count++}`);
      }, 1000);
  });
}
  
port.onmessage = async (event) => {
  console.log(`js: ${event.data}`);
  if (event.data == "sync") {
    let data = await test();
    event.setResult(data);
  } else if (event.data == "exit") {
    port.postMessage("exit javascript");
    lwnode.unref();
  } else if (event.data == "sync-timeout") {
    // Do not intentionally set the result value to test the timeout.
  }
};
