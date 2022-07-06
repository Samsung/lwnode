# Service App Guide

In this guide, we will show you how to write a `hello world`  service app.

## 1. Create a service app
You can start developing a service app by using a service app [template](../modules/apps/template/).

Copy the template, and start editing `lib/index.js`. The template contains [code](../modules/apps/template/lib/index.js) to print `hello world`.

## 2. Compile and run the service app on Linux

You can decide which `lwnode` executable to use when running a service app. For example, you can either use `lwnode` preinstalled on a Tizen platform, or use `lwnode` executable included when the service app is packaged. The latter approach allows you to use a `lwnode` executable specific for each service app.

First, add a `lwnode` executable to your app.

```sh
mkdir bin
cd bin

# Use the preinstalled lwnode on Tizen
ln -s /usr/bin/lwnode {lwnode path}/lwnode

# OR Include lwnode for this service
cp lwnode {lwnode path}/lwnode
```

Then, install `Express.js` module.
```sh
npx pnpm i express
```

While developing, you can run the service app on Linux as follows.

```sh
./bin/lwnode lib
```

## 3. Build Tizen RPMs

### 3.1 Create a RPM package
Build Tizen RPMs as follows.

```sh
gbs -c .github/gbs.conf build \
    -A armv7l \
    -P profile.t70std \
    -B ~/GBS-ROOT/helloworld \
    --include-all --incremental
```

In this example, we use `~/GBS-ROOT/helloworld` as our output directory. Please note that `gbs` only works if the current directory is under a git repo, i.e., `gbs` searches for `.git` directory in your project root.


### 3.2 Install the RPM package

Install the rpm package on a target device. You can find the package in the following directory.

```sh
~/GBS-ROOT/helloworld/local/repos/t65std/armv7l/RPMS/helloworld-1.0.0-1.armv7l.rpm
```

Install the package on a target device
```
rpm -Uvh --nodeps --force helloworld-1.0.0-1.armv7l.rpm
```


## 4. Connect to the Web service
Open `http://{target device IP}:3000` on a Web browser to see the service running. In this example, `hello world` should be printed on the browser.
