//Radio, SX1278
#include <RadioLib.h>

#include <SPIFFS.h>
#include <FS.h>

#include <soc/rtc.h>

#include "morse.h"

//Settings
typedef struct SETTINGS {
    char magic[8];              //Make sure the loaded settings are correct.
    unsigned long delay_call;   //Time between IDs.
    unsigned long delay_blurb;  //Time between transmitting the blurb, in addition to the ID.
    float power;                //2 to 17 or so
    unsigned char mode;         //MODE_FM or MODE_CW
    float frequency;            //In Mhz, A beacon may be automatically controlled between 432.300 and 432.400
    float deviation;            //About 5 or so for FM, .6 -- the minimum -- for CW. 5 is probably about 12Khz
    char call[12];              //In ASCII
    char blurb[256];            //Printed before the call, once every delay_blurb msec.
    char init[256];             //Printed before the call at startup.
    unsigned char dot;          //Length of a Morse dot
    char eot[8];                //Second magic number.
} Settings;

//Used to clear the space possibly occupied by the callsign/power level.
#define EMPTYSTRING  "               "

//Magic strings to use in stored presets.
#define MAGIC "LORAFHBX"
#define EOT   "LORABEOT"
#define MAGIC_LEN 8


//Display
#include <U8g2lib.h>
#include <U8x8lib.h>
#define OLED_SDA  4
#define OLED_SCL  15
#define OLED_RST  16
//Use the second constructor to set Vcom deselect to zero in order to dim the display a bit.
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(OLED_SCL, OLED_SDA, OLED_RST);
U8X8_SSD1306_128X64_VCOMH0_SW_I2C u8x8(OLED_SCL, OLED_SDA, OLED_RST);

//CW modulation frequency in Hz.
#define TONE_FREQ 960


//Minimum delay between transmissions of a certain type.  Any setting below this is considered to be disabled.
#define DELAY_MIN 10000

//This set from the Heltec PDF, verified by multimeter.
#define RADIO_CS    18
#define RADIO_DIO0  26
#define RADIO_DIO1  35
#define RADIO_DIO2  34

//DIO2 is hard-wired to an input-only pin on the Heltec.
//  It's jumpered over here for FM.  Pin 12 works on the Heltec WiFi LoRa V2
//#define RADIO_MOD_ALT 13
#define RADIO_MOD_ALT 12

//Modes available
#define MODE_CW 0
#define MODE_FM 1

//Visual indicators
#define RADIO_INIT  'X'
#define PRESET_NEXT '>'
char *heartbeat = "-\|/";

SX1278 radio = new Module(RADIO_CS,RADIO_DIO0,RADIO_DIO1);
//Beacon is on/off.
char radstate = 0;

//Display power save
char screen_on = 0;
//Time (ms) after which to blank screen.
#define SCREEN_OFF_TIME 60000

//Define this if you're compiling for 40Mhz
#define CPU_SCALING

#ifdef CPU_SCALING

//CPU frequency scaling
#define CPU_FREQ_HIGH RTC_CPU_FREQ_80M

//Compile this for 40Mhz or so, or disable scaling.  20Mhz is too low.
#define CPU_FREQ_LOW  RTC_CPU_FREQ_XTAL

//Scale vs. -- I think -- 80Mhz.  Probably don't go below 40Mhz, or .5
//Hardware limitations begin to cause trouble there.  
#define CPU_SCALE   0.5

#else
//CPU Scaling is disabled
#define CPU_SCALE   1
#endif


//Are we in the command shell?
//This is important because we shouldn't change the CPU speed while the serial connection is active.
char shell = 0;

//For interrupting a delay loop.
char breaksig = 0;

//Have we gone through the initialization process?
char booted = 0;

// Store the previous clock value to detect roll-over
//unsigned long lastms = 0;

//Push-button handling
unsigned long debounce = 0;
unsigned long buttonDown = 0;
unsigned long buttonUp = 0;
char buttonState = 0;
//Debounce wait time in ms.  
#define DBTIME  24
//How long to hold a button for secondary function.
#define LONG_PRESS_TIME 650
char flip = 0;  //Set when switching presets by button-press.

//440Mhz band limits, with some extra buffer for deviation
//A beacon may be automatically controlled between 432.300 and 432.400
#define FREQ_MIN  420.02
#define FREQ_MAX  449.98
//Power limits of the radio in dBm
#define POW_MIN 2
#define POW_MAX 17

//Assume deviation is about 12Khz
#define deviation_fm 4.0
#define deviation_cw 0.6

//Code speed guidelines
#define DOT_LEN_MIN   8
#define DOT_LEN_MAX   200

Settings config;

//Opened by setup, and used to iterate through presets.
File root;

//Max filename length
#define FNSIZE 32
// and this will include one / and a terminating null, so 30 characters is about it..
char confFN[FNSIZE];


//Longest wait time without checking to abort the wait.
#define WAIT_MS     70
#define WAIT_SLICE  (WAIT_MS*CPU_SCALE)


//How long to power-save in ms.  Larger values save more power, but may reduce responsiveness.
#define SLEEP_PERIOD  1200

//We can shut down occasionally in the main loop to save power.
void powerSave(unsigned int ms) {
  esp_sleep_enable_timer_wakeup(ms*1000); //Microseconds.
  esp_light_sleep_start();


}

//Interruptible delay loop, returns false if interrupted
//We can't sleep very easily while we're doing PWM.  Things don't work exactly right.
char wait(int msec,char progress) {
  int scaled = msec*CPU_SCALE;
  char* hchar = heartbeat;
  for (int s = 0;s<scaled;s+=WAIT_SLICE) {
    if (progress) {
       u8x8.drawGlyph(1,4,*hchar);
       if (hchar >= &heartbeat[strlen(heartbeat)-1]) {
              hchar = heartbeat;
       } else {hchar++;}
    }
    if (breaksig)break;    
    if (scaled-s<WAIT_SLICE) {
      delay(scaled-s);
    } else {
      delay(WAIT_SLICE);
    } 
  }
  if (progress) u8x8.drawGlyph(1,4,' ');
  if (breaksig) {
    breaksig=0;
    return 0;
  }
  return 1;
}


//Radio on and off
void radOn() {
  #ifdef CPU_SCALING
    if (!shell)
     rtc_clk_cpu_freq_set(CPU_FREQ_HIGH);
  #endif  
  u8x8.drawGlyph(1,6,' ');
  #ifdef CPU_SCALING
    if (!shell)
      rtc_clk_cpu_freq_set(CPU_FREQ_LOW);
  #endif
  keyup();
  radstate = 1;
}
void radOff() {
  #ifdef CPU_SCALING
    if (!shell)
     rtc_clk_cpu_freq_set(CPU_FREQ_HIGH);
  #endif
  u8x8.drawGlyph(1,6,'!');
  #ifdef CPU_SCALING
    if (!shell)
     rtc_clk_cpu_freq_set(CPU_FREQ_LOW);
  #endif
  keyup();
  radstate = 0;
}
void radToggle() {
  if (radstate) {radOff();} else {radOn();}
}

void screenOff() {
  u8x8.setPowerSave(1);
  screen_on = 0;
}
void screenOn() {
  u8x8.setPowerSave(0);
  screen_on = 1;
}
void screenToggle() {
  if (screen_on) {screenOff();} else {screenOn();}
}

//Stop transmitting
void keyup() {
  //Just in case
  end_char();
  if (config.mode == MODE_FM)
    radio.standby();
    digitalWrite(BUILTIN_LED,LOW); 
}

//This starts a transmission.  CW doesn't need it, because we're switching the carrier on 
//  and off rather than modulating it.  
void keydown() {
  if (!radstate)return;
   digitalWrite(BUILTIN_LED,HIGH);
   if (config.mode == MODE_FM) { 
     radio.transmitDirect();
     if(!wait(600,true)) keyup();  //To wait for the squelch to open.
    }
}

void start_char() {
  if (!radstate)return;
    if (config.mode == MODE_FM) {
     ledcWriteTone(0,TONE_FREQ);
   } else {  //CW
     radio.transmitDirect();
    }
}
void end_char() {
  if (config.mode == MODE_FM) {
    ledcWrite(0,0);
  } else {   //CW
    radio.standby();
  }
}

//Transmit a string as code.
void xmit (char *input) {
  if (!radstate)return;
  const char *letter = "";
  keydown();
  for (int p = 0;p < strlen(input);p++) {
    letter = entity(toupper(input[p]));
    if (letter[0] == ' ') {  //Space is a special case
      wait(config.dot*WORDSPACE_MULT,true);
    } else {
      for (int m = 0;m<strlen(letter);m++) {
        start_char();
        switch (letter[m]) {
          case '.':
            u8x8.drawGlyph(13,4,'.');
            wait(config.dot,true);
            break;
          case '-':
            u8x8.drawGlyph(13,4,'-');
            wait(config.dot*DASH_MULT,true);
            break;
          default: 
            break;
       }
          end_char();
          u8x8.drawGlyph(13,4,' ');
          wait(config.dot*SPACE_MULT,true);

          //If we're at the end of the current letter, delay longer.
          if (m == strlen(letter)-1) {
            wait(config.dot*LTRSPACE_MULT,true);
          }
        }
      }
    }
  keyup(); 
}


//Reset and re-tune according to current global values
int radio_reset() {
  char tmp[18];
  int err;

  #ifdef CPU_SCALING
    if (!shell)
      rtc_clk_cpu_freq_set(CPU_FREQ_HIGH);  //Do this quickly.
  #endif

  //Needs to be done before keyup, because keyup also sets the tone frequency.
  ledcSetup(0,8000,8); //PWM channel 0, 8Khz sample, 8 bits.
  ledcAttachPin(RADIO_MOD_ALT,0);
  pinMode(RADIO_MOD_ALT,OUTPUT);

  keyup();
  u8x8.drawGlyph(14,4,RADIO_INIT);
  sprintf(&(tmp[0]),"%0.3f Mhz",config.frequency);
  u8x8.drawString(1,0,tmp);
  u8x8.drawString(1,2,&EMPTYSTRING[1]); //Clear to end of line.
  sprintf(&(tmp[0]),"PWR: %0.2f dBm",config.power);
  u8x8.drawString(1,2,tmp);
  u8x8.drawString(3,4,&EMPTYSTRING[3]);
  u8x8.drawString(3,4,config.call);
  u8x8.drawString(3,6,&EMPTYSTRING[3]);
  u8x8.drawString(3,6,confFN);
  
  if (config.mode == MODE_FM) { //Frequency, bitrate, deviation, rxbw, power, currentlimit, OOK
            
      //Radio init
      err = radio.beginFSK(config.frequency,1.2,config.deviation,125.0,config.power,100,16,false);
      if (err != ERR_NONE) {return err;}
      err = radio.setDataShaping(0.0);

      //Visual indicators
      u8x8.drawString(14,0,"FM");
      
  } else {  //MODE_CW
      pinMode(RADIO_MOD_ALT,INPUT); //Maybe this will prevent the output settings from needing to change
      err = radio.beginFSK(config.frequency,1.2,config.deviation,125.0,config.power,100,16,true);
      u8x8.drawString(14,0,"CW");
  }
  if (err == ERR_NONE)
    u8x8.drawGlyph(14,4,' ');

  #ifdef CPU_SCALING
    if (!shell)
      rtc_clk_cpu_freq_set(CPU_FREQ_LOW);
  #endif
  return err;
}

//Load/save from flash
int loadSettings(const char *preset) {
  Settings tmp;
  File input;
  char filename[FNSIZE];
  filename[0] = '/';
  strncpy(&filename[1],preset,FNSIZE-2); //This will be our maximum name length.

  if (input = SPIFFS.open(filename,"r")) {
    if (input.read((uint8_t *)&tmp,sizeof(Settings)) == sizeof(Settings)) {
      if (!strncmp(tmp.magic,MAGIC,MAGIC_LEN) && !strncmp(tmp.eot,EOT,MAGIC_LEN)) {
        memcpy(&config,&tmp,sizeof(Settings));
        if (!strncmp(config.magic,MAGIC,MAGIC_LEN) && !strncmp(config.eot,EOT,MAGIC_LEN)) {
          strncpy((char *)&confFN,preset,FNSIZE);
            return 1;
        }
      }
    }
    input.close();
  }
  return 0;
}
int saveSettings(const char *preset) {
  File output;
  char filename[32];

  filename[0] = '/';
  strncpy(&filename[1],preset,30);  //This will be our maximum name length.
  
  if ((!strncmp(config.magic,MAGIC,MAGIC_LEN) && !strncmp(config.eot,EOT,MAGIC_LEN))) {
     if (output = SPIFFS.open(filename,"w")) {
        if (output.write((uint8_t *)&config,sizeof(Settings)) == sizeof(Settings)) {
          return 1; 
        }
        output.close();
     }
  } // Settings were corrupt in memory.
  return 0;
}



//New configuration with default settings.
void defaultSettings() {
  //Configuration:
  sprintf(config.call,"N0CALL");
  config.blurb[0] = '\0';
  config.init[0] = '\0';
  config.delay_call  = 45000;
  config.delay_blurb   = 180000;
  config.frequency    = 432.350;
  config.power        = 2;
  config.mode         = MODE_FM;
  config.deviation    = deviation_fm;
  config.dot          = 30;
  strncpy((char *)&config.magic,MAGIC,sizeof(config.magic));
  strncpy((char *)&config.eot,EOT,sizeof(config.eot));
  strcpy(confFN,"No file!");
}


void nextPreset() {
  File np = root.openNextFile();

  if(!root || !np) {
    root = SPIFFS.open("/");
    np = root.openNextFile();    
  } //Try resetting
  
  if (!root || !np || !loadSettings(&np.name()[1])) {
    defaultSettings();  // ... or it might be right to just leave things alone here.
  } 
  radio_reset();
}

//Handle backspaces appropriately, also strip out unprintable characters.
String stringClean(String input) {
  for (int i = 0;i< input.length();i++) {
    if ((input.charAt(i) < 32) || (input.charAt(i) > 126)) {
      if ((i > 0) && (input.charAt(i) == 8)) {
        input.remove(i-1,2);
        i-=2;
      } else {
        input.remove(i,1);
        i--;
      }
    }
  }
  return input;
}

void cmdDel(const char *preset) {
  File target;
  char filename[32];
  filename[0] = '/';
  strncpy(&filename[1],preset,30);  //This will be our maximum name length.
  Serial.print(preset);
  if (!SPIFFS.remove(filename)) {
    Serial.print(" not ");
  }
  Serial.println(" removed.");
  return;
}

void cmdDir() {
  File root = SPIFFS.open("/");
  File preset = root.openNextFile();
  if (!preset) {
    Serial.println("No files.");
    return;
  }
  while (preset) {
     Serial.println(preset.name());
     preset = root.openNextFile();
  }
  return;
}

void cmdHelp() {
    Serial.println();
    Serial.println("--- Commands for current settings ---");
    Serial.println("B : Set beacon message");
    Serial.println("C : Set callsign");
    Serial.println("D : Set delay between transmissions.");
    Serial.println("F : Set frequency");
    Serial.println("I : Set initialization message");
    Serial.println("M : Set mode");
    Serial.println("N : Clear current settings and use new, mostly safe defaults.");
    Serial.println("O : Set output power");
    Serial.println("P : Print current configuration");
    Serial.println(". : Set length of the Morse .");
    Serial.println();
    Serial.println(" --- Commands for file management --- ");
    Serial.println("E : Erase flash/format SPIFFS");
    Serial.println("G : Get preset from storage");
    Serial.println("L : List presets");
    Serial.println("R : Remove preset from storage");
    Serial.println("S : Save preset to storage");
    Serial.println();
    Serial.println(" --- Other --- ");
    Serial.println("A : Toggle beacon activity on/off");
    Serial.println("X : End command session, continue");
    
    Serial.println("! : Reset radio now");
}

void commandLine() {
  int input;
  String inStr;
  float inFlt;
  long inInt;
  shell = 1;
  screenOn();
  Serial.println("Serial monitor");
  //Turn on local echo; doesn't seem to work on minicom.
  Serial.print("\e[12l");

  while (true) {
    input = -1; //Unsuccessful read.
    //Sometimes we get a dangling carriage-return.
    Serial.read();
    Serial.print("(Enter ? for help) > ");
      while (input == -1) {
        input = Serial.read();
      }
      Serial.println();
      switch (toupper((char)input)) {
          case '?':
            cmdHelp();
          break;
          case 'X':
            buttonUp = millis();  //Make sure the display doesn't flip off at an inopportune time.
            radio_reset();
            Serial.println("Exit.");
            return;
          case '!': 
            Serial.println("Resetting radio.");
            radio_reset();
            break;
         case 'A':
           radToggle();
           Serial.print("Toggle radio beacon: ");
           if (radstate) {
            Serial.println("On.");
           } else { 
            Serial.println("Off.");
           }
           break;
         case 'N':
          Serial.println("Initialize new settings settings.");
          defaultSettings();
          radio_reset();
         break;
         case 'M':
          Serial.print("Set mode: ");
          if (config.mode == MODE_FM) { //to CW
            config.mode = MODE_CW;
            config.deviation = deviation_cw;
          } else {  //CW to FM
            config.mode = MODE_FM;
            config.deviation = deviation_fm;            
          }
          //We could do this above, but this way we change the settings and indicate whether or not it actually worked.
          if (config.mode == MODE_FM) {
            Serial.println("FM");
          } else {  //CW
            Serial.println("CW");
          }
          radio_reset(); //Also updates the display.
          break;
         case 'B':
            Serial.println("Set beacon message.");
            Serial.println("    This message should generally be transmitted less often than the call itself.");  
            Serial.println("    It should probably include your callsign.");
            Serial.print("New message: ");
            inStr = Serial.readStringUntil('\r');
            inStr = stringClean(inStr);
            if (inStr.length() > (sizeof(config.blurb)-1)) {Serial.println("WARNING: Message will be truncated.");}
            sprintf(config.blurb, "%.255s", inStr.c_str());
            Serial.println("Set beacon message to:");
            Serial.print("    ");
            Serial.println(config.blurb);
            break;
         case 'I':
            Serial.println("Set init message.");
            Serial.println("    This message is transmitted once when the beacon is initialized.");  
            Serial.println("    You can leave it empty to disable it.");
            Serial.print("New message: ");
            inStr = Serial.readStringUntil('\r');
            inStr = stringClean(inStr);
            if (inStr.length() > (sizeof(config.init)-1)) {Serial.println("WARNING: Message will be truncated.");}
            sprintf(config.init, "%.255s", inStr.c_str());
            Serial.println("Set init message to:");
            Serial.print("    ");
            Serial.println(config.init);
          break;
         case 'C':
            Serial.println("Set callsign.");
            Serial.print("Enter new call: ");
            inStr = Serial.readStringUntil('\r');
            inStr = stringClean(inStr);
            Serial.println(inStr);
            if (inStr.length() > (sizeof(config.call)-1)) { Serial.println("WARNING: Callsign will be truncated.");}
            sprintf(config.call,"%.11s",inStr.c_str());  //Call is 12 bytes long.
            Serial.print("\nSet call to ");
            Serial.println(config.call);
            radio_reset(); //Also updates the display.
            break;
         case 'D':
           Serial.println("Set delay:  ");
           Serial.println("    If you set the delay too low, the associated message will simply be disabled.");
           Serial.println("    Setting it to 0 is one reasonable way to stop transmission of a certain type of ");
           Serial.println(" message.");
           Serial.print("Seconds between callsign transmissions: ");
           inFlt = Serial.parseFloat();
           config.delay_call = inFlt*1000;
           if (config.delay_call < DELAY_MIN) {config.delay_call = 0;}
           Serial.println();
           Serial.print("Call transmissions will occur every ");
           Serial.print(config.delay_call/1000);
           Serial.println(" seconds.");
           Serial.print("Seconds between message transmissions: ");
           inFlt = Serial.parseFloat();
           config.delay_blurb = inFlt*1000;
           if (config.delay_blurb < DELAY_MIN) {config.delay_blurb = 0;}
           Serial.println();
           Serial.print("Message transmissions will occur every ");
           Serial.print(config.delay_blurb/1000);
           Serial.println(" seconds.");
           break;
         case 'F':
           Serial.println("Set Frequency: ");
           Serial.println("    70CM amateur band in the US is from 420-450Mhz.  For FM, consider the range ");
           Serial.println(" 420.02-449.98 to be the maximum outer bounds.");
           Serial.println("    Additionally, beacons may be automatically controlled between 432.300 and 432.400. "); 
           Serial.println(" For FM, maybe stay between 432.312 and 432.388.");
           Serial.print("New Frequency: ");
           inFlt = Serial.parseFloat();
           if (inFlt < FREQ_MIN) {
            config.frequency = FREQ_MIN;
           } else {
            if (inFlt > FREQ_MAX) {
              config.frequency = FREQ_MAX;
            } else {
              config.frequency = inFlt;
            }
           }
           Serial.print("\nFrequency set to ");
           Serial.println(config.frequency,4);
           radio_reset(); //Also updates the display.
           break;
         case '.':
           Serial.println("Set code dot length: ");
           Serial.println("    This is the length of a code dot in ms.");
           Serial.print("New length: ");
           inInt = Serial.parseInt();
           if (inInt < DOT_LEN_MIN) {
            config.dot = DOT_LEN_MIN;
           } else {
            if (inInt > DOT_LEN_MAX) {
              config.dot = DOT_LEN_MAX;
            } else {
              config.dot = inInt;
            }
           }
           Serial.print("\nCode dot length set to ");
           Serial.println(config.dot,DEC);
           break;
         case 'O':
           Serial.println("Set output power: ");
           Serial.println("    This is a floating-point number of dBm between 2 and 17.");
           Serial.print("New power level: ");
           inFlt = Serial.parseFloat();
           if (inFlt < POW_MIN) {
            config.power = POW_MIN;
           } else {
            if (inFlt > POW_MAX) {
              config.power = POW_MAX;
            } else {
              config.power = inFlt;
            }
           }
           Serial.print("\nPower level set to ");
           Serial.print(config.power,4);
           Serial.println("dBm");
          radio_reset(); //Also updates the display.
           break;
         case 'P':
          Serial.println("Print current configuration:");
          Serial.print("Frequency:\t");
          Serial.println(config.frequency,4);
          Serial.print("Mode:\t\t");
          if (config.mode == MODE_FM) {
            Serial.println("FM");
          } else {
            Serial.println("CW");
          }
          Serial.print("Power:\t\t");
          Serial.print(config.power,4);
          Serial.println("dBm");
          Serial.print("Dot length:\t");
          Serial.println(config.dot,DEC);
          Serial.print("Station call:\t");
          Serial.println(config.call);
          Serial.println("Initialization message:");
          Serial.print("    ");
          Serial.println(config.init);
          Serial.println("Message:");
          Serial.print("    ");
          Serial.println(config.blurb);
          Serial.print("Call beacon time:\t");
          Serial.println(config.delay_call/1000);
          Serial.print("Message time:\t\t");
          Serial.println(config.delay_blurb/1000);
          Serial.print("Radio beacon is ");
          if (radstate) {
            Serial.println("On.");
           } else { 
            Serial.println("Off.");
           }
           Serial.println();
          break;
         case 'L':
          Serial.println("Directory of flash");
          cmdDir();
          break;
         case 'R':
            Serial.println("Delete a preset.");
            Serial.print("Filename: ");
            inStr = Serial.readStringUntil('\r');
            inStr = stringClean(inStr);
            if (inStr == "") {
              Serial.println("Abort.");
            } else {
              cmdDel(inStr.c_str());
            }
          break;
         case 'E': 
          Serial.println("Erase presets / format SPIFFS");
          Serial.println("    This will delete all presets in the flash.  It will not change the current settings, ");
          Serial.println(" which you may save again afterward if you want to keep them.");
          Serial.println("    Are you sure you want to do this?  Push Y to continue, or any other key to abort: ");
          input = -1;
          while (input == -1) {
              input = Serial.read();
          }
          if (toupper(input) == 'Y') {
            Serial.print("Formatting... ");
            SPIFFS.format();
            Serial.println("done.");
          } else {
            Serial.println("Format cancelled.");
          }
          break;
         case 'G':
          Serial.println("Get a preset from flash.");
          Serial.print("Filename: ");
          inStr = Serial.readStringUntil('\r');
          inStr = stringClean(inStr);
            if (inStr == "") {
              Serial.println("Abort.");
            } else {
              if (loadSettings(inStr.c_str())) {
                Serial.print("Loaded ");
              } else {
                Serial.print("Could not load ");
              }
              Serial.println(inStr);
            }
          break;
         case 'S':
          Serial.println("Save a preset to flash.");
          Serial.print("Filename: ");
          inStr = Serial.readStringUntil('\r');
            if (inStr == "") {
              Serial.println("Abort.");
            } else {
              inStr = stringClean(inStr);
              if (saveSettings(inStr.c_str())) {
                Serial.print("Saved ");
              } else {
                Serial.print("Could not save ");
              }
              Serial.println(inStr);
            }
          break;
         default:
          Serial.println();
           if (input != 13) { //CR            
            Serial.print("\nBad command: ");
            Serial.print((char)toupper((char)input));
            Serial.println();
           }
      }
    }
  shell = 0;
  return;
}

//Clear the display.  This function runs the CPU at 80Mhz.
void init_display() {
  u8x8.begin();
  u8x8.setContrast(15); //8-bit contrast, but it's naturally pretty high, so set it low.
  u8x8.setFont(u8x8_font_8x13B_1x2_r); // use _r to import a restricted character set, _f for full.
  u8x8.clear(); //maybe not needed.
  screenOn();
  return;  
}

void prg_button(unsigned long tval) {

      if ((digitalRead(KEY_BUILTIN) == LOW) && (tval > debounce+DBTIME)) {
      debounce = tval;      
//  u8x8.drawGlyph(15,6,'D');
      if (!buttonState) { //Initial button press
        buttonDown = tval;
        buttonState = 1;
      }
/* There is a bug, probably hardware-related, where if you have the serial port initialized on the remote side, GPIO pin 0 can
  go low and stay low.  Maybe a patch wire could fix the problem.  Otherwise just don't leave the serial port connected when you're
  not programming the device.  We can detect the problem this way.
*/
      if (tval-buttonDown >5000) {
        Serial.print("Warning: Button stuck! T: ");
        Serial.print(tval);
        Serial.print(" Since: ");
        Serial.print(buttonDown);
        return;
      }

      if (tval - buttonDown > LONG_PRESS_TIME) {
        if (!flip) {
          flip = 1;
          #ifdef CPU_SCALING
            rtc_clk_cpu_freq_set(CPU_FREQ_HIGH);
          #endif
          u8x8.drawGlyph(15,6,PRESET_NEXT);
          #ifdef CPU_SCALING
            rtc_clk_cpu_freq_set(CPU_FREQ_LOW);
          #endif
        }
      }      
    } else {
    if (buttonState && tval > debounce+DBTIME) {
// u8x8.drawGlyph(15,6,'U');
      debounce = tval;
      buttonUp = tval;
      buttonState = 0;
      if (!screen_on) {screenOn();flip=0;} else { //Button up.  Turn screen on, or
        if (flip) {  //Hold button for a while.
         //Turn off indicator PRESET_NEXT
         #ifdef CPU_SCALING
          rtc_clk_cpu_freq_set(CPU_FREQ_HIGH);
         #endif
         u8x8.drawGlyph(15,6,' ');
         #ifdef CPU_SCALING
          rtc_clk_cpu_freq_set(CPU_FREQ_LOW);
         #endif
         flip = 0;
         nextPreset();  //Flip through presets.
        } else {
         radToggle(); //Toggle radio on/off
        }
      }
    }
  }

  //Turn off display after a while so we don't kill too many pixels.
  if (tval - buttonUp > SCREEN_OFF_TIME && screen_on) {
    screenOff();
  }
  
}

void setup() {
  //Enable wake on button press.
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable(GPIO_NUM_0,GPIO_INTR_LOW_LEVEL);

  //Initialize some things
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(KEY_BUILTIN, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN,LOW);  
  init_display();
  File topPreset;

  //Radio starts out off anyway, but the indicator is not lit for it.
  radOff();
  booted = 0;
  shell = 0;

  //Configuration:
  Serial.begin(115200);
  //Wait a few minutes before timing out trying to read a string on the serial port.
  Serial.setTimeout(1500000);


  if (SPIFFS.begin(true)) { //Try to mount the filesystem.  It should be auto-formatted.
     root = SPIFFS.open("/");
     topPreset = root.openNextFile();

    if (loadSettings("Default")) {
      radio_reset();
      radOn();
    } else {
      if (topPreset && loadSettings(&topPreset.name()[1])) {  //Skip the / here, since it's added back in the loadSettings function.
        radio_reset();
      } else {
        defaultSettings();
        radio_reset();
      }
    }

  } else {
    defaultSettings(); //SPIFFS is screwed up.  Load some defaults and wait for manual configuration anyway.
  }
  
}

void loop() {
unsigned long tval = millis();

  prg_button(tval);

  if (!booted && (strlen(config.init) > 0)) {  //Boot time varies with CPU speed, adjust this.
    xmit(config.init);
    booted = 1;
  } else {
    if ((config.delay_blurb > DELAY_MIN) && (strlen(config.blurb) > 0) && (tval % config.delay_blurb < (SLEEP_PERIOD+1000))) {
      xmit(config.blurb);
    } else {
      if ((config.delay_call > DELAY_MIN) && (strlen(config.call) > 0) && (tval % config.delay_call < (SLEEP_PERIOD+1000))) {
        if (strlen(config.call) > 0) {
          xmit(config.call);
        }
      }
    }
  }
    powerSave(SLEEP_PERIOD); //Power down

//Serial and powerSave have a weird interaction.  We need to wait for the line to come back up.
//  If there's no connection, the below delay shouldn't make it into the main loop, which I guess
//  is fine.  Otherwise, we'll consider checking if(Serial) in init, and only starting the
//  command-line once per boot.
//Serial wake time seems to be about 250ms, sometimes more.
  if (Serial) {
    delay(250);
    if (Serial.read() != -1) { //Eat the first keystroke, which is probably space or <CR> or something.
      commandLine();
    }
  }
  
}
