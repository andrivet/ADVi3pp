# Generic Proximity / switch-like probe for Wanhao Duplicator i3 Plus mainboard (5.1)

The first version of the mainboard - version 5.1 - for the Wanhao Duplicator i3 Plus features a connector named `Z-Probe`. This connector allows to connect a switch-like probe, preferrably a proximity sensor. These probes are very affordable and available. They all work and are accurate enough for the usecase, so it does not matter if it is a capacitive or a inductive one. There are several diameters, 12 millimeter and 18 millimeters. The models are for example:

* LJ18A3-8-Z/(BX)
* LJ18A3-B-Z/(BX)
* LJ12A3-4-Z/(BX)

Most of these sensors **require 6-36V** to work, So 5 volts are not sufficient. By default, the board delivers 24V on pin 1 onf the connector. See [here](sebastien.andrivet.com/en/posts/wanhao-duplicator-i3-plus-3d-printers) for a full description of the connector. All listed sensors come, **normally closed**, which means they are `HIGH` when not triggered and vice versa. Use the `M 119` comand to check the status of the switches beforehand. Only a  **normally closed** sensors are currently supported. The described connector on the mainboard can be used to hook up a probe as follows.

<img src="./assets/lj_sensor_wiring.svg" width="400" description="Probe hookup">

The resistors from the example are not very well chosen. I just used what I had laying around. Do your own calculation for an appropriate voltage divider to end up with about 2.5 volts for a `HIGH`.