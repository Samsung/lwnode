# How to build lwnode

## Supported Platforms
* Ubuntu 18.04, 16.04
* Tizen 4.0 and above

## Checkout code
```
git clone git@github.sec.samsung.net:lws/node-escargot.git
git submodule update --init --recursive
```

## How to Compile: Ubuntu
### Install required packages
```
sudo apt-get install -y build-essential cmake clang libicu-dev
```

### How to build and run
```
./lwnode/build.sh
./out/linux/Release/lwnode ./test/message/hello_world.js
```

### How to run testcases
```
./lwnode/build-cctest.sh
./cctest
```

## How to Compile: Tizen
### Prerequisite
Set up a gbs build environment on a Ubuntu machine.

### Build with gbs command
You can also build by using the gbs command without using the build util.
When using the gbs command, you need to defien a profile.
```
gbs -c ~/gbs.conf build -A arm7l
```

### Installing lwnode executable
Install `lwnode-devel.rpm` to get the `lwnode` executable.
