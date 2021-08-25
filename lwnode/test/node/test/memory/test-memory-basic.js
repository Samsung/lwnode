function numberWithCommas(number) {
  return number.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}

setTimeout(() => {
  console.log(`RSS: ${numberWithCommas(process.lwnode.RssUsage())} kB`.padStart(20));
  console.log(`PSS: ${numberWithCommas(process.lwnode.PssUsage())} kB`.padStart(20));
  console.log(
    `PSS+SWAP: ${numberWithCommas(process.lwnode.PssSwapUsage())} kB`.padStart(20)
  );
  console.log(` dump: ${process.lwnode.MemSnapshot()}`);
}, 1000);
