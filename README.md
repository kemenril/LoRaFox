# LoRaFox
A simple fox-hunt beacon built with an ESP32 microcontroller and an SX1278 LoRa transceiver.


### Respository contents:

   * A schematic describing how you might build a beacon using a NodeMCU-32S, an SX1278, and an SSD1306-based I2C OLED display, both as a KiCad document and in a JPEG image.
   * An Arduino sketch that will turn either the device in the schematic, or a Heltec WiFi LoRa 32, or a TTGO LoRa 32, or a similar board, into a fox hunt beacon.
   * Some images of my device.
   
### Required libraries and extras:
   
   * *U8x8* for the display.
   * *Radiolib* for the transceiver.
   
You may install the required libraries through the Arduino library manager.

You will also want at least the Arduino ESP32 core.  Instructions for installing the core can be found here: 

https://github.com/espressif/arduino-esp32

You may as well also install the Heltec board variant too.  Instructions here: 

https://docs.heltec.cn/#/en/user_manual/how_to_install_esp32_Arduino

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

The pre-built modules often have pin DIO2 of their SX1278 chip attached to GPIO pin 32.  Pin 32 on an ESP32 is input-only, and is useless for our purposes when we're trying to use it to send the modulation signal into the FSK modem.  You'll need to run a jumper wire from pin 32 to the pin defined as RADIO_MOD_ALT in the sketch.  At the moment, this is set to GPIO12.  You can change it if you like, but I picked this pin because it is more or less empty otherwise and easy to reach from pin 32.  In the TTGO LoRa32 V2 module, pin DIO2 is just dangling off of the edge of the board somewhere, and not really attached to anything as far as I can tell.  You'll also need to run a jumper wire for this arrangement.  Connect the following points on the board.

For the **Heltec WiFi LoRa V2**, probably the **WiFi LoRa V1**, and the **TTGO LoRa32 V1** modules: 32 -> 12

For the **TTGO LoRa32 V2**: LoRa_DIO2 (which is apparently separate on the board) -> 12

... the schematic for a component-bulit version shows DIO2 connected to pin 12 and should work as shown.

Nothing here will prevent you from programming the sketch into an empty board, but you won't get FM from it until you connect pin DIO2 to the right place.

### Starting the beacon for the first time

When the beacon is first started, no configuration information will be available.  The transmitter will be disabled automatically, and the call will be set *N0CALL*.  The SPIFFS filesystem will be initialized, so the initial start might take a few seconds.  Please don't run the beacon like this.  It's uncivil.

### Configuring the beacon using the serial interface
   **Warning:** There is a minor bug in the handling of the flash button vs. some of the hardware attached to the USB serial driver.  I suspect it's hardware related.  When you're working with the serial port, the flash button occasionally appears to become stuck down.  For this reason, you probably don't actually want to run the beacon with a terminal program attached to its serial port.  Plugging it in for programming should be fine, and a reset will clear the condition.
   
Plug the beacon into a USB port and connect a serial terminal with the following settings:

   * Baud: **115200**
   * Parity: **None**
   * Data Bits: **8**
   * Stop Bits: **1**
   * Local Echo: **On** if you can do it.  Otherwise you probably won't see what you're typing.

After the call is displayed on the OLED screen, hit a button on the connected computer's keyboard a few times, *return* will be fine.  You should see a prompt.  Take its advice and press **?** for help.

Take a look at the current beacon settings by hitting **P**.

Now you'll want to hit a key to change one of a few options:
   * **B**eacon message
   * **C**allsign
   * **D**elay between transmissions
   * **F**requency
   * **M**ode
   * **O**utput power
   
Get everything set the way you like, and then consider saving the preset by pressing **S**.  You will be prompted for a name.  You can name the preset any reasonable thing you like, but if you call it *Default*, the beacon will load it automatically during startup, even if it's not the first preset on the list.  It will also start with the radio on and transmit the **I**nit message.  If there is no *Default* preset, the first preset saved will be loaded, but the beacon will start disabled.

The beacon can store a very large number of presets in flash, practically limited only by your willingness to manage them.  Feel free to put in as many as you like.  Once you're done with the configuration, either reset your beacon, or at least hit **X** to get out of the command interpreter.  The beacon will not operate while it's running the command-line.
   

### Using the beacon

Just apply power to it.  Once it's on, a quick press of the *flash* button on the microcontroller board will enable or disable the beacon's transmitter.  A long press on the button (a couple seconds or so) will load the next preset in the list.  You could pretty easily add a battery in a few different ways.  If you're using one of the prebuilt boards, you will have a lipo battery charger built in.  I'm using a 600mAh lipo cell with mine, and based on some rough calculations and power draw tests I think I should have a bit longer than 50 hours of runtime, even on high power.  Idle power draw is below 10mA. 

