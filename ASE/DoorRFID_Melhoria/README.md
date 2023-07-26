The 'start.sh' script is used to launch the dashboard and api on the laptop
The directory structure is as follows:
```
src/ contains the api and dashboard code
src/api.py source code for the api on the PC
src/app.py source code for the dashboard
src/static contains the dashboard static files, in this case a single image
src/templates contains the dashboard html templates
src/data.json used to store our data

door/ is the root directory for the esp32 code that does the reading of the rfid sensor and permits or denies access
door/main contains the esp32 code, which are .h and .c files
door/main/api.* contains code to handle communication from the esp32 to the api
door/main/buzzer.* contains code to configure and use the buzzer
door/main/lcd.* contains code to configure and use the lcd
door/main/leds.* contains code to configure and use the led
door/main/project.c contains the main code for the esp32
door/main/rc522.* contains code to configure and use the rfid sensor
door/main/server.* contains code to handle communication from api to esp32
door/main/uart.* contains code to configure and use the uart

station/ is the root directory for the esp32 code for the control node
station/main contains the esp32 code, which are .h and .c files
station/main/api.* contains code to handle communication from the esp32 to the api
station/main/buzzer.* contains code to configure and use the buzzer
station/main/leds.* contains code to configure and use the led
station/main/rc522.* contains code to configure and use the rfid sensor
station/main/server.* contains code to handle communication from api to esp32
station/main/station.c contains the main code for the esp32
station/main/uart.* contains code to configure and use the uart

```
