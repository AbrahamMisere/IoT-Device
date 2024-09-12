/***************************************************************************
  This is an example program for the sending a counter to Adafruit IO using
  an ESP8266 WiFi module.  You will need to correct the WiFi SSID and password
  and add your Adafruit IO username and Key.

  written by Theo Fleck and Rick Martin
  03/25/2020
 ***************************************************************************/
#include "Arduino.h"
#include <SoftwareSerial.h>        //Allows us to use two GPIO pins for a second UART
#include "Arduino_SensorKit.h"
 
//uncomment line below if using DHT20 (black sensor)
#define Environment Environment_I2C



SoftwareSerial espSerial(10, 11);  //Create software UART to talk to the ESP8266
String IO_USERNAME = "amisere";
String IO_KEY = "*************";  //key has been removed
String WIFI_SSID = "UD Devices";  //Only need to change if using other network, eduroam won't work with ESP8266
String WIFI_PASS = "";            //Blank for open network
float num = 1.0;                  //Counts up to show upload working
float temp = 0;
float hum = 0;
int COLD = 13;
int HOT = 12;


void setup() {
  Serial.begin(115200);     // set up serial monitor with 115200 baud rate
  espSerial.begin(9600);  // set up software UART to ESP8266 @ 9600 baud rate
  pinMode(HOT, OUTPUT);
  pinMode(COLD,OUTPUT);
  Serial.println("setting up");
  String resp = espData("get_macaddr", 2000, true);      //get MAC address of 8266
  resp = espData("wifi_ssid=" + WIFI_SSID, 2000, true);  //send Wi-Fi SSID to connect to
  resp = espData("wifi_pass=" + WIFI_PASS, 2000, true);  //send password for Wi-Fi network
  resp = espData("io_user=" + IO_USERNAME, 2000, true);  //send Adafruit IO info
  resp = espData("io_key=" + IO_KEY, 2000, true);
  resp = espData("setup_io", 30000, true);  //setup the IoT connection
  if (resp.indexOf("connected") < 0) {
    Serial.println("\nAdafruit IO Connection Failed");
    while (1)
      ;
  }
  resp = espData("setup_feed=1,CPEG-ELEG298", 2000, false);  //start the data feed
  resp = espData("setup_feed=2,Humidity", 2000, false);  //start the data feed
  Serial.println("------ Setup Complete ----------");
  Wire.begin();
 
  //uncomment line below if you're connecting your DHT20 to pin a different than 3
  //Environment.setPin(4);
 
  Environment.begin();
}

void loop() {

  // free version of Adafruit IO only allows 30 uploads/minute, it discards everything else
  delay(5000);  // Wait 5 seconds between uploads
  temp = Environment.readTemperature()*9/5+32;
  hum = Environment.readHumidity();
  if(temp<=76) {
    digitalWrite(COLD, LOW); // turn the LED on (HIGH is the voltage level)
    digitalWrite(HOT, LOW);  // turn the LED off by making the voltage LOW
  }
  else{
    digitalWrite(COLD, HIGH);  // turn the LED off by making the voltage LOW
    digitalWrite(HOT, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if(hum>=99){
    digitalWrite(COLD, LOW); // turn the LED on (HIGH is the voltage level)
    digitalWrite(HOT, HIGH);  // turn the LED off by making the voltage LOW
  }
  Serial.print("Temp is: ");
  Serial.println(temp);
  Serial.print("Humidity is: ");
  Serial.println(hum);
  String resp = espData("send_data=1," + String(temp), 2000, false);  //send feed to cloud
  String resp1 = espData("send_data=2," + String(hum), 2000, false);  //send feed to cloud
  Serial.print("Temperature = ");
  Serial.print(temp); //print temperature
  Serial.println(" F ");
  Serial.print("Humidity = ");
  Serial.print(hum); //print humidity
  Serial.println(" %");
  
}

String espData(String command, const int timeout, boolean debug) {
  String response = "";
  espSerial.println(command);  //send data to ESP8266 using serial UART
  long int time = millis();
  while ((time + timeout) > millis()) {  //wait the timeout period sent with the command
    while (espSerial.available()) {      //look for response from ESP8266
      char c = espSerial.read();
      response += c;
      Serial.print(c);  //print response on serial monitor
    }
  }
  if (debug) {
    Serial.println("Resp: " + response);
  }
  response.trim();
  return response;
}

