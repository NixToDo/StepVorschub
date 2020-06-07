EESchema Schematic File Version 4
LIBS:StepVorschub-cache
EELAYER 29 0
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
L Connector_Generic:Conn_01x09 J?
U 1 1 5EDCB7BF
P 7900 4150
F 0 "J?" H 7850 4650 50  0000 L CNN
F 1 "Spindle Encoder and Power" H 7650 3600 50  0000 L CNN
F 2 "" H 7900 4150 50  0001 C CNN
F 3 "~" H 7900 4150 50  0001 C CNN
	1    7900 4150
	1    0    0    -1  
$EndComp
$Comp
L Display_Character:EA_T123X-I2C U?
U 1 1 5EDCCE15
P 3550 3150
F 0 "U?" H 3980 3196 50  0000 L CNN
F 1 "SSD1306" H 3980 3105 50  0000 L CNN
F 2 "" H 3550 2550 50  0001 C CNN
F 3 "" H 3550 2650 50  0001 C CNN
	1    3550 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:Rotary_Encoder_Switch SW?
U 1 1 5EDCDA20
P 3550 4500
F 0 "SW?" H 3550 4867 50  0000 C CNN
F 1 "Manual Rotary Encoder with Switch" H 3550 4776 50  0000 C CNN
F 2 "" H 3400 4660 50  0001 C CNN
F 3 "~" H 3550 4760 50  0001 C CNN
	1    3550 4500
	1    0    0    -1  
$EndComp
Text GLabel 6200 4050 2    50   BiDi ~ 0
SDA
Text GLabel 6200 4150 2    50   Output ~ 0
SCL
Text GLabel 3000 2850 0    50   Output ~ 0
SCL
Text GLabel 3000 2950 0    50   BiDi ~ 0
SDA
Wire Wire Line
	3050 2850 3000 2850
Wire Wire Line
	3000 2950 3050 2950
$Comp
L power:+5V #PWR?
U 1 1 5EDC9FB6
P 3550 2650
F 0 "#PWR?" H 3550 2500 50  0001 C CNN
F 1 "+5V" H 3565 2823 50  0000 C CNN
F 2 "" H 3550 2650 50  0001 C CNN
F 3 "" H 3550 2650 50  0001 C CNN
	1    3550 2650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDCA8FC
P 3550 3650
F 0 "#PWR?" H 3550 3400 50  0001 C CNN
F 1 "GND" H 3555 3477 50  0000 C CNN
F 2 "" H 3550 3650 50  0001 C CNN
F 3 "" H 3550 3650 50  0001 C CNN
	1    3550 3650
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR?
U 1 1 5EDCB493
P 5550 2650
F 0 "#PWR?" H 5550 2500 50  0001 C CNN
F 1 "VCC" H 5567 2823 50  0000 C CNN
F 2 "" H 5550 2650 50  0001 C CNN
F 3 "" H 5550 2650 50  0001 C CNN
	1    5550 2650
	1    0    0    -1  
$EndComp
NoConn ~ 3450 2650
NoConn ~ 3050 3450
$Comp
L power:GND #PWR?
U 1 1 5EDCBC83
P 3850 4600
F 0 "#PWR?" H 3850 4350 50  0001 C CNN
F 1 "GND" H 3855 4427 50  0000 C CNN
F 2 "" H 3850 4600 50  0001 C CNN
F 3 "" H 3850 4600 50  0001 C CNN
	1    3850 4600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDCC144
P 3250 4500
F 0 "#PWR?" H 3250 4250 50  0001 C CNN
F 1 "GND" V 3255 4372 50  0000 R CNN
F 2 "" H 3250 4500 50  0001 C CNN
F 3 "" H 3250 4500 50  0001 C CNN
	1    3250 4500
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDCC820
P 5750 4650
F 0 "#PWR?" H 5750 4400 50  0001 C CNN
F 1 "GND" H 5800 4500 50  0000 C CNN
F 2 "" H 5750 4650 50  0001 C CNN
F 3 "" H 5750 4650 50  0001 C CNN
	1    5750 4650
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5EDCD28A
P 5850 2650
F 0 "#PWR?" H 5850 2500 50  0001 C CNN
F 1 "+5V" H 5865 2823 50  0000 C CNN
F 2 "" H 5850 2650 50  0001 C CNN
F 3 "" H 5850 2650 50  0001 C CNN
	1    5850 2650
	1    0    0    -1  
$EndComp
NoConn ~ 5750 2650
$Comp
L MCU_Module:Arduino_Nano_v2.x A?
U 1 1 5EDCE02A
P 5650 3650
F 0 "A?" H 5300 2700 50  0000 C CNN
F 1 "Arduino Nano" H 6100 2700 50  0000 C CNN
F 2 "" H 5800 2700 50  0001 L CNN
F 3 "" H 5650 2650 50  0001 C CNN
	1    5650 3650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDCCD26
P 5650 4650
F 0 "#PWR?" H 5650 4400 50  0001 C CNN
F 1 "GND" H 5600 4500 50  0000 C CNN
F 2 "" H 5650 4650 50  0001 C CNN
F 3 "" H 5650 4650 50  0001 C CNN
	1    5650 4650
	1    0    0    -1  
$EndComp
Text GLabel 3900 4400 2    50   Output ~ 0
Button
Text GLabel 6200 3850 2    50   Input ~ 0
Button
Wire Wire Line
	6150 3850 6200 3850
Wire Wire Line
	6200 4050 6150 4050
Wire Wire Line
	6150 4150 6200 4150
Wire Wire Line
	3850 4400 3900 4400
Text GLabel 3200 4400 0    50   Output ~ 0
MENC_A
Text GLabel 3200 4600 0    50   Output ~ 0
MENC_B
Wire Wire Line
	3200 4400 3250 4400
Wire Wire Line
	3200 4600 3250 4600
Text GLabel 6200 3650 2    50   Input ~ 0
MENC_A
Text GLabel 6200 3750 2    50   Input ~ 0
MENC_B
Wire Wire Line
	6200 3650 6150 3650
Wire Wire Line
	6200 3750 6150 3750
$Comp
L power:GND #PWR?
U 1 1 5EDD1D5D
P 7700 2800
F 0 "#PWR?" H 7700 2550 50  0001 C CNN
F 1 "GND" V 7705 2672 50  0000 R CNN
F 2 "" H 7700 2800 50  0001 C CNN
F 3 "" H 7700 2800 50  0001 C CNN
	1    7700 2800
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDD267C
P 7700 3750
F 0 "#PWR?" H 7700 3500 50  0001 C CNN
F 1 "GND" V 7705 3622 50  0000 R CNN
F 2 "" H 7700 3750 50  0001 C CNN
F 3 "" H 7700 3750 50  0001 C CNN
	1    7700 3750
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDD2BA9
P 7700 4250
F 0 "#PWR?" H 7700 4000 50  0001 C CNN
F 1 "GND" V 7705 4122 50  0000 R CNN
F 2 "" H 7700 4250 50  0001 C CNN
F 3 "" H 7700 4250 50  0001 C CNN
	1    7700 4250
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5EDD32DE
P 7700 4350
F 0 "#PWR?" H 7700 4100 50  0001 C CNN
F 1 "GND" V 7705 4222 50  0000 R CNN
F 2 "" H 7700 4350 50  0001 C CNN
F 3 "" H 7700 4350 50  0001 C CNN
	1    7700 4350
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5EDD34C7
P 7700 3200
F 0 "#PWR?" H 7700 3050 50  0001 C CNN
F 1 "+5V" V 7700 3350 50  0000 L CNN
F 2 "" H 7700 3200 50  0001 C CNN
F 3 "" H 7700 3200 50  0001 C CNN
	1    7700 3200
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x05 J?
U 1 1 5EDD42D4
P 7900 3000
F 0 "J?" H 7850 3300 50  0000 L CNN
F 1 "Stepper Driver Output" H 7700 2650 50  0000 L CNN
F 2 "" H 7900 3000 50  0001 C CNN
F 3 "~" H 7900 3000 50  0001 C CNN
	1    7900 3000
	1    0    0    -1  
$EndComp
Text GLabel 7650 2900 0    50   Input ~ 0
Enable
Text GLabel 7650 3000 0    50   Input ~ 0
Direction
Text GLabel 7650 3100 0    50   Input ~ 0
Step
Wire Wire Line
	7650 2900 7700 2900
Wire Wire Line
	7700 3000 7650 3000
Wire Wire Line
	7650 3100 7700 3100
Text GLabel 5100 4150 0    50   Output ~ 0
Enable
Text GLabel 5100 4050 0    50   Output ~ 0
Direction
Text GLabel 5100 3950 0    50   Output ~ 0
Step
Wire Wire Line
	5150 3950 5100 3950
Wire Wire Line
	5100 4050 5150 4050
Wire Wire Line
	5150 4150 5100 4150
Text GLabel 7650 3850 0    50   Output ~ 0
SENC_A
Text GLabel 7650 3950 0    50   Output ~ 0
SENC_B
Text GLabel 7650 4050 0    50   Output ~ 0
SENC_Z
Wire Wire Line
	7700 3850 7650 3850
Wire Wire Line
	7650 3950 7700 3950
Wire Wire Line
	7700 4050 7650 4050
$Comp
L power:+5V #PWR?
U 1 1 5EDD7B80
P 7700 4150
F 0 "#PWR?" H 7700 4000 50  0001 C CNN
F 1 "+5V" V 7700 4300 50  0000 L CNN
F 2 "" H 7700 4150 50  0001 C CNN
F 3 "" H 7700 4150 50  0001 C CNN
	1    7700 4150
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR?
U 1 1 5EDD9F60
P 7700 4550
F 0 "#PWR?" H 7700 4400 50  0001 C CNN
F 1 "VCC" V 7700 4700 50  0000 L CNN
F 2 "" H 7700 4550 50  0001 C CNN
F 3 "" H 7700 4550 50  0001 C CNN
	1    7700 4550
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR?
U 1 1 5EDD903B
P 7700 4450
F 0 "#PWR?" H 7700 4300 50  0001 C CNN
F 1 "VCC" V 7700 4600 50  0000 L CNN
F 2 "" H 7700 4450 50  0001 C CNN
F 3 "" H 7700 4450 50  0001 C CNN
	1    7700 4450
	0    -1   -1   0   
$EndComp
Text GLabel 5100 3450 0    50   Input ~ 0
SENC_A
Text GLabel 5100 3550 0    50   Input ~ 0
SENC_B
Text GLabel 5100 3350 0    50   Input ~ 0
SENC_Z
Wire Wire Line
	5100 3350 5150 3350
Wire Wire Line
	5150 3450 5100 3450
Wire Wire Line
	5100 3550 5150 3550
NoConn ~ 5150 3050
NoConn ~ 5150 3150
NoConn ~ 5150 3250
NoConn ~ 5150 3650
NoConn ~ 5150 3750
NoConn ~ 5150 3850
NoConn ~ 5150 4250
NoConn ~ 5150 4350
NoConn ~ 6150 4350
NoConn ~ 6150 4250
NoConn ~ 6150 3950
NoConn ~ 6150 3450
NoConn ~ 6150 3150
NoConn ~ 6150 3050
Text Notes 8050 3100 0    50   ~ 0
Stepper drivers have typicaly opto isolated inputs.\nConnected these outputs to the coresponding inputs\nand all return pins to the +5V pin.
Text Notes 8050 4100 0    50   ~ 0
The spindle quadrature encoder get its power from the +5V and GND.\nThe current consumtion should be low enough\nnot to overload the Arduino Nano voltage regulator.\nAll signals are at 0 / 5V level.
Text Notes 8050 4450 0    50   ~ 0
Here comes the supply voltage in.\nRange: 7 - 12V
Text Notes 2900 2400 0    50   ~ 0
This symbol shows an different GLCD.\nI used an SSD1306.
$EndSCHEMATC
