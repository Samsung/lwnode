# Node.js on Escargot

Node.js on Escargot is a memory efficient node.js implementation,
which runs on top of [Escargot](https://github.com/Samsung/escargot),
a memory optimized JavaScript Engine developed by Samsung Research,
instead of the default V8 JS engine.

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

### Compile
1. Generate build config files and build node.js
```
./lwnode/build.sh
./lwnode/build-cctest.sh
```

### How to run
```
./node ./test/message/hello_world.js
```

## How to Compile: Tizen
### Prerequisite
Set up a gbs build environment on a Ubuntu machine.

### Build with gbs command
You can also build by using the gbs command without using the build util.
When using the gbs command, you need to defien a profile.
```
gbs -c ~/gbs.conf build -A arm7l --define 'build_profile tv'
```
Build Options
* --define 'build_profile `none|tv|kiosk|soundbar`': default is `none`
* --define 'build_mode `release|debug`': default is `release`

## Maintainers
A list of maintainers can be found in [MAINTAINERS.md](MAINTAINERS.md).
