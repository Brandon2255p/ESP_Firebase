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
  ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

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
  CheckWiFiConnection();
  WiFiClient client = server.available();
  ArduinoOTA.handle();
  bool buttonPressed = !digitalRead(button1Pin);
  if(buttonPressed){
    Serial.println("Button 1 Pressed");
    delay(200);
    firebase.RequestJwt();
    firebase.GetToken();
    //firebase.ReadDb("/devices/esp2.json");
    firebase.PutDb("/devices/0001.json", "{\"name\":\"Bedside Light\",\"ip\":\"" + String(WiFi.localIP().toString()) + "/1\"}");
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
  if (req == "/1?state=on")
  {
    digitalWrite(pinRelay1, HIGH);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Switching relay 1 on";
    s += "</html>\r\n\r\n";
    Serial.println("Sending 200");
  }
  else if (req == "/1?state=off")
  {
    digitalWrite(pinRelay1, LOW);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Switching relay 1 off";
    s += "</html>\r\n\r\n";
    Serial.println("Sending 200");
  }
  else
  {
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
    Serial.println("Sending 404");
  }
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
