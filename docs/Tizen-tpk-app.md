# How to make tpk for Tizen App

In this guide, we will show you how to write a `hello world` tizen tpk app.

## 1. Create a tpk app with Tizen CLI

### Create project

Create a project with the command below.
`tizen create native-project -n <app-name> -p <profile> -t ServiceApp`

For example,
```sh
tizen create native-project -p iot-headless-6.5 -t ServiceApp -n "helloworld"
```

### Make index.js in 'res' folder

Make entry point JavaScript file(`index.js`) in `res` foler to start lwnode.

For example,
```js
// res/index.js
console.log('hello world');
```

### Modify `tizen-manifest.xml`

To start lwnode for executable file, modify `exec` path in `tizen-mainifest.xml`.

```
<service-application appid="org.example.hello" exec="/usr/bin/lwnode" ...>
```


## 2. Package App

Use the command below to package the app.

```sh
mkdir -p .buildResult
zip ./.buildResult/<app-id>.tpk * -r
tizen package -t tpk -- ./.buildResult/<app-id>.tpk
```

## 3. Install and Run App on device

After moving the tpk file to the device, install and run it.
And then check the console log using `dlogutil <app-id>`.

```sh
dlogutil <app-id>
```
