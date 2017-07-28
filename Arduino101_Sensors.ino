/*
Arduino code for Salvation Module
by the following members of Makerspace Puerto Rico (https://www.facebook.com/search/str/makerspace+pr)
  José Miranda
  Ramiro Vidal
  Alex Martínez
  Edwin Delgado
  Damaso Cardenales
  
  Made for the Intel Hacks 2017 Hackathon. Submitted on 07/27/2017.
  
  This code is designed to work with the Arduino 101 board, but it should work fine in other boards with a Serial1 interface.
  It can also be easily adapted to work with any Arduino board, albeit with the requirement of using Software Serial in some cases.
*/


//*****************
//Load Includes
//*****************
#include <DHT.h>
#include <CurieIMU.h>
#include <neotimer.h>   //https://github.com/jrullan/neotimer

// Access Point Info
String ssid ="SSID";
String password="PASSWORD";

// Api information
String HOST_NAME = "API_URL";
String HOST_PORT = "80";
String token = "API_Token";


// System Variables for Operation
boolean connected = false;

// Atemps to connect to AP
#define CONNECT_ATTEMPTS 10

// Temperature sensor initialization
#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Accelerometer initialization
float ax, ay, az;

// Gas sensor initialization
#define GAS_PIN A3

// Sensor variable initialization
float lastTemp = 0;
float lastHumid = 0;
float lastAccel = 0;
float lastGasQ = 0;
float lastTime=0;

// Timers initialization
Neotimer dhtTimer = Neotimer(500);

// Timers initialization (Set to 2 seconds for testing purposes. Should be changed to 5 minutes.)
Neotimer apiTimer = Neotimer(2000);

// Initial Setup
void setup() {
  Serial.begin(9600);      // USB serial for debugging messages
  Serial1.begin(115200);   // Serial1 (hardware serial) on Arduino 101

  Serial.print("Loading...");
  Serial1.println("AT+RST");
  delay(1000);
  
  // try to establish a connection
  Serial.print("Connecting to Wifi...");
  for (int i = 0; i < CONNECT_ATTEMPTS; i++) {
    Serial.print(".");
    if (wifiConnect()) {
      //Serial.println("");
      connected = true;
      Serial.println("\nConnected to:" + ssid);
      break;
    }
  }

  if(connected == false){
    Serial.println("Unable to Connect :(");
  }

  while(!connected)
  
  // Setup IMU
  Serial.println("Initializing IMU device...");
  CurieIMU.begin();
  CurieIMU.setAccelerometerRange(4);    // Set the accelerometer range to 4G

  // Setup DHT11
  Serial.println("Initializing DHT11 sensor...");
  dht.begin();
  dhtTimer.start();

  apiTimer.start();
}

//*****************
//Main Program Loop
//*****************
void loop() {  
  //Get Time
  lastTime=millis();

  //This is done twice per second
  if(dhtTimer.done()) {
    // Read sensors
    lastHumid = dht.readHumidity();
    lastTemp = dht.readTemperature(true);
    lastGasQ = analogRead(GAS_PIN);
    
    // Timer reset
    dhtTimer.reset();
    dhtTimer.start();
  }
  // Read IMU
  CurieIMU.readAccelerometerScaled(ax, ay, az);
  lastAccel=az;

  if(apiTimer.done()) {
    if(connected) {
      // Send data to NubeIoT
      sendData();
      // Timer reset
      apiTimer.reset();
      apiTimer.start();
    } else {
      Serial.print (lastTemp);
      Serial.print (" ");
      Serial.print (lastHumid);
      Serial.print (" ");
      Serial.print (lastAccel);
      Serial.print (" ");
      Serial.println (lastGasQ);
    }
  }
}

boolean sendData(){
  String cmd = "AT+CIPSTART=\"TCP\",\"" + HOST_NAME +"\",80";
  
  String path_data;
  path_data = "GET /?token=" + token;
  path_data = path_data + "&Temperature=" + lastTemp;
  path_data = path_data + "&Humidity=" + lastHumid;
  path_data = path_data + "&gasQuality=" + lastTemp;
  path_data = path_data + "&Acceleration=" + lastAccel;
  path_data = path_data + " HTTP/1.0\r\nHost: " + HOST_NAME + "\r\nConnection: close\r\n\r\n";
  
  Serial.println("Opening TCP connection to " + HOST_NAME + "...");

  Serial.print("Sending data...  ");
  Serial1.println(cmd);
  delay(500);
  Serial1.print("AT+CIPSEND=");
  Serial1.println(path_data.length());
  delay(1000);
  Serial1.println(path_data);

  Serial1.println("AT+CIPCLOSE");
  Serial.println("Done!");
}

boolean wifiConnect(){
  Serial1.println("AT+CWMODE=1");
  Serial1.println("AT+CWJAP=\"" +ssid+"\",\"" + password + "\"" );
 
  delay(2000);
  if (Serial1.find("OK")) {
  return true;
  } else {
  return false;
  }
}
