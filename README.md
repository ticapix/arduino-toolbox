# arduino-toolbox

| Branch | CI | Coverage |
|---|---|---|
| master | [![Build Status](https://travis-ci.org/ticapix/arduino-toolbox.svg?branch=master)](https://travis-ci.org/ticapix/arduino-toolbox) | [![Coverage Status](https://coveralls.io/repos/ticapix/arduino-toolbox/badge.png?branch=master)](https://coveralls.io/r/ticapix/arduino-toolbox?branch=master) |
| 1.0 | [![Build Status](https://travis-ci.org/ticapix/arduino-toolbox.svg?branch=1.0)](https://travis-ci.org/ticapix/arduino-toolbox) | [![Coverage Status](https://coveralls.io/repos/ticapix/arduino-toolbox/badge.png?branch=1.0)](https://coveralls.io/r/ticapix/arduino-toolbox?branch=1.0) |


##  [RingBuffer](/RingBuffer/)

Simple `RingBuffer<Size, Type>` class with a specialization for `StringBuffer<Size>`

## [AsyncComm](/AsyncComm/)

Class helper for async communication over `HardwareSerial`, `SoftwareSerial` or any object exposing `read`, `write`, `available` methods.

## [GPRS](/GPRS/)

A class for communicating with a GSM/GPRS modem like the popular SIM900 used in many Arduino and Seeeduino shield.

## Tests

All toolboxs are unit-tested using `GTest` and `GMock` frameworks.
