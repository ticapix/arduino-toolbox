# arduino-toolbox

| Branch | CI | Coverage |
|---|---|---|
| master | [![Build Status](https://travis-ci.org/ticapix/arduino-toolbox.svg?branch=master)](https://travis-ci.org/ticapix/arduino-toolbox) | [![Coverage Status](https://img.shields.io/coveralls/ticapix/arduino-toolbox.svg)](https://coveralls.io/r/ticapix/arduino-toolbox?branch=master) |
| 1.0 | [![Build Status](https://travis-ci.org/ticapix/arduino-toolbox.svg?branch=1.0)](https://travis-ci.org/ticapix/arduino-toolbox) | [![Coverage Status](https://img.shields.io/coveralls/ticapix/arduino-toolbox.svg)](https://coveralls.io/r/ticapix/arduino-toolbox?branch=1.0) |


##  [RingBuffer](/RingBuffer/)

Simple `RingBuffer<Size, Type>` class with a specialization for `StringBuffer<Size>`

## [AsyncComm](/AsyncComm/)

Class helper for async communication over `HardwareSerial`, `SoftwareSerial` or any object exposing `read`, `write`, `available` methods.

## Tests

All toolboxs are unit-tested using `GTest` and `GMock` frameworks.
