# Boonchai Security System
## Team Members
- 64100738 Kasidit Boonchai
- 64111412 Methanusorn Sutthirat
- 64114069 Setthapong Kiankhao

# Project Structure

## Mosquitto.conf File
This file is used to run the MQTT Broker.

## indoor_esp8266.ino File
Source code to control the microcontroller built-in LED using MQTT.

## Folder: write_rfid
Contains the following file:
- `write_rfid.ino`: Source code to control writing data to RFID cards using a microcontroller.

## Folder: rfid_read
Contains the following file:
- `rfid_read.ino`: Source code to read data from RFID cards using a microcontroller and send it to the MQTT Broker. This code is designed for the ATmega328P+ESP8266 board, which combines two chips for processing.

## Folder: Backend
Contains essential files for the Web API Server, Database (JSON Server), and Web Frontend module. The Web Frontend module is stored in the `templates` folder.

# Project Demo
To view a demo of the project, check out the YouTube video [here](https://youtu.be/ImKdBB_0DGw?si=FHAnJP-pQUcLjYLM)

