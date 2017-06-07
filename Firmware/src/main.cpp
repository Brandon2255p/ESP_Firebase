#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
#include "user.h"
#include "firebase.h"

const int button1Pin = 2;
const int pinRelay1 = 12;
const int pinRelay2 = 13;

bool bRelay1 = false;
bool bRelay2 = false;

unsigned long ltime = 0;
unsigned long triggerTime = 0;
bool triggerState = false;
int triggerPin = pinRelay1;

void CheckWiFiConnection();

Firebase firebase(JwtFunctionHost, JwtFingerprint, FirebaseUrl, FirebaseFingerprint);
WiFiServer server(80);

void setup() {
  pinMode(button1Pin, INPUT);
  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  digitalWrite(pinRelay1, bRelay1);
  digitalWrite(pinRelay2, bRelay2);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  // ArduinoOTA.setHostname("myesp8266");
  ArduinoOTA.setHostname("extensioncord");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
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
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  server.begin();
  Serial.println("TCP server started");
}

void loop() {
  ltime = millis();
  if(ltime > triggerTime){
    digitalWrite(triggerPin, triggerState);
  }
  CheckWiFiConnection();
  WiFiClient client = server.available();
  ArduinoOTA.handle();
  bool buttonPressed = !digitalRead(button1Pin);
  bool refreshFirebase = (((int)(ltime / 100 / 60)) % 12000 == 0) || (millis() < 20000); // every 20 minutes or first 20s of boot
  if(buttonPressed || refreshFirebase){
    Serial.println("Executing firebase...");
    delay(300);
    firebase.RequestJwt();
    firebase.GetToken();
    // firebase.PutDb("/devices/0.json", "{\"name\":\"Light\",\"ip\":\"" + String(WiFi.localIP().toString()) + "/1\"}");
    // firebase.PutDb("/devices/1.json", "{\"name\":\"None\",\"ip\":\"" + String(WiFi.localIP().toString()) + "/2\"}");
    firebase.PutDb("/devices/0/ip.json", "\"" + String(WiFi.localIP().toString()) + "/1\"");
    firebase.PutDb("/devices/1/ip.json", "\"" + String(WiFi.localIP().toString()) + "/2\"");
  }

  if (!client) {
    return;
  }
  Serial.println("");
  Serial.println("New client");
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');

  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  Serial.print("Request: ");
  Serial.println(req);
  client.flush();

  String s;
  int querySep = req.indexOf('?');
  int relay = req.substring(1, querySep).toInt();
  bool state = req.indexOf("state=on") != -1;
  Serial.println(String("Found relay '") + relay + "' with state '" + state + "'.");
  int delaySep = req.indexOf("delay=");
  bool hasDelay = delaySep != -1;
  if(hasDelay){
    int timeDelay = req.substring(delaySep + 6, req.length()).toInt();
    triggerTime = millis() + (timeDelay * 1000);
    Serial.println("found delay");
    Serial.println(timeDelay);
  }
  bool errorOccured = relay == 0 || relay > 2;
  if(errorOccured){
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
    Serial.println("Sending 404");
  }
  else{
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Command Accepted</html>\r\n\r\n";
    Serial.println("Sending 200");
    int pin = relay == 1 ? pinRelay1 : pinRelay2;
    if(!hasDelay){
        digitalWrite(pin, state);
    }
    triggerState = state;
    triggerPin = pin;
  }
  s += "<script type=\"text/javascript\">window.addEventListener('load',\
  function() {\
    window.close();\
  }, false);</script>\r\n\r\n";
  client.print(s);

  Serial.println("Done with client");
}

void CheckWiFiConnection(){
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}
