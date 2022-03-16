/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var assert = require('assert');
var debug = console.log;

function shouldBeFalse(arg) {
    var r = eval(arg);
    return assert(!r);
}

function shouldBeTrue(arg) {
    var r = eval(arg);
    return assert(r);
}

debug('tizen.time.getCurrentDateTime: ' + tizen.time.getCurrentDateTime().toString());
debug('tizen.time.getLocalTimezone: ' + tizen.time.getLocalTimezone());
debug('tizen.time.getAvailableTimezones: ' + tizen.time.getAvailableTimezones());
debug('tizen.time.getDateFormat: ' + tizen.time.getDateFormat());
debug('tizen.time.getTimeFormat: ' + tizen.time.getTimeFormat());

shouldBeFalse("tizen.time.isLeapYear(2013)");
shouldBeFalse("tizen.time.isLeapYear(2014)");
shouldBeFalse("tizen.time.isLeapYear(2015)");
shouldBeTrue("tizen.time.isLeapYear(2016)");
shouldBeFalse("tizen.time.isLeapYear(2017)");
shouldBeFalse("tizen.time.isLeapYear(2018)");
shouldBeFalse("tizen.time.isLeapYear(2019)");
shouldBeTrue("tizen.time.isLeapYear(2020)");
shouldBeFalse("tizen.time.isLeapYear(2021)");
shouldBeFalse("tizen.time.isLeapYear(2022)");
shouldBeFalse("tizen.time.isLeapYear(2023)");

var now = tizen.time.getCurrentDateTime();
debug('Now = ' + now);

var one_day = new tizen.TimeDuration(24, 'HOURS');
debug('One day = ' + one_day);

var tomorrow = now.addDuration(one_day);
debug('Tomorrow = ' + tomorrow);

var two_days = new tizen.TimeDuration(2, 'DAYS');
debug('Two days = ' + two_days);

var minus_one_day = new tizen.TimeDuration(-24, 'HOURS');
var yesterday = now.addDuration(minus_one_day);
debug('Yesterday = ' + yesterday);

var day_after_tomorrow = now.addDuration(two_days);
debug('Day after tomorrow = ' + day_after_tomorrow);

var tomorrow_plus_one_day = tomorrow.addDuration(one_day);
debug('Tomorrow plus one day = ' + tomorrow_plus_one_day);

shouldBeTrue("day_after_tomorrow.equalsTo(tomorrow_plus_one_day)");
shouldBeFalse("day_after_tomorrow.earlierThan(tomorrow_plus_one_day)");
shouldBeFalse("day_after_tomorrow.laterThan(tomorrow_plus_one_day)");

shouldBeTrue("tomorrow.laterThan(now)");
shouldBeFalse("tomorrow.earlierThan(now)");
shouldBeFalse("tomorrow.equalsTo(now)");

shouldBeFalse("yesterday.equalsTo(now)");
shouldBeFalse("now.earlierThan(yesterday)");
shouldBeTrue("now.laterThan(yesterday)");

process.on('SIGINT', () => {
    console.log('SIGINT');
});
