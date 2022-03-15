function log (text) {
  document.querySelector('#textbox').innerHTML += `\n${text}`;
}

async function initTizen () {
  const dir = await resolve('wgt-package/res');
  const files = await listFiles(dir);
  await Promise.all(files.map(async file => {
    await copyFile(dir, file, '/tmp/');
  }))
  .catch(error => {
    log(error.message);
  });
}

function resolve (path) {
  return new Promise((resolve, reject) => {
    tizen.filesystem.resolve(path, file => resolve(file), error => reject(error), 'r');
  });
}

function listFiles (directory) {
  return new Promise((resolve, reject) => {
    directory.listFiles(files => resolve(files), error => reject(error));
  });
}

function copyFile (dir, file, dest) {
  return new Promise((resolve, reject) => {
    dir.copyTo(
      file.fullPath,
      dest,
      true,
      () => {
        log(`${file.name} copied to ${dest}`);
        resolve();
      },
      error => {
        log(`Cannot copy ${file.name}: ${error.message}`);
        reject(error);
      }
    )
  });
}