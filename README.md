# spur-wasm
Compile SPUR to WebAssembly using emscripten.

This is a fork of [SPUR](https://github.com/ZaydH/spur) by Zayd Hammoudeh.

Tested on Ubuntu 16.04 LTS.

## Prerequisites
### emscripten
Follow the instructions on https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html in order to install emscripten and all of its dependencies.

### GMP
* xz-utils (to unpack GMP sources)

### For Demo Web App
* [browserify](https://www.npmjs.com/package/browserify)
* [http-server](https://www.npmjs.com/package/http-server)

## Building
Run `build.sh` to compile SPUR using emscripten as well as to build the JavaScript modules (see *Usage*).

Run `build.sh demo` to additionally build and run a demo web app listening on http://localhost:8080.

## Usage
emscripten generates the CommonJS module `release/spur.js`. Additionally there is a wrapper module `release/Wrapper.js` that is recommended to use, since it does some optimization regarding memory consumption.

```js
const SPUR = require("./Wrapper.js");

SPUR.setModule(require("./spur.js"));
SPUR.setWASM("./spur.wasm");

SPUR.run("p cnf 5 3 [...]", ["-count-only"]).then(result => {
	console.log(result);
}).catch(reason => {
	console.error(reason);
});
```

## Original License
Copyright (c) 2018 Zayd Hammoudeh & Dimitris Achlioptas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
