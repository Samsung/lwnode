## How to package lightweight node.js project

### Amend project name
You need to change the project name in the file below.
+ `name` in `package.json`
+ `package` and `app-id` in `tizen-manifest.xml`
+ `Name` in `packaging/helloworld.spec`
+ `project` in `CMakeLists.txt`

### Copy lightweight node binary
You should copy node binary file to bin folder.
```bash
cd <project_path>
cp <node_file_path> ./bin
```

### Work JavaScript
You can work on your JavaScript code in `lib` folder.

### Package tpk
To package tpk, you should build gbs.
For example, 
``` bash
gbs -c ~/gbs.conf build -A armv7l -B ~/GBS-ROOT/helloworld --incremental --include-all
```
After build, you can find tpk in `out` folder.

### Include native module (napi)
If you want to include native-module, work on the native code in `src` folder.
You can also implement build script in `CMakeLists.txt`.
To package tpk with natvie module, add config `--define='native_module true'` when you build gbs.
On linux, you can use command `npm run build` to build native module.

### Show log
To view node console log, use `dlogutil <app-id>` command.
For example,
``` bash
dlogutil helloworld
```
