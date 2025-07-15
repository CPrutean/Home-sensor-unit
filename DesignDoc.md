# Design document
## Sensor unit design
### Will be connected to one or more sensors which will be used to communicate with the main
### board
###
### Sensors will hold onto packets which will then be pulled down by the main communication board
### 
### Different sensors will send information down upon an event based off the urgency (ex: laser sensor guarding a door
### will take priority over a weather sensor unit)
###
### Sensors will take readings at different time intervals which can be adjusted based off user input
###
### Using the ESP32 platform sensor units send information using ESPNow communication protocol
###
### 
## Communication units design
### We will have one central unit which will be used to pull down and recieve information based on time intervals/user requests.
### 
### Communication unit will take information then send it off to database to store information
### 
### Communication unit will also recieve urgent pushes from sensors based off more urgent information and send user the information.
### 
### Communication unit will use ESPNow to send and recieve information
### 
### The communication unit will connect to a raspberry Pi to send information off to the database using UART/I2C/SPI
## Overall design ideology
### Sensor units will store readings to be pulled down upon request
### More urgent sensor units will push down information to be sent off to the application and warn the user
###
### The communication unit will pull down information on time intervals/user requests
### The communication unit will take the packets and send them off to the database to store daily event information
### for logging reasons
### 
### The application is represented by a desktop application or a mobile app or a website to notify/give access to the information from the devices
### The application will give information from each of the sensor units at a glance. 
### When the application is opened sensor units will be refreshed.
### The application will communicate directly with the database.
###
### The database will store information and store them in different folders based off the date and time.
### The events will be stored in a CSV format and be parsed by the application for viewing past information
### 
### Raspberry pi will act as the database to make requests to for events/information
### Raspberry pi will communicate with main ESP32 board that will communicate with the other ESP32 sensor units.
## Application communication protocols
### The application will make requests to the main Raspberry Pi to request information and recieve and display information based off connected sensor units
### 
### The application displays information on sensor units such as battery life/last reading recieved/last time sensor was tripped.
## Database design/hosting
### Database will be hosted on raspberry pi/a seperate computer which will store information on when sensor units were tripped/when readings from sensor units 
### were made/what requests the user made for debugging/user reading the history of the application
### 
### Information on sensor readings will be formatted into a CSV file for each day which will contain what the event was, the time the event happened
### ,what happened during the event and if it was user prompted or automatically prompted.

