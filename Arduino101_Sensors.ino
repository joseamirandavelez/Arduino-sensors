//*****************
//Load Includes
//*****************
#include <DHT.h>
#include <CurieIMU.h>
#include <ELClient.h>
#include <ELClientRest.h>
#include <neotimer.h>

// General variables
const int serialSpeed = 38400;

// ESP-Link client initialization
ELClient esp(&Serial1, &Serial1);
ELClientRest rest(&esp);
boolean wifiConnected = false;
#define BUFLEN 266                                    //Data buffer
#define api_key "eaaeebace112754929d81074576e9e1b"    // NubeIoT API Key

// Temperature sensor initialization
#define DHTPIN 2        // what digital pin we're connected to
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

// Timers initializstion
Neotimer dhtTimer = Neotimer(500);

// Initial Setup
void setup() {
  Serial.begin(serialSpeed);    // USB serial for debugging messages
  Serial1.begin(serialSpeed);   // Hardware serial for communication with ESP8266 (Arduino 101 uses Serial1)

  // Start wifi connection code //
  Serial.println("");
  Serial.println("EL-Client starting!");

  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) {
      Serial.println("");
      Serial.println("EL-Client sync failed! err: ");
      Serial.println(ok);
    }
  } while(!ok);
  Serial.println("EL-Client synced!");

  // Wait for WiFi to be connected.
  esp.GetWifiStatus();
  ELClientPacket *packet;
  Serial.println("Waiting for WiFi ");
  if ((packet=esp.WaitReturn()) != NULL) {
    Serial.print(".");
    Serial.println(packet->value);
  }
  Serial.println("");

  // Set up the REST client to talk to nubeIoT, this doesn't connect to that server,
  // it just sets-up stuff on the esp-link side
  int err = rest.begin("api.nubeiot.com");
  if (err != 0) {
    Serial.print("REST begin failed: ");
    Serial.println(err);
    while(1) ;
  }
  Serial.println("ESP-Link REST ready!");
  // End wifi connection code //
  
  // Setup IMU
  Serial.println("Initializing IMU device...");
  CurieIMU.begin();
  // Set the accelerometer range to 2G
  CurieIMU.setAccelerometerRange(4);

  // Setup DHT11
  Serial.println("Initializing DHT11 sensor...");
  dht.begin();
  dhtTimer.start();
}

// Callback made from esp-link to notify of wifi status changes
// Here we print something out and set a global flag
void wifiCb(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial1.println("WIFI CONNECTED");
      wifiConnected = true;
    } else {
      Serial1.print("WIFI NOT READY: ");
      Serial1.println(status);
      wifiConnected = false;
    }
  }
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
  
    //Serial.print (lastTime);
    //Serial.print (" ");
    Serial.print (lastTemp);
    Serial.print (" ");
    Serial.print (lastHumid);
    Serial.print (" ");
    Serial.print (lastAccel);
    Serial.print (" ");
    Serial.println (lastGasQ);

  //Send data to NubeIoT
  // process any callbacks coming from esp_link
  esp.Process();

  // if we're connected make an REST request
  if(wifiConnected) {
    // Reserve a buffer for sending the data
    char path_data[BUFLEN];
    // Copy the path and API key into the buffer
    sprintf(path_data, "%s", "/?token=");
    sprintf(path_data + strlen(path_data), "%s", api_key);
    
    // Copy the field number and value into the buffer
    // If you have more than one field to update,
    // repeat and change field1 to field2, field3, ...

    sprintf(path_data + strlen(path_data), "%s", "&Temperature=");
    sprintf(path_data + strlen(path_data), "%f", lastTemp);
    sprintf(path_data + strlen(path_data), "%s", "&Humidity=");
    sprintf(path_data + strlen(path_data), "%f", lastHumid);
    sprintf(path_data + strlen(path_data), "%s", "&gasQuality=");
    sprintf(path_data + strlen(path_data), "%f", lastGasQ);
    sprintf(path_data + strlen(path_data), "%s", "&Acceleration=");
    sprintf(path_data + strlen(path_data), "%f", lastAccel);

    
    // Send PUT request to nubeiot.com
    rest.post(path_data,"");

    // Reserve a buffer for the response from Thingspeak
    char response[BUFLEN];
    // Clear the buffer
    memset(response, 0, BUFLEN);
    // Wait for response from Thingspeak
    uint16_t code = rest.waitResponse(response, BUFLEN-1);
    // Check the response from Thingspeak
    if(code == HTTP_STATUS_OK){
      Serial.println("Thingspeak: POST successful:");
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("Thingspeak: POST failed with error ");
      Serial.println(code);
      Serial.print("Response: ");
      Serial.println(response);
    }
    
  }
}
