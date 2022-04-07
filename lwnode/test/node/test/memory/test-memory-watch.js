const { MemWatcher } = process.lwnode;

let theThing = null;
let closureLeak = function () {
  let originalThing = theThing; // leak
  let unused = function () {
    if (originalThing) console.log('hi');
  };
  theThing = {
    longStr: new Array(1000000).join('*'), // leak
    someMethod: function () {},
  };
};

console.warn('This code intentionally makes leaks');
const id = setInterval(closureLeak, 1000);

const watcher = new MemWatcher();
watcher.on('stats', (stats) => {
  console.log(new Date().toTimeString());
  console.table(stats);
});
watcher.on('max', (stats) => {
  console.log(stats);
});

setTimeout(() => {
  clearInterval(id);
  watcher.end();
}, 1000 * 60);
