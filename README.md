<H1>Arduino code for Salvation Module</H1>
Developed by the following members of Makerspace Puerto Rico (https://www.facebook.com/search/str/makerspace+pr)
  <ul>
  <li><a href="https://www.facebook.com/josemiranda">José Miranda</a></li>
  <li><a href="https://www.facebook.com/itsvidal">Ramiro Vidal</a></li>
  <li><a href="https://www.facebook.com/alex.o.martinez.7">Alex Martínez</a></li>
  <li><a href="https://www.facebook.com/edwin.g.delgado">Edwin Delgado</a></li>
  <li><a href="https://www.facebook.com/damaso.cardenales">Damaso Cardenales</a></li>
  </ul>
  
  Made for the Intel Hacks 2017 Hackathon. Submitted on 07/27/2017.
  
  <h2>Description</h2>
  This sketch takes readings from a temperature/humidity sensor (DHT11), a gas quality sensor (MQ-2), and the Arduino 101's Accelerometer.
  The data is collected real time and sent over WiFi to a remove web server using an ESP-01 (ESP8266) module. The ESP-01 is used as a
  slave WiFi module to communicate with the server every 2 seconds (for testing purposes). This time interval should be canged to something
  more realistic, as taking data every 2 seconds can generate a lot of data over the day.
  
  <h2>Action Items</h2>
  <ul>
  <li>Add earthquake detection algorithm.</li>
  <li>Add seismic data (time-accelerations) transmittal algorithm (right now it only sends data every two seconds. It will send real-time
   data in packages when an earthquake is detected, in order to provide scientific data).</li>
  <li>Implement a buzzer alarm to tell the user when either smoke, propane, or CO2 is detected, and when an earthquake is detected.</li>
  </ul>
  
  <h2>Note</h2>
  This code is designed to work with the Arduino 101 board, but it should work fine in other boards with a Serial1 interface.
  It can also be easily adapted to work with any Arduino board, albeit with the requirement of using Software Serial in some cases.
