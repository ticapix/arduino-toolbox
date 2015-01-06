# arduino-toolbox

[![Build Status](https://travis-ci.org/ticapix/arduino-toolbox.svg)](https://travis-ci.org/ticapix/arduino-toolbox) [![Coverage Status](https://img.shields.io/coveralls/ticapix/arduino-toolbox.svg)](https://coveralls.io/r/ticapix/arduino-toolbox?branch=master)

##  [RingBuffer](/RingBuffer/)

Simple `RingBuffer<Size, Type>` class with a specialization for `StringBuffer<Size>`

## [AsyncComm](/AsyncComm/)

Class helper for async communication over `HardwareSerial`, `SoftwareSerial` or any object exposing `read`, `write`, `available` methods.

## Tests

All toolboxs are unit-tested using `GTest` and `GMock` frameworks.
