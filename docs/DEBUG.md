# How to debug lwnode

`lwnode` provides preliminary debugging capabilities via VS code. This document shows you how to debug a `hello.js`.

We assume `lwnode` is checkouted out in the `lwnode` directory in the following code example. The default directory can be either `lwnode` or `node-escargot` depending on from which repo the source code has checkouted out.

## 1. Setup VS Code for lwnode

### 1.1. Install Escargot Debug plugin

```sh
$ git clone https://github.com/Samsung/escargot-vscode-extension.git
$ cd escargot-vs-code-extension
$ npm install
$ npm install -g vsce
$ npm run compile
$ vsce package

// escargot-vscode-extension.vsix is created
```

### 1.2. Install `escargot-vscode-extension.vsix`
```sh
$ cd lwnode
$ code .
```
Install the plugin as follows:
* Type `ctrl+shrt+p` and type `Extensions: Install from VSIX...`
* Select `escargot-vs-code-extension.vsix`


### 1.3. Add the following debug configuration to `.vscode/launch.json`

```sh
$ cd lwnode
$ vi .vscode/launch.json
```
```json
// .vscode/launch.json
{
    "version": "0.2.0",
    "configurations": [
        // Your existing targets may appear here.
        // Add the following target to your launch.json
        {
            "name": "Debug: lwnode",
            "type": "escargot",
            "request": "attach",
            "address": "localhost",
            "port": 6501,
            "localRoot": "${workspaceRoot}",
            "debugLog": 4
        }
    ]
}
```

## 2. Compile lwnode with debugger support
Compile `lwnode` with a debugger option.
```sh
$ cd lwnode
$ ./lwnode/build.sh -di
```

## 3. Run debugger with VS Code

### 3.1. Start a debug server on a new terminal

```sh
// Open a new terminal
$ cd lwnode
$ out/linux/Debug/lwnode --start-debug-server ./hello.js
```
A sample of `hello.js` is given below.
```js
// hello.js
let sum = 0;
for (let i = 0; i < 10; i++) {
  sum += 1;
}
```

`lwnode` will wait for a connection from VS code.

### 3.2. Open `hello.js` in VS Code and run debugger
```sh
$ cd lwnode
$ code .
```
On VS Code, run as follows:
* Open `hello.js` in VS Code
* Select `Run and Debug (ctrl+shift+D)` on the left panel
* Set target to `Debug: lwnode` found on the top
* Select `Run -> Start Debugging (F5)` on the menu


Known Issues:
* The debugger by default stops at every `*.js` file loaded before loading `hello.js`. This behaviour is expected to be fixed in the next version. (i.e., the `console.log()` function will call many subsequence JS functions calls.)
* To add a break point, wait until `hello.js` is loaded in VS code, and add a break point.
