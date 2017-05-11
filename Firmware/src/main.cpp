#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include <WiFiClientSecure.h>

const char* ssid = "*****";
const char* password = "******";

const char* host = "****.cloudfunctions.net";
const char* urlGoogleApi = "www.googleapis.com";

const int httpsPort = 443;

const int button1Pin = 2;
// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "16 9B 08 24 17 9E 4D 88 CF 0E B0 06 A7 EB 9D 77 5E CD 5C B2";
const char* googleApiFingerprint = "60 40 DB 92 30 6C C8 BC EB 31 CA CA C8 8D 10 74 30 B1 6A FF";
//Prototypes
String GetJwt();
String GetToken(String Jwt);
void CheckWiFiConnection();

void setup() {
  pinMode(button1Pin, INPUT);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
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
}

void loop() {
  CheckWiFiConnection();
  ArduinoOTA.handle();
  delay(100);
  bool buttonPressed = !digitalRead(button1Pin);
  if(buttonPressed){
    Serial.println("Button 1 Pressed");
    delay(200);
    String Jwt = GetJwt();
    String token = GetToken(Jwt);
    Serial.println(token);
  }
}

String GetJwt()
{

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
      return "";
    }

    if (client.verify(fingerprint, host)) {
      Serial.println("certificate matches");
    } else {
      Serial.println("certificate doesn't match");
    }

    String url = "/sendIP";
    Serial.print("requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received:");
        Serial.println("==========");
        Serial.println(line);
        Serial.println("==========");
        break;
      }
    }
    String line = client.readStringUntil('\n');
    // if (line.startsWith("{\"state\":\"success\"")) {
    //   Serial.println("esp8266/Arduino CI successfull!");
    // } else {
    //   Serial.println("esp8266/Arduino CI has failed");
    // }
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    return line;
}

String GetToken(String Jwt)
{
    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(urlGoogleApi);
    if (!client.connect(urlGoogleApi, httpsPort)) {
      Serial.println("connection failed");
      return "";
    }

    if (client.verify(googleApiFingerprint, urlGoogleApi)) {
      Serial.println("certificate matches");
    } else {
      Serial.println("certificate doesn't match");
    }

    String url = "/oauth2/v4/token";
    Serial.print("requesting URL: ");
    Serial.println(url);
    String requestBody = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=" + Jwt + "\r\n\r\n";
    String request = String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + urlGoogleApi + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Content-Length: " + requestBody.length() + "\r\n\r\n" + requestBody;
    Serial.println(request);
    client.print(request);

    Serial.println("request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received:");
        Serial.println("==========");
        Serial.println(line);
        break;
        Serial.println("==========");
      }
    }
    Serial.println("reply was:");
    Serial.println("==========");
    while(client.available()){
      String line = client.readStringUntil('\n');
      // if (line.startsWith("{\"state\":\"success\"")) {
      //   Serial.println("esp8266/Arduino CI successfull!");
      // } else {
      //   Serial.println("esp8266/Arduino CI has failed");
      // }
      Serial.println(line);
    }
    Serial.println("==========");
    Serial.println("closing connection");
    return "No reply";
}

void CheckWiFiConnection(){
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}
