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
$Comp
L Isolator:PC817 U2
U 1 1 610E5DA6
P 4750 4950
F 0 "U2" H 4750 5275 50  0001 C CNN
F 1 "PC817" H 4750 5183 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 4550 4750 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 4750 4950 50  0001 L CNN
	1    4750 4950
	1    0    0    -1  
$EndComp
$Comp
L Isolator:PC817 U1
U 1 1 610E5EDD
P 4750 3950
F 0 "U1" H 4750 4275 50  0001 C CNN
F 1 "PC817" H 4750 4183 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 4550 3750 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 4750 3950 50  0001 L CNN
	1    4750 3950
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5050 5050 7900 5050
Wire Wire Line
	7900 5050 7900 4400
Wire Wire Line
	7550 4850 7550 4250
$Comp
L Device:R R1
U 1 1 610ECA4A
P 4050 5050
F 0 "R1" V 3843 5050 50  0001 C CNN
F 1 "500" V 3935 5050 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3980 5050 50  0001 C CNN
F 3 "~" H 4050 5050 50  0001 C CNN
	1    4050 5050
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 610ECC5F
P 5300 4050
F 0 "R3" V 5093 4050 50  0001 C CNN
F 1 "500" V 5185 4050 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5230 4050 50  0001 C CNN
F 3 "~" H 5300 4050 50  0001 C CNN
	1    5300 4050
	0    1    1    0   
$EndComp
Wire Wire Line
	5050 4050 5150 4050
Wire Wire Line
	4450 5050 4200 5050
Wire Wire Line
	7350 4550 8150 4550
Wire Wire Line
	5450 4050 7550 4050
Connection ~ 7550 4250
$Comp
L Isolator:PC817 U3
U 1 1 610F21D3
P 6050 3550
F 0 "U3" H 6050 3875 50  0001 C CNN
F 1 "PC817" H 6050 3783 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 5850 3350 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 6050 3550 50  0001 L CNN
	1    6050 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 3850 6350 3850
Wire Wire Line
	6350 3850 6350 3650
Wire Wire Line
	6350 3450 7350 3450
Wire Wire Line
	7350 3450 7350 4550
$Comp
L Device:R R2
U 1 1 610F453C
P 5300 3650
F 0 "R2" V 5093 3650 50  0001 C CNN
F 1 "500" V 5185 3650 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5230 3650 50  0001 C CNN
F 3 "~" H 5300 3650 50  0001 C CNN
	1    5300 3650
	0    1    1    0   
$EndComp
Wire Wire Line
	5750 3650 5450 3650
Wire Wire Line
	5050 4850 7550 4850
$Comp
L MCU_Module:WeMos_D1_mini U4
U 1 1 61170FA0
P 2650 4300
F 0 "U4" H 2650 3411 50  0001 C CNN
F 1 "WeMos_D1_mini" H 2650 3411 50  0000 C CNN
F 2 "Module:WEMOS_D1_mini_light" H 2650 3150 50  0001 C CNN
F 3 "https://wiki.wemos.cc/products:d1:d1_mini#documentation" H 800 3150 50  0001 C CNN
	1    2650 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 3650 3200 3650
Wire Wire Line
	3200 3650 3200 4100
Wire Wire Line
	3200 4100 3050 4100
Wire Wire Line
	4450 3850 3300 3850
Wire Wire Line
	3300 3850 3300 4200
Wire Wire Line
	3300 4200 3050 4200
Wire Wire Line
	4450 4050 3550 4050
Wire Wire Line
	3550 4050 3550 5100
Wire Wire Line
	3550 5100 2650 5100
Wire Wire Line
	5750 3450 4150 3450
Wire Wire Line
	4450 4850 4150 4850
Wire Wire Line
	4150 4850 4150 3450
Connection ~ 4150 3450
Wire Wire Line
	4150 3450 2750 3450
Wire Wire Line
	3900 5050 3750 5050
Wire Wire Line
	3750 5050 3750 4300
Wire Wire Line
	3750 4300 3050 4300
$Comp
L Connector:Conn_01x01_Female 5V_Heater1
U 1 1 61191526
P 9100 4600
F 0 "5V_Heater1" H 9128 4580 50  0000 L CNN
F 1 "Conn_01x01_Female" H 9128 4535 50  0001 L CNN
F 2 "Connector_Pin:Pin_D1.3mm_L11.0mm" H 9100 4600 50  0001 C CNN
F 3 "~" H 9100 4600 50  0001 C CNN
	1    9100 4600
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Female GND_Heater1
U 1 1 6119FF51
P 9100 4500
F 0 "GND_Heater1" H 9128 4480 50  0000 L CNN
F 1 "Conn_01x01_Female" H 9128 4435 50  0001 L CNN
F 2 "Connector_Pin:Pin_D1.3mm_L11.0mm" H 9100 4500 50  0001 C CNN
F 3 "~" H 9100 4500 50  0001 C CNN
	1    9100 4500
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Female Serial1
U 1 1 611A016D
P 9100 4400
F 0 "Serial1" H 9128 4380 50  0000 L CNN
F 1 "Conn_01x01_Female" H 9128 4335 50  0001 L CNN
F 2 "Connector_Pin:Pin_D1.3mm_L11.0mm" H 9100 4400 50  0001 C CNN
F 3 "~" H 9100 4400 50  0001 C CNN
	1    9100 4400
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Female GND_In1
U 1 1 611A02A0
P 8950 3950
F 0 "GND_In1" H 8978 3930 50  0000 L CNN
F 1 "Conn_01x01_Female" H 8978 3885 50  0001 L CNN
F 2 "Connector_Pin:Pin_D1.3mm_L11.0mm" H 8950 3950 50  0001 C CNN
F 3 "~" H 8950 3950 50  0001 C CNN
	1    8950 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8750 3850 7850 3850
Wire Wire Line
	7850 3850 7850 3150
Wire Wire Line
	7850 3150 2750 3150
Wire Wire Line
	2750 3150 2750 3450
Connection ~ 2750 3450
Wire Wire Line
	2750 3450 2750 3500
Wire Wire Line
	8750 3950 8300 3950
Wire Wire Line
	8300 3950 8300 2800
Wire Wire Line
	8300 2800 2000 2800
Wire Wire Line
	2000 2800 2000 5100
Wire Wire Line
	2000 5100 2650 5100
Connection ~ 2650 5100
Wire Wire Line
	7550 4050 7550 4250
$Comp
L Connector:Conn_01x01_Female 3.3V_In1
U 1 1 611A0527
P 8950 3850
F 0 "3.3V_In1" H 8978 3830 50  0000 L CNN
F 1 "Conn_01x01_Female" H 8978 3785 50  0001 L CNN
F 2 "Connector_Pin:Pin_D1.3mm_L11.0mm" H 8950 3850 50  0001 C CNN
F 3 "~" H 8950 3850 50  0001 C CNN
	1    8950 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 4600 8150 4550
Wire Wire Line
	8150 4600 8900 4600
Wire Wire Line
	8400 4500 8400 4400
Wire Wire Line
	7900 4400 8400 4400
Wire Wire Line
	8400 4500 8900 4500
Wire Wire Line
	8900 4400 8500 4400
Wire Wire Line
	8500 4400 8500 4250
Wire Wire Line
	7550 4250 8500 4250
$EndSCHEMATC
