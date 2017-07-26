//*****************
//Load Includes
//*****************
#include <DHT.h>
#include <CurieIMU.h>
#include <neotimer.h>

// Access Point Info
String ssid ="Miranda_Net_Arris";
String password="cafeconleche";

// Api information
String token = "58156522556c68d0963a3906b7f0c033";
String apiServer ="api.nubeiot.com";

// General variables
const int serialSpeed = 115200;
boolean connected = false;
#define CONNECT_ATTEMPTS 10       // Atemps to connect to AP

// Temperature sensor initialization
#define DHTPIN 3        // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
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

// Timers initializastion
Neotimer dhtTimer = Neotimer(500);

// Timers initializastion
Neotimer apiTimer = Neotimer(60000);

// Initial Setup
void setup() {
  Serial.begin(9600);    // USB serial for debugging messages
  Serial1.begin(serialSpeed);   // Serial1 (hardware serial) on Arduino 101
 
  Serial.print("Loading...");
  Serial1.println("AT+RST");
  delay(1000);
  
  boolean connected = false;
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
  if(connected == true){
    // Count for delay get ip dhcp bla bla bla...
    Serial.print("Adquiring DHCP");
    for(int i=0; i < 5; i ++){
      Serial.print(".");
      delay(1000);
    }
  } else {
    Serial.println("Unable to Connect after 10 tries. Go figure... :(");
  }
  // End wifi connection code //
  
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
    // Send data to NubeIoT
    boolean answer = sendData();
    
    // Timer reset
    apiTimer.reset();
    apiTimer.start();
  }

  //Serial.print (lastTime);
  //Serial.print (" ");
  Serial.print (lastTemp);
  Serial.print (" ");
  Serial.print (lastHumid);
  Serial.print (" ");
  Serial.print (lastAccel);
  Serial.print (" ");
  Serial.println (lastGasQ);
}

boolean wifiConnect(){
  Serial1.println("AT+CWMODE=1");
  Serial1.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"" );
 
  delay(2000);
  if (Serial1.find("OK")) {
  return true;
  } else {
  return false;
  }
}

boolean sendData(){
  String path_data;
  path_data = "GET /?token=" + token;
  path_data = path_data + "&Temperature=" + lastTemp;
  path_data = path_data + "&Humidity=" + lastHumid;
  path_data = path_data + "&gasQuality=" + lastGasQ;
  //path_data = path_data + "&Acceleration=" + lastAccel;
  path_data = path_data + " HTTP/1.0\r\nHOST: api.nubeiot.com" + "\r\nConnection: close\r\n\r\n";
  
  String cmd = "AT+CIPSTART=\"TCP\",\"" + apiServer +"\",80";
  //String getCmd = "GET /?token=" + token + "&test=999 HTTP/1.0\r\nHost: " + apiServer +"\r\nConnection: close\r\n\r\n" ;
  Serial.println(cmd);
  Serial1.println(cmd);
  delay(500);
  Serial.print("AT+CIPSEND=");
  Serial.println( path_data.length() );
  Serial1.print("AT+CIPSEND=");
  Serial1.println( path_data.length() );
 
  delay(1000);
  Serial1.println(path_data);
  Serial.println(path_data);
  Serial1.println("AT+CIPCLOSE");
  Serial.println("AT+CIPCLOSE");
  
  return true;
}
