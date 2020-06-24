EESchema Schematic File Version 4
EELAYER 30 0
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
L MDBT42Q:MDBT42Q U?
U 1 1 5EEFDFBC
P 5900 3300
F 0 "U?" H 6200 4400 50  0000 C CNN
F 1 "MDBT42Q" H 6300 4300 50  0000 C CNN
F 2 "MDBT42Q:MDBT42Q" H 5900 3500 50  0001 C CNN
F 3 "https://statics3.seeedstudio.com/assets/file/bazaar/product/MDBT42Q-Version_B.pdf" H 7700 2900 50  0001 C CNN
	1    5900 3300
	1    0    0    -1  
$EndComp
$Comp
L Memory_Flash:MX25R3235FM1xx0 U?
U 1 1 5EF01A4F
P 4200 2700
F 0 "U?" H 4250 3200 50  0000 C CNN
F 1 "MX25R6435FM2IH0" H 3950 3100 50  0000 C CNN
F 2 "Package_SO:SOP-8_3.9x4.9mm_P1.27mm" H 4200 2100 50  0001 C CNN
F 3 "http://www.macronix.com/Lists/Datasheet/Attachments/7534/MX25R3235F,%20Wide%20Range,%2032Mb,%20v1.6.pdf" H 4200 2700 50  0001 C CNN
	1    4200 2700
	-1   0    0    -1  
$EndComp
$Comp
L barkbit:TLV61225 U?
U 1 1 5EF035C4
P 5950 1350
F 0 "U?" H 5950 1625 50  0000 C CNN
F 1 "TLV61225" H 5950 1534 50  0000 C CNN
F 2 "" H 5950 1350 50  0001 C CNN
F 3 "" H 5950 1350 50  0001 C CNN
	1    5950 1350
	1    0    0    -1  
$EndComp
$Comp
L barkbit:MMA8451Q U?
U 1 1 5EF084C8
P 8200 2800
F 0 "U?" H 8650 2650 50  0000 C CNN
F 1 "MMA8451Q" H 8800 2750 50  0000 C CNN
F 2 "" H 8250 2900 50  0001 C CNN
F 3 "" H 8250 2900 50  0001 C CNN
	1    8200 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 1850 6000 1850
Wire Wire Line
	8100 1850 8100 2300
Wire Wire Line
	8100 1850 8200 1850
Wire Wire Line
	8200 1850 8200 2300
Connection ~ 8100 1850
Wire Wire Line
	6000 1850 6000 2300
Wire Wire Line
	4000 1850 4000 2300
Wire Wire Line
	6000 1850 8100 1850
Connection ~ 6000 1850
$EndSCHEMATC
