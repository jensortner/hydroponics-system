# hydroponics-system
Bachelor Thesis Project for a embedded IoT system.

University :Royal Institute of Technology

KTHCourse :MF133X, Degree Project in Mechatronics
TRITA number :TRITA-ITM-EX 2019:61
Authors : Jens Ortner and Erik Ågren
Name of the program : CMAST16
Name of the project : Autonomated Hydroponics
Finalized :2019-05-29
 
 
This code is used to control an Arduino Uno that has three
sensor connected to it:

1) pH sensor.
2) EC psensor.
3) Water temperature sensor.
   
The Arduino also controls two 12V liquid pumps via two
separete relay boards.
The main idea of the setup is to make the feeding and controlling of
a hydroponics system. The code is designed to automaticly take pH, water
temp and EC
of a water solvent and add pH down buffer solution and/or nutrient solution
to the water solvent through one of the two pumps.
 
The readings from the different sensor reading is uploaded every loop
iteration to a Thinkspeak IoT channel via a WiFi chip.
