#include <OneWire.h>
#include <stdlib.h>
/* Below all the parameters and data types needed will be defined
 *
 *
 */
// Initializing pin for temperature sensor
int DS18S20_Pin = 2; // Signal pin on digital 2 on the Arduino Uno
OneWire ds(DS18S20_Pin); // Creating a OneWire object
//Declaring Variables for temperature sensor
float TemperatureSum; // The avrage of a samples taken from one temp test will
be stored in this variable
byte data[12]; // Variable to temporary hold readings
byte addr[8]; // Variable to temporary hold the memory adress of the readings
// Initializing pins for the relay boards controlling the pumps
const int in_1 = 7 ; // Declaring pump to digital port 7
const int in_2 = 8 ; // Declaring pump to digital port 8
// Initilizing pins for EC sensor
const int ECPin= A1; // Assigning analog port 1 reference pin
const int ECGround = A2; // Assigning analog port 2 as ground level
const int ECPower = A3; // Assigning analog port 3 as power pin
//Declaring variables for EC sensor
int R1= 1000; // Declaring the internal resistance to variable R1
int Ra=25; // Declaring the powering pin resistance to variable Ra
float EC=0; // Declaring variable for EC-value
float EC25 =0; // Declaring variable for the equivialent EC-value at temp 25
degree celcius
float K=1.44; // trail measured coefficient
float TemperatureCoef = 0.019; // Temperature coefficient for water that is
estimated by trials.
float raw= 0; // The raw data from a EC-sensor reading is declared to this
variable
float Vin= 5; // The internal voltage supply from Arduino Uno to a analog pin
float Vdrop= 0; // The voltage drop measured from a EC-reading will be store
here
float Rc= 0; // The voltage of the water solvent
// Initilizing pin for pH sensor
const int pHpin = A0; // Declaring analog port 0 to the pH-sensor probe
//Declaring Variables for pH sensor
unsigned long int avgValue; //Store the average value of the sensor feedback
float phValue; // Storing the calculated pH reading in assinged to this
variable
int buf[10],temp; // Temporary variable used to sort the 10 pH reading samples
// WiFi network setup
#define SSID "A WiFi network" // The name of the WiFI network to connect
to is entered here
#define PASS "password" // The password of the WiFI network to connect to
is entered here
//ThingSpeak channel setup
#define IP "184.106.153.149"// The ip-adress for thingspeak.com
String msg = "GET /update?key=94IADK04DP5YY184"; //A GET parameter associated
with the personal thingspeak channel
//____________________________________
/*
* In this section the setup for the pins are defined
*
*/
void setup(){
 Serial.begin(115200); // Setting the baudrate


 pinMode(in_1,OUTPUT); // Setting pin modes for the relay board controlling
the pump feeding pH down buffer solution
 pinMode(in_2,OUTPUT); // Setting pin modes for the relay board controlling
the pump feeding nutrient solution
 pinMode(ECPin,INPUT); // Setting pin mode for the input pin for the EC
sensor probe
 pinMode(ECPower,OUTPUT); // Setting pin mode for sourcing current
 pinMode(ECGround,OUTPUT); // Setting pin mode for sinking current
 digitalWrite(ECGround,LOW); // Setting ground level for the EC sensor probe
 R1=(R1+Ra);// Taking into account Powering Pin Resitance

 Serial.println("AT"); //Hayes command call for attention
 delay(5000); // 5000 ms delay
 connectWiFi(); //Calling on function to connect to WiFI
}
//__________________________________________
/*
* This is the main loop that will operate the sensor readings, control the
pumps, and call on 
* the funtion to upload data to the ThingSpeak channel
*
*/
void loop(){
 //_____Checking water temperature___________
 if ( !ds.search(addr)) {
 //no more sensors on chain, reset search
 ds.reset_search();
 return -1000;
 }
 ds.reset(); // Reset the 1-wire bus
 ds.select(addr); // Selecting the adress of the device to operate
 ds.write(0x44,1); // Write a byte, and leave power applied to the 1 wire bus.
 byte present = ds.reset();
 ds.select(addr); // Selecting the adress of the device to operate
 ds.write(0xBE); // write to the temperature sensors RAM at this adress

 for (int i = 0; i < 9; i++) { // we need 9 bytes
 data[i] = ds.read(); // Reading the nine bytes stored in the temperature
sensors RAM
 }

 ds.reset_search(); //resetting the OneWire device search

 byte MSB = data[1]; //storing the temp reading
 byte LSB = data[0]; //storing the temp reading
 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 TemperatureSum = tempRead / 16; // converting from hexa
 //_____Checking pH-value______________

 for(int i=0;i<10;i++) //Get 10 sample value from the sensor for smooth
the value
 {
 buf[i]=analogRead(pHpin); //Taking ten analog readings from the pH probe

 delay(10); // 10 ms delay
 }
 for(int i=0;i<9;i++) //sort the 10 analog readings from small to large
 {
 for(int j=i+1;j<10;j++)
 {
 if(buf[i]>buf[j])
 {
 temp=buf[i];
 buf[i]=buf[j];
 buf[j]=temp;
 }
 }
 }

 avgValue=0; // a temporary parameter for the pH readings
 for(int i=2;i<8;i++){ //take value of the 6 center samples
 avgValue+=buf[i];
 }
 avgValue=avgValue/6; //take avrage value of the 6 center samples
 phValue=(avgValue*5.0)/1024; //convert the analog readings into volt
 phValue=3.157895*phValue+0.9152632; //convert the millivolt into pH value.
Floats are coeffiocients from sensor calibration
 //______Checking EC level_______________
 // Estimates Resistance of Liquid
 digitalWrite(ECPower,HIGH); //Setting the power pin for EC sensor to high
 raw= analogRead(ECPin);
 raw= analogRead(ECPin);// First reading will be inconclusive due to low
charge in probe
 digitalWrite(ECPower,LOW); //Setting the power pin for EC sensor to low


 // Convert voltage to EC
 Vdrop= (Vin*raw)/1024.0; // The voltage drop measured
 Rc=(Vdrop*R1)/(Vin-Vdrop); // The resistance of the water solvent
 Rc=Rc-Ra; //acounting for Digital Pin Resitance
 EC = 1000/(Rc*K); // The calculate EC value


 // Compensating For the temperature in the water solvent//
 EC25 = EC/ (1+ TemperatureCoef*(TemperatureSum-25.0)); //
 updateTS(); //Calling on funtion to update the ThingSpeak channel with new
data
 //_________DC motor control pumps___________

 if (EC25<1.4 && phValue>6.6) { //if the nutrient level unsufficient, and
the pH value to high

 digitalWrite(in_1,HIGH) ; //Sending signal to relay to power up
nutrient solution pump
 delay(2000); // Running pump for 2 seconds
 digitalWrite(in_1,LOW) ; //Cutting power to pump
 }
 else if (EC25>1.4 && phValue>6.6){ //if the nutrient level sufficient and
the pH value to high
 digitalWrite(in_2,HIGH) ; //Sending signal to relay to power up pH
down buffer solution pump
 delay(2000); // Running pump for 2 seconds
 digitalWrite(in_2,LOW) ; //Cutting power to pump
 }
 else if (EC25<1.4 && phValue<6.6) { //if the nutrient level sufficient
and the pH value sufficient
 digitalWrite(in_1,HIGH) ;//Sending signal to relay to power up nutrient
solution pump
 delay(2000); // Running pump for 2 seconds
 digitalWrite(in_1,LOW) ; //Cutting power to pump
 }
 delay(60000); //Wait 100 second before starting next loop iteration
}
 //______ThingSpeak channel update function___________
void updateTS(){
 String cmd = "AT+CIPSTART=\"TCP\",\""; // A serial command to intruct the
WiFi chip
 cmd += IP; //Adding the string containing the IP for ThingSpeak
 cmd += "\",80"; // The port to communicate with ThingSpeak through
 Serial.println(cmd); //Establishing connection with ThingSpeak
 delay(2000); // time delay