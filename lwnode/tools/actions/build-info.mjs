import fs from 'fs';
import path from 'path';
import { execSync } from 'child_process';

function exec(cmd) {
  return execSync(cmd, { stdio: 'pipe' }).toString().trim();
}

function execEcho(cmd) {
  return console.log(execSync(cmd).toString().trim());
}

function getSectionInfo(path) {
  let sections = {};
  try {
    let section_size_output = exec(`size -A ${path}`);
    let filtered = section_size_output.split(/\s+/).slice(5, -2);

    for (let i = 0; i < filtered.length; i++) {
      let key = filtered[i++];
      let value = filtered[i++];
      // skip: let address = filtered[i];
      sections[key] = +value;
    }
  } catch (e) {
    return;
  }
  return sections;
}

if (process.argv.length <= 2) {
  console.log(
    'Usage: node [this-script.mjs] [path-including-binaries] <outputpath>',
  );
  process.exit(1);
}

const targetPath = path.resolve(process.argv.slice(2)[0]);
const files = ['lwnode', 'lwnode.dat', 'libescargot.so'];
const outputFilename = 'build-info.txt';

try {
  let outputSections = files
    .map((name) => {
      let sections = getSectionInfo(`${targetPath}/${name}`);
      let size = +exec(`du -b ${targetPath}/${name}`).split(/\s+/)[0];
      if (!sections) {
        return {
          name,
          size,
        };
      }
      return {
        name,
        size,
        bss: sections['.bss'],
        text: sections['.text'],
        data: sections['.data'],
        rodata: sections['.rodata'],
      };
    })
    .filter((item) => (item ? true : false));

  // https://git-scm.com/docs/pretty-formats
  let hash_short = exec('git log -1 --format=%h');
  let hash = exec('git log -1 --format=%H');
  let date = exec('git log -1 --format=%ci'); // %cd
  let branch = exec('git rev-parse --abbrev-ref HEAD');
  let message = exec('git log -1 --format=%s');
  let author = exec('git log -1 --format=%an');

  let buildInfo = {
    hash,
    hash_short,
    date,
    message,
    author,
    branch,
    outputs: outputSections,
  };

  let outputFile;
  if (process.argv.length > 3) {
    outputFile = path.resolve(process.argv.slice(2)[1]);
  } else {
    outputFile = targetPath;
  }
  outputFile += `/${outputFilename}`;

  fs.writeFileSync(
    outputFile,
    JSON.stringify(buildInfo, null, 1).concat('\n'),
    'utf8',
  );
  execEcho(`cat ${outputFile}`);
  console.log(`${outputFile} is created`);
} catch (e) {
  console.log(e);
  console.error(`Error: maybe there is no file on ${targetPath}`);
}
