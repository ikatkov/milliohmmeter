EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev "1"
Comp ""
Comment1 "Design for JLCPCB 1-2 Layer Service"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Transistor_FET:Si4532DY Q1
U 2 1 61AC9576
P 4500 2550
F 0 "Q1" H 4705 2596 50  0000 L CNN
F 1 "Si9942DY" H 4705 2505 50  0000 L CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 4700 2475 50  0001 L CNN
F 3 "https://www.onsemi.com/pub/Collateral/SI4532DY-D.PDF" H 4600 2550 50  0001 L CNN
	2    4500 2550
	1    0    0    1   
$EndComp
$Comp
L Transistor_FET:Si4532DY Q1
U 1 1 61ACD011
P 3950 3200
F 0 "Q1" H 4155 3246 50  0000 L CNN
F 1 "Si9942DY" H 4155 3155 50  0000 L CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 4150 3125 50  0001 L CNN
F 3 "https://www.onsemi.com/pub/Collateral/SI4532DY-D.PDF" H 4050 3200 50  0001 L CNN
	1    3950 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 3400 4050 3500
Wire Wire Line
	4300 2550 4050 2550
Wire Wire Line
	4050 2550 4050 3000
Wire Wire Line
	3750 3200 3650 3200
Wire Wire Line
	4600 2950 4600 2750
Text GLabel 4600 2950 3    50   Input ~ 0
ADC
Text GLabel 3400 3200 0    50   Input ~ 0
VCC
$Comp
L Device:R_US R1
U 1 1 61AD192D
P 4050 2400
F 0 "R1" H 4118 2446 50  0000 L CNN
F 1 "1M" H 4118 2355 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Vertical" V 4090 2390 50  0001 C CNN
F 3 "~" H 4050 2400 50  0001 C CNN
	1    4050 2400
	1    0    0    -1  
$EndComp
Connection ~ 4050 2550
Wire Wire Line
	4050 2250 4050 2150
Wire Wire Line
	4050 2150 4600 2150
Wire Wire Line
	4600 2150 4600 2350
Wire Wire Line
	4600 2150 4600 1900
Connection ~ 4600 2150
Text GLabel 4600 1900 1    50   Input ~ 0
BATT
$Comp
L Connector:Conn_01x04_Male J1
U 1 1 61AD81BC
P 5650 2350
F 0 "J1" H 5758 2631 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5758 2540 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 5650 2350 50  0001 C CNN
F 3 "~" H 5650 2350 50  0001 C CNN
	1    5650 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 2250 6100 2250
Wire Wire Line
	5850 2350 6100 2350
Wire Wire Line
	5850 2450 6100 2450
Wire Wire Line
	5850 2550 6100 2550
Text GLabel 4050 3700 3    50   Input ~ 0
GND
Text GLabel 6100 2250 2    50   Input ~ 0
GND
Text GLabel 6100 2350 2    50   Input ~ 0
VCC
Text GLabel 6100 2450 2    50   Input ~ 0
ADC
Text GLabel 6100 2550 2    50   Input ~ 0
BATT
$Comp
L Device:R_US R2
U 1 1 61ADE2C4
P 3650 3350
F 0 "R2" H 3718 3396 50  0000 L CNN
F 1 "10k" H 3718 3305 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Vertical" V 3690 3340 50  0001 C CNN
F 3 "~" H 3650 3350 50  0001 C CNN
	1    3650 3350
	1    0    0    -1  
$EndComp
Connection ~ 3650 3200
Wire Wire Line
	3650 3200 3400 3200
Wire Wire Line
	3650 3500 4050 3500
Connection ~ 4050 3500
Wire Wire Line
	4050 3500 4050 3700
Text Label 4650 2050 0    50   ~ 10
4.11V
Text Label 4050 3000 0    50   ~ 10
3.68V
Text Label 3550 3150 0    50   ~ 10
0V
Text Label 4100 3500 0    50   ~ 10
0V
Text Notes 4650 2400 0    50   ~ 0
source
Text Notes 4650 2750 0    50   ~ 0
drain\n
Text Notes 4100 3100 0    50   ~ 0
drain\n
Text Notes 4100 3350 0    50   ~ 0
source
$EndSCHEMATC
