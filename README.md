# Arduino EEC IV Reader

Arduino implementation of the DCL-Protocol to communicate with Ford EEC-IV ECUs.

Mostly based on findings of this implementation https://github.com/babroval/ford-eec-iv-diagnostic

This is currently a work in progress and not completely working. Use at your own risk!

## TODO
* Read live values
* Refactor state machine
* Move buffer to class
* Why does sync not end after fault code read?
* How to read multiple fault codes?
* Add buttons to select mode and restart reading