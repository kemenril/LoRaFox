# LoRaFox
A simple fox-hunt beacon built with an ESP32 microcontroller and an SX1278 LoRa transceiver.


### Respository contents:

   * A simple schematic describing how you might build a beacon using a NodeMCU-32S, an SX1278, and an SSD1306-based I2C OLED display, both as a KiCad document and in a JPEG image.
   * An Arduino sketch for programming the microcontroller.
   * Some images of my device.
   
### Required libraries and extras:
   
   * *U8x8* for the display.
   * *Radiolib* for the transceiver.
   
You may install the required libraries through the Arduino library manager.

You will also want at least the Arduino ESP32 core.  Installations for the core can be found here: https://github.com/espressif/arduino-esp32

You may as well also install the Heltec board variant too.  Instructions here: https://docs.heltec.cn/#/en/user_manual/how_to_install_esp32_Arduino

### Supported hardware:

   * A **Heltec WiFi LoRa 32** (tested)
   * A **TTGO LoRa 32** (untested)
   * Something like what's described in the included schematic (also untested for the moment)
   
### Buid settings for the Arduino IDE:

   * Board: **Heltec WiFi LoRa 32 V2** (probably, or you could try something else.
   * CPU Frequency: **40Mhz** (It will automatically be raised to 80 at times for responsiveness.)
   * Flash Frequency: Also **40Mhz**
   * Partition Scheme: **Default 8MB** (... or whatever, it doesn't use enough resources to matter)

### Wiring the Heltec WiFi LoRa 32 or TTGO LoRa 32 modules


... the schematic for a home-built version should probably work as shown.

### Starting the beacon for the first time

When the beacon is first started, no configuration information will be available.  The transmitter will be disabled automatically, and the call will be set *N0CALL*.  The SPIFFS filesystem will be initialized, so the initial start might take a few seconds.

### Configuring the beacon using the serial interface
   **Warning:** There is a minor bug in the handling of the flash button vs. some of the hardware attached to the USB serial driver.  I suspect it's hardware related.  When you're working with the serial port, the button occasionally appears to become stuck down.  For this reason, you probably don't actually want to run the beacon with a terminal program attached to its serial port.  Plugging it in for programming should be fine, and a reset will clear the condition.
   
Plug the beacon into a USB port and connect a serial terminal with the following settings:

   * Baud: **115200**
   * Parity: **None**
   * Data Bits: **8**
   * Stop Bits: **1**
   * Local Echo: **On**

After the call is displayed on the screen, hit a button on the keyboard a few times, *return* will be fine.  You should see a prompt.  Take its advice and press **?** for help.

Take a look at the current settings by hitting **P**.

### Using the beacon

The beacon can store a very large number of presets in flash, practically limited only by your willingness to manage them.  A quick press of the *flash* button on the microcontroller board will enable or disable the beacon's transmitter.  A long press on the button (a couple seconds or so) will load the next preset in the list.  

