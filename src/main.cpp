#include <Arduino.h> 
#include <ESP8266WiFi.h> 
 // 1-Wire sensor communication libary 
#include <OneWire.h> 
// DS18B20 sensor library 
#include <DallasTemperature.h> 
const int oneWireBus = D3;      
 
// Setup a oneWire instance to communicate with any OneWire devices 
OneWire oneWire(oneWireBus); 
 
// Pass our oneWire reference to Dallas Temperature sensor  
DallasTemperature DS18B20(&oneWire); 
DeviceAddress deviceAddress;
boolean sensorFound = false;
boolean disMsgShown = false;
// access credentials for WiFi network. 
const char* ssid = "Mohawk-IoT"; 
const char* password = "IoT@MohawK1"; 
long startTime = 0;
// WiFI server.  Listen on port 80, which is the default TCP port for HTTP 
WiFiServer server(80); 
 
 
void setup() 
{ 
  Serial.begin(115200); 
  Serial.println("\nWeb Server Demo");
  startTime = millis();
  if(DS18B20.getAddress(deviceAddress,0)){
    sensorFound = true;
    Serial.print("\nDS18B20 sensor with address: ");

    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(deviceAddress[i],HEX);
    }

    // Start the DS18B20 sensor 
    Serial.println(" \n");
    DS18B20.begin(); 
  }else{
    Serial.println("\nNo DS18B20 sensors found.");
  }
  Serial.printf("\nConnecting to %s ", ssid); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) 
  { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println(" connected"); 
 
  server.begin(); 
  Serial.printf("Web server started, open %s in a web browser\n", 
  WiFi.localIP().toString().c_str()); 
} 
 
 
void loop() 
{
  if(!DS18B20.getAddress(deviceAddress,0)){
    sensorFound = false;
    if(!disMsgShown){
      Serial.println("Sensor Disconnected");
      disMsgShown = true;
    }
  }else{
    sensorFound = true;
    if(disMsgShown){
      Serial.println("Sensor Reconnected");
      disMsgShown = false;
    }
  }
  String statement = "";
  float fTemp;
  WiFiClient client = server.available(); 
  if(sensorFound){
    // ask DS18B20 for the current temperature 
    DS18B20.requestTemperatures(); 
    // fetch the temperature.  We only have 1 sensor, so the index is 0. 
    fTemp = DS18B20.getTempCByIndex(0);
    statement = "Current temperature is: ";
    // wait 5s (5000ms) before doing this again 
    delay(5000);
  }else{
    statement = "Temperature Sensor is unavailable";
  }
  if (client) 
  { 
    Serial.println("\n>> Client connected"); 
    Serial.println(">> Client request:"); 
    while (client.connected()) 
    { 
      // read line by line what the client (web browser) is requesting 
      if (client.available()) 
      { 
        // print information about the request (browser, OS, etc) 
        String line = client.readStringUntil('\r'); 
        Serial.print(line);
        long currentTime = millis();
        long timeAlive = currentTime - startTime;
        long hours = timeAlive / 3600 / 1000;
        long minutes = timeAlive/ 60 / 1000;
        long seconds =timeAlive / 1000;
        // wait for end of client's request 
        if (line.length() == 1 && line[0] == '\n') 
        { 
          client.println("<html><head><link href='https://cdn.jsdelivr.net/npm/bootstrap@5.2.2/dist/css/bootstrap.min.css' rel='stylesheet' integrity='sha384-Zenh87qX5JnK2Jl0vWa8Ck2rdkQ2Bzep5IDxbcnCeuOxjzrPF/et3URy9Bv1WTRi' crossorigin='anonymous'></head><body><div style='height:100%; display: flex;flex-direction:column;justify-content: center;align-items: center;'><div style='text-align:center'><h1>"+statement+"</h1><h2>"+String(fTemp) + " C</h2></div><div style='text-align:center'>" + "<h1>From IP Address:</h1><h2>");
          client.println(client.localIP());
          client.println("</h2></div>");
          client.println("<div style='text-align:center'><h1>Total Time Device is Alive:</h1><h2>"+String(hours)+":"+String(minutes)+":"+String(seconds)+"</h2></div></div><script src='https://cdn.jsdelivr.net/npm/bootstrap@5.2.2/dist/js/bootstrap.bundle.min.js' integrity='sha384-OERcA2EqjJCMA+/3y+gxIOqMEjwtxJY7qPCqsdltbNJuaOe923+mo//f6V8Qbsw3' crossorigin='anonymous'></script></body></html>"); 
          Serial.println(">> Response sent to client"); 
          break; 
        } 
      } 
    } 
 
    // keep read client request data (if any).  After that, we can terminate 
    // our client connection 
    while (client.available()) { 
      client.read(); 
    } 
 
    // close the connection: 
    client.stop(); 
    Serial.println(">> Client disconnected"); 
  } 
}