# EV-Battery-Controller
Battery Charge/Discharge and Heater Controller for small electric vehicles.

This system provides Battery telemetry services such as Voltage, Current, and Temperature. It also regulates charge/discharge
voltages, limits current when battery temps are too high or too low, controls a battery heater for cold environments, and basic
cooling fan control. Works with lithium ion batteries, but does not provide cell ballancing. That must be done with a
dedicated BMS. The circuit provided should handle up to 30 amps with proper cooling. It also does basic error logging and will
shut down the system on events such as under/over voltage, over current, temps out of range, etc.


File EVBatteryCTRLv1.X.zip contains the source code for the firmware. Extract and open using MPLab IDE. Current version is under debugging hell, but mostly work.

File BTMSv3.zip contains schematic files for KiCad. This version is currently under debugging hell. Voltage spikes get sent thoughout the system when the output gets ramped up, and new power saving feature is unusable/causes problems. Still working on a fix for this so I don't recommend using this design yet. ;)

Enjoy!
