let i = 0;

let id = setInterval(() => {
  if (++i > 10000) {
    clearInterval(id);
  }
  console.log(`heartbeat #${i}`);
}, 1000);
