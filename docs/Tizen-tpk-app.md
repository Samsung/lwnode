# Tizen Service App Guide

In this guide, we will show you how to write a `hello world` tpk (Tizen Package) using `lwnode`.

## 1. Generate a Tizen native app project using Tizen CLI

### Create a project

Create a project with the command below.

`tizen create native-project -n <app-name> -p <profile> -t ServiceApp`

For example,
```console
tizen create native-project -p iot-headless-6.5 -t ServiceApp -n "helloworld"
```

### Add `index.js` in the `res` folder

There will be a `res` folder in your project. It's the place where your code should be. 

Create a JavaScript file named `index.js` in the folder. It will be an entry point for your app.

For example,
```js
// path: res/index.js
console.log('hello world');
```

### Configure your application information

`tizen-manifest.xml`, the application manifest, consists of application information, such as 
package, privileges, and version which are available for your application. To run your code on `lwnode`, 
you need to write an executable file path as below.

```xml
<!-- add `/usr/bin/lwnode` into the `exec` attribute. -->
<service-application appid="org.example.hello" exec="/usr/bin/lwnode" ...>
```


## 2. Create a tpk (Tizen Package)

Use the commands below to pack your app as a tpk.

```console
mkdir -p .buildResult
zip ./.buildResult/<app-id>.tpk * -r
tizen package -t tpk -- ./.buildResult/<app-id>.tpk
```

## 3. Install and run the app on a device

Install the Tizen package on a target device and run it.
You can find the console log, `hello world`, using `dlogutil <app-id>`.

```console
dlogutil <app-id>
```
