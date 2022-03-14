## How to package lightweight node.js project

### Amend project name
You need to change the project name in the file below.
+ `Name` in `packaging/helloworld.spec`
+ app path in `packaging/helloworld.service` 
  (ExecStart=/usr/bin/lwnode /usr/apps/`{project-name}`/script)
+ `project` in `CMakeLists.txt`

### Work JavaScript
You can work on your JavaScript code in `lib` folder.

### Copy lightweight node binary
You should copy node binary file to bin folder.
```bash
cd <project_path>
cp <node_file_path> ./bin
```

### Package rpm
To package rpm, you should build gbs.
For example, 
``` bash
gbs -c ~/gbs.conf build -A armv7l -B ~/GBS-ROOT/helloworld --incremental --include-all
```

### Include native module (napi)
If you want to include native-module, work on the native code in `src` folder.
You can also implement build script in `CMakeLists.txt`.
To package rpm with natvie module, add config `--define='native_module true'` when you build gbs.
On linux, you can use command `npm run build` to build native module.

### Show log
To view node console log, use `dlogutil NODE_CONSOLE` command.
