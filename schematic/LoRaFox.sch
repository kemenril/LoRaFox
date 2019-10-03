EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:ssd1306
LIBS:lora_sx1278
LIBS:ssd1306_i2c
LIBS:nodemcu_esp32
LIBS:LoRaFox-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L LoRa_SX1278 J?
U 1 1 5D953108
P 3500 2200
F 0 "J?" H 3500 2850 50  0000 C CNN
F 1 "LoRa_SX1278" V 3600 2200 50  0000 C CNN
F 2 "" H 3500 2200 50  0001 C CNN
F 3 "" H 3500 2200 50  0001 C CNN
	1    3500 2200
	0    -1   -1   0   
$EndComp
$Comp
L SSD1306_I2C J?
U 1 1 5D9537B4
P 7000 2550
F 0 "J?" H 7000 2800 50  0000 C CNN
F 1 "SSD1306_I2C" V 7100 2550 50  0000 C CNN
F 2 "" H 7000 2550 50  0001 C CNN
F 3 "" H 7000 2550 50  0001 C CNN
	1    7000 2550
	0    -1   -1   0   
$EndComp
$Comp
L NodeMCU_ESP32 J?
U 1 1 5D9544C8
P 5600 4150
F 0 "J?" H 5000 5000 50  0000 C CNN
F 1 "NodeMCU_ESP32" V 5100 4200 50  0000 C CNN
F 2 "" H 5000 4200 50  0001 C CNN
F 3 "" H 5000 4200 50  0001 C CNN
	1    5600 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3250 2400 3250 2850
Wire Wire Line
	3250 2850 6400 2850
Wire Wire Line
	6400 2850 6400 3800
Wire Wire Line
	6400 3800 5750 3800
Wire Wire Line
	3650 2400 3650 3000
Wire Wire Line
	3650 3000 4800 3000
Wire Wire Line
	2950 2400 2950 4700
Wire Wire Line
	2950 4700 4800 4700
Wire Wire Line
	6850 2750 6850 3000
Wire Wire Line
	6850 3000 5750 3000
Wire Wire Line
	6950 2750 4850 2750
Wire Wire Line
	4850 2750 4850 3000
Wire Wire Line
	5750 4200 7150 4200
Wire Wire Line
	7150 4200 7150 2750
Wire Wire Line
	4800 4000 3450 4000
Wire Wire Line
	3450 4000 3450 2400
Wire Wire Line
	5750 3700 6100 3700
Wire Wire Line
	6100 3700 6100 2600
Wire Wire Line
	6100 2600 3550 2600
Wire Wire Line
	3550 2600 3550 2400
Wire Wire Line
	5750 4100 7400 4100
Wire Wire Line
	7400 4100 7400 2550
Text Label 7550 2550 0    60   ~ 0
Disp_RST
Wire Wire Line
	5750 4500 7050 4500
Wire Wire Line
	7050 4500 7050 2750
Wire Wire Line
	5750 3900 6600 3900
Wire Wire Line
	6600 3900 6600 2500
Wire Wire Line
	6600 2500 3350 2500
Wire Wire Line
	3350 2500 3350 2400
Wire Wire Line
	4800 4100 3050 4100
Wire Wire Line
	3050 4100 3050 2400
Wire Wire Line
	4800 3900 3150 3900
Wire Wire Line
	3150 3900 3150 2400
Wire Wire Line
	4800 3700 3950 3700
Wire Wire Line
	3950 3700 3950 2400
Text Label 3950 3700 0    60   ~ 0
A1-ADC5-GPIO33
Wire Wire Line
	4800 4200 3850 4200
Wire Wire Line
	3850 4200 3850 2400
$EndSCHEMATC
