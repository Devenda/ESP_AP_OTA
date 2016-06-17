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
    Serial.println(WiFi.softAPIP());
  }
}
