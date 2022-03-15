const chalk = require('chalk');
const Moment = require('moment');

/* istanbul ignore next */
function getTime() {
  const now = new Moment();
  const time = chalk.dim(`[${now.format('HH:mm:ss')}]`);
  return time;
}

/* istanbul ignore next */
export function log(...message) {
  const time = getTime();
  const type = chalk.bold('[LOG]');
  console.log(`${time}${type}`, ...message);
}

/* istanbul ignore next */
log.info = (...message) => {
  const time = getTime();
  const type = chalk.bold(chalk.cyan('[INFO]'));
  console.info(`${time}${type}`, ...message);
};

/* istanbul ignore next */
log.warn = (...message) => {
  const time = getTime();
  const type = chalk.bold(chalk.yellow('[WARN]'));
  console.error(`${time}${type}`, ...message);
};

/* istanbul ignore next */
log.error = (...message) => {
  const time = getTime();
  const type = chalk.bold(chalk.red('[ERROR]'));
  console.error(`${time}${type}`, ...message);
};

// module.exports = log;
