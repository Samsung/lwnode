const { MemWatcher } = process.lwnode;

const watcher = new MemWatcher();

watcher.on("stats", (stats) => {
  console.log(new Date().toTimeString());
  console.table(stats);
});

setTimeout(() => {
  watcher.end();
}, 1000 * 20);
