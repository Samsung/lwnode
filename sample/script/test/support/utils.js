const variables = require('./variables');
const url = `http://${variables.DB_HOSTNAME}:${variables.DB_PORT}/api`;

/* istanbul ignore next */
exports.requestAPI = async function (api, name) {
  let method = 'GET';
  if (api === 'delete') {
    method = 'DELETE';
  }

  let response = await fetch(`${url}/${api}/${name}`, {
    method: method,
  });
  if (!response.ok) {
    throw new Error('Cannot response server-api!');
  }
  return await response.text();
};

/* istanbul ignore next */
exports.timeout = function (ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
};

/* istanbul ignore next */
exports.log = function (text) {
  document.querySelector('#textbox').innerHTML += `\n${text}`;
}
