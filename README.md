# LoRaFox
A simple fox-hunt beacon built with an ESP32 microcontroller and an SX1278 LoRa transceiver.


This project consists of the following components:

   * A simple schematic describing how you might build a beacon using a NodeMCU-32S, an SX1278, and an SSD1306-based I2C OLED display, both as a KiCad document and in a JPEG image.
   * An Arduino sketch for programming a Heltec WiFi LoRa 32 (tested), a TTGO LoRa 32 (untested), or a device built according to the schematic (also untested -- honestly, the production boards are probably not all that much more expensive than the components).
   * Some images of the first of these devices.
   
In order to build this sketch, aside from the Arduino IDE, you'll also need the following libraries:
   
   *U8x8* for the display.
   *Radiolib* for the transceiver.
   
You may install the required libraries through the Arduino library manager.   

