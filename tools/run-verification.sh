#!/bin/bash

./node_modules/nyc/bin/nyc.js -x sample/script/test/support/helper.js ./node_modules/mocha/bin/mocha --timeout 480000 --exit sample/script/test/*.test.js

