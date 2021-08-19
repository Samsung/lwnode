setTimeout(() => {
  console.log(`RSS: ${process.lwnode.RssUsage()} kB`);
  console.log(`PSS+SWAP: ${process.lwnode.PssUsage()} kB`);
  console.log(`dump: ${process.lwnode.MemSnapshot()}`);
}, 1000);
