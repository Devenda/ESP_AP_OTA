#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//#include <arduino.h>

//setup OTA
const char* ssidOTA = "telenet-53CC2";
const char* passwordOTA = "MFwkt6cvewxf";

//Setup AP
const char* ssidAP = "TinusESP8266";
const char* passwordAP = "123456789";

WiFiServer server(80);

int previousMillis = millis();
bool state = HIGH;
bool OTAsetupDone = false;
bool APsetupDone = false;

void blinkLED(){
  if (millis() >= previousMillis + 1000) {
    previousMillis = millis();
    digitalWrite(2, state);
    if (state == 0) {
      state = 1;
    }
    else {
      state = 0;
    }
  }
}

//setup as Access Point
void setupAP(){
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssidAP, passwordAP);
  server.begin();
  Serial.println(WiFi.softAPIP());
}

//Connect to network
void connectToAP(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidOTA, passwordOTA);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}

//setup OTA
void setupOTA(){
  // Extra seetings
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  //IO
  pinMode(2, OUTPUT);
  pinMode(14, INPUT);
}

void webServer(){
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/led/0") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/led/1") != -1)
    val = 1; // Will write LED high
  // Otherwise request will be invalid. We'll say as much in HTML

  // Set GPIO5 according to the request
  if (val == 0){
    digitalWrite(2, HIGH);
  }
  if (val == 1) {
    digitalWrite(2, LOW);
  }

  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the LED, print out a message saying we did
  if (val >= 0)
  {
    s += "LED is now ";
    s += (val)?"on":"off";
  }
  else
  {
    s += "Invalid Request.<br> Try /led/1 or /led/0";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

void loop() {
  //OTA setup
  if (digitalRead(14) == 0) {
    if (!OTAsetupDone) {
      Serial.println("OTA Setup");
      connectToAP();
      setupOTA();
      OTAsetupDone = true;
      APsetupDone = false;
    }
    Serial.println("OTA Update");
    Serial.println(WiFi.localIP());
    blinkLED();
    ArduinoOTA.handle();
  }

  //AP setup
  if (digitalRead(14) == 1){
    if (!APsetupDone) {
      digitalWrite(2, HIGH);
      Serial.println("AP Setup");
      setupAP();
      APsetupDone = true;
      OTAsetupDone = false;
    }
    //Serial.println(WiFi.softAPIP());
    webServer();
  }
}
