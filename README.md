# EV-Battery-Controller
Battery Charge/Discharge and Heater Controller for small electric vehicles.

This system provides Battery telemetry services such as Voltage, Current, and Temperature. It also regulates charge/discharge
voltages, limits current when battery temps are too high or too low, controls a battery heater for cold environments, and basic
cooling fan control. Works with lithium ion batteries, but does not provide cell ballancing. That must be done with a
dedicated BMS. The circuit provided should handle up to 30 amps with proper cooling. It also does basic error logging and will
shut down the system on events such as under/over voltage, over current, temps out of range, etc.


File EVBatteryCTRLv1.X.zip contains the source code for the firmware. Extract and open using MPLab IDE.

File BTMSv3.zip contains schematic files for KiCad. This version is untested as of June 8th, 2020. I'm still waiting for my
boards to come in the mail. ;)

Enjoy!
