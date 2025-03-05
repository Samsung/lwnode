
### Copy lightweight node header and library
 - copy `liblwnode.so` to `lib` folder
 - copy `libescargot.so` to `lib` folder
 - copy `lwnode.dat` to `bin` folder
 - copy `lwnode-public.h` to `include` folder
 - copy `message-port.h` to `include` folder
```bash
cd <app_project_root>
cp <lwnode_root>/out/tizen/arm/Release/lib/liblwnode.so ./lib/
cp <lwnode_root>/out/tizen/arm/Release/gen/escargot/libescargot.so ./lib/
cp <lwnode_root>/out/tizen/arm/Release/lwnode.dat ./bin/
cp <lwnode_root>/include/lwnode/lwnode-public.h ./include/
cp <lwnode_root>/include/lwnode/message-port.h ./include/
```

### Copy certification profile files
 - If you want to sign your app, copy some files to `packaging` folder.
```bash
cp tizen_author.p12 ./packaging/
cp tizen-distributor-partner-manufacturer-signer.p12 ./packaging/
```

### Edit packaging/lwnode-example-messageport.spec
 - You should input signer url in spec file.
 ```
 %define _signer_url http://x.x.x.x/kuep_net_signer.sh
 ```

### Package tpk
 - To package tpk, you should build gbs.
For example, 
``` bash
gbs -c ~/gbs.conf build -A armv7l -B ~/GBS-ROOT/helloworld --incremental --include-all
```
After build, you can find tpk file in `out` folder.


### Show log
 - To view node console log, use `dlogutil lwnode` command.
``` bash
dlogutil lwnode
```
