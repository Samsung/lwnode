const lwnode = process.lwnode;
const port = process.lwnode.port;
const post_first = +process.argv[2];

lwnode.ref();

function getTimestamp() {
  const now = new Date();
  return now.toISOString().slice(14, 23);
}

if (post_first) {
  setImmediate(() => {
    console.log(`${getTimestamp()} JS ping`);
    lwnode.postMessage(`ping`);
  });
}

const stop_count = 10;
let count = 0;

lwnode.onmessage = async (event) => {
  if (stop_count <= ++count) {
    console.log(`${getTimestamp()} JS stop`);
    lwnode.unref();
    return;
  }

  if (event.data == 'ping') {
    console.log(`${getTimestamp()} JS pong`);
    lwnode.postMessage(`pong ${count}`);
  } else {
    console.log(`${getTimestamp()} JS ping`);
    lwnode.postMessage(`ping`);
  }
};
