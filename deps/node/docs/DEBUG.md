# How to debug lwnode

`lwnode` provides preliminary debugging capabilities via VS code. This document shows you how to debug a `hello.js`.

## 1. Setup VS Code for lwnode

1. Install Escargot Debug plugin.

```sh
$ git clone https://github.com/Samsung/escargot-vscode-extension.git
$ cd escargot-vs-code-extension
$ npm install
$ npm install -g vsce
$ npm run compile
$ vsce package

// escargot-vscode-extension.vsix is created
```

2. Install `escargot-vscode-extension.vsix`.
* Run VSCode
* Type `ctrl+shrt+p` and type `Extensions: Install from VSIX...`
* Select `escargot-vs-code-extension.vsix`


3. Add the following debug configuration to `.vscode/launch.js`.
```js
// launch.json
{
    ...
    "configurations": [
        ...
        {
            "name": "Escargot: Attach",
            "type": "escargot",
            "request": "attach",
            "address": "localhost",
            "port": 6501,
            "localRoot": "${workspaceRoot}",
            "debugLog": 4
        }
        ...
    ]
}
```

## 2. Compile lwnode with debugger support
Compile `lwnode` with a debugger flag, `--escargot-debugger`.
```sh
$ vi lwnode/build.sh
CONFIG="...
       --escargot-debugger \
       ..."
$ ./lwnode/build.sh -d
```

## 3. Run debugger with VS Code
1. Start a debug server on a new terminal.

```js
// hello.js
let sum = 0;
for (let i = 0; i < 10; i++) {
  sum += 1;
}
```

```sh
$ out/linux/Debug/lwnode --start-debug-server ./hello.js
```

lwnode will wait for a connection from VS code.


2. Open `hello.js` in VS Code and run debugger.

Known Issues:
* The debugger by default stops at every `*.js` file loaded before loading `hello.js`. This behaviour is expected to be fixed in the next version. (i.e., the `console.log()` function will call many subsequence JS functions calls.)
* To add a break point, wait until `hello.js` is loaded in VS code, and add a break point.
