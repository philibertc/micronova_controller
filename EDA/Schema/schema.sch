EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "MicroNova Controller"
Date "August 2021"
Rev "1.0"
Comp "philibertc"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 1500 2450 0    50   Input ~ 0
RX
Text GLabel 1500 2900 0    50   Input ~ 0
TX
$Comp
L Isolator:PC817 U?
U 1 1 610E5DA6
P 3000 3250
F 0 "U?" H 3000 3575 50  0001 C CNN
F 1 "PC817" H 3000 3483 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 2800 3050 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 3000 3250 50  0001 L CNN
	1    3000 3250
	1    0    0    -1  
$EndComp
$Comp
L Isolator:PC817 U?
U 1 1 610E5EDD
P 3000 2250
F 0 "U?" H 3000 2575 50  0001 C CNN
F 1 "PC817" H 3000 2483 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 2800 2050 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 3000 2250 50  0001 L CNN
	1    3000 2250
	-1   0    0    -1  
$EndComp
Text GLabel 1500 2750 0    50   Input ~ 0
3.3V_ESP
Wire Wire Line
	2700 3150 2700 2750
Wire Wire Line
	2700 2750 2200 2750
Text GLabel 6500 2550 2    50   Input ~ 0
Stove_Serial
Text GLabel 6500 2850 2    50   Input ~ 0
5V_Stove
Text GLabel 6500 2700 2    50   Input ~ 0
0V_Stove
Text GLabel 1500 2600 0    50   Input ~ 0
0V_ESP
Wire Wire Line
	3300 3350 6150 3350
Wire Wire Line
	6150 3350 6150 2700
Wire Wire Line
	2700 2350 1850 2350
Wire Wire Line
	1850 2350 1850 2600
Wire Wire Line
	1850 2600 1500 2600
Wire Wire Line
	2700 2150 1700 2150
Wire Wire Line
	1700 2150 1700 2450
Wire Wire Line
	1700 2450 1500 2450
Wire Wire Line
	5800 3150 5800 2550
$Comp
L Device:R R?
U 1 1 610ECA4A
P 2300 3350
F 0 "R?" V 2093 3350 50  0001 C CNN
F 1 "500" V 2185 3350 50  0000 C CNN
F 2 "" V 2230 3350 50  0001 C CNN
F 3 "~" H 2300 3350 50  0001 C CNN
	1    2300 3350
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 610ECC5F
P 3550 2350
F 0 "R?" V 3343 2350 50  0001 C CNN
F 1 "500" V 3435 2350 50  0000 C CNN
F 2 "" V 3480 2350 50  0001 C CNN
F 3 "~" H 3550 2350 50  0001 C CNN
	1    3550 2350
	0    1    1    0   
$EndComp
Wire Wire Line
	3300 2350 3400 2350
Wire Wire Line
	2700 3350 2450 3350
Wire Wire Line
	6150 2700 6500 2700
Wire Wire Line
	5800 2550 6500 2550
Wire Wire Line
	2150 3350 2150 2900
Wire Wire Line
	2150 2900 1500 2900
Wire Wire Line
	5600 2850 6500 2850
Wire Wire Line
	3700 2350 5800 2350
Wire Wire Line
	5800 2350 5800 2550
Connection ~ 5800 2550
$Comp
L Isolator:PC817 U?
U 1 1 610F21D3
P 4300 1850
F 0 "U?" H 4300 2175 50  0001 C CNN
F 1 "PC817" H 4300 2083 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 4100 1650 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 4300 1850 50  0001 L CNN
	1    4300 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 2150 4600 2150
Wire Wire Line
	4600 2150 4600 1950
Wire Wire Line
	4600 1750 5600 1750
Wire Wire Line
	5600 1750 5600 2850
Text GLabel 1500 2300 0    50   Input ~ 0
Enable_RX
$Comp
L Device:R R?
U 1 1 610F453C
P 3550 1950
F 0 "R?" V 3343 1950 50  0001 C CNN
F 1 "500" V 3435 1950 50  0000 C CNN
F 2 "" V 3480 1950 50  0001 C CNN
F 3 "~" H 3550 1950 50  0001 C CNN
	1    3550 1950
	0    1    1    0   
$EndComp
Wire Wire Line
	4000 1950 3700 1950
Wire Wire Line
	3400 1950 1550 1950
Wire Wire Line
	1550 1950 1550 2300
Wire Wire Line
	1550 2300 1500 2300
Wire Wire Line
	4000 1750 2200 1750
Wire Wire Line
	2200 1750 2200 2750
Connection ~ 2200 2750
Wire Wire Line
	2200 2750 1500 2750
Wire Wire Line
	3300 3150 5800 3150
$EndSCHEMATC
