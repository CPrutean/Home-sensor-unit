# Home-Sensor-Unit
Working on a Home-sensor-unit project which takes data from seperate Sensor-unit devices and relays them back to the Sensor Unit manager, then the Sensor-unit-manager makes that information available locally to your network or to a web server which can then relay the information to a website.

**Link to project:** (LIVE DEMO TO COME SOON)


## How It's Made:

**Tech used:** ESP32, PlatformIO, C++, UART, Python, JavaScript, HTML, CSS, React, Linux.
There are 2 seperate ways that this Home-sensor-unit can be hosted. The first way is that we have multiple Sensor units working independently, a sensor unit manager which pings the devices for readings, then handles those readings and returns them back to the server that it is directly connected to via Usb cable using UART as a communication protocol. The server that it is connected to is a linux machine running a debian based distribution(The distributions that have been tested have been Ubuntu and Raspberry Pi OS) and will store those readings locally for later use.

The second way is the Sensor unit manager will host its own HTTPS web server locally, probing sensor units for readings, and also making itself locally available on the network as an API to pull said information for other services.

Currently the project is facing major refactoring changes since the first instance of the project did work, but the code was very slow since it was parsing strings instead of data.

## Lessons Learned:

Some of the lessons I learned where learning to take different approaches into account. As mentioned above the Messages would rely on string parsing back and forth which was slow and very computation heavy for a device such as a micro-controller which has much weaker resources available to it. Additionally I learned a lot about design principles, security, and data reliability building my own systems and working on recreating them myself. 

