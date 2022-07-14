# CAN-Network
This project consists of two CAN nodes puplishing and listening to messages on a CAN bus based on an esp32.

ECU_1 is programmed to read data using SPI from an ABPDRRV015PDSA3 sensor which can read both pressure and temperature. ECU_1 converts the temperature to celsius and converts the pressure to 0.01 millibar and puplishes these two values in two different CAN frames with predefined identifiers.

ECU_2 is programmed to listen to pressure and temperature frames on the bus and prints them upon receiving.

Video Overview of the working application:

Hardware: https://youtu.be/kiKiHDHKg5g

Software: https://youtu.be/EGtzW8QNXfk

### How to compile

Install esp_idf framework: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html.

Open ESP-IDF CMD

Connect the esp to any USB port and do the following:
```bash
git clone https://github.com/mohamed-mosbah/CAN-Network/
cd CAN-Network/ECU_1/
idf.py set-target esp32
idf.py -p <PORT> flash monitor
```
You need to know what is the name of the port, being used. This can be known from the device manager under Ports (COM & LPT). In my case it is typically COM7 so the last command would be:
```bash
idf.py -p COM7 flash monitor
```
