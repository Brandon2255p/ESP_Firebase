#include "firebase.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>

Firebase::Firebase(char const * JwtUrl, char const* JwtFingerprint, char const * FirebaseUrl, char const * FirebaseFingerprint){
  jwtUrl = JwtUrl;
  jwtFingerprint = JwtFingerprint;
  firebaseUrl = FirebaseUrl;
  firebaseFingerprint = FirebaseFingerprint;
}

Firebase::~Firebase(){
}

void Firebase::RequestJwt(){
  WiFiClientSecure client = ConnectSecure(jwtUrl, jwtFingerprint);
  if(!client.connected()){
    Serial.print("Not connected :(");
    return;
  }
  String url = "/sendIP";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + jwtUrl.c_str() + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received:");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  Serial.print("connecting to ");
  Serial.println(urlGoogleApi);
  if (!client.connect(urlGoogleApi, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(googleApiFingerprint, urlGoogleApi)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
    return;
  }

  url = "/oauth2/v4/token";
  Serial.print("requesting URL: ");
  Serial.println(url);
  String requestBody = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=" + line + "\r\n\r\n";
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
    if (line.startsWith(" \"access_token\"")) {
      Serial.println("found access token");
      ParseAndStoreAccessToken(line);
      continue;
    } else if (line.startsWith(" \"token_type\"")) {
      Serial.println("found token type");
      if(TokenIsBearer(line)){
        break;
      }
      else{
        return;
      }
    } else {
      Serial.println("nothing found");
    }
    Serial.println(line);
  }
  Serial.println("==========");
  Serial.println("closing connection");
  return;
}

void Firebase::ParseAndStoreAccessToken(String const &accessTokenLine){
  Serial.println(accessTokenLine);
  int beginIndex = accessTokenLine.indexOf("access_token\": \"") + 16;
  int endIndex = accessTokenLine.lastIndexOf("\"");
  accessToken = accessTokenLine.substring(beginIndex, endIndex);
  Serial.println(accessToken);
}

bool Firebase::TokenIsBearer(String const &bearerLine){
  Serial.println(bearerLine);
  bool isBearer = (bearerLine.indexOf("Bearer") > 0);
  return isBearer;
}

void Firebase::TestDb(String const &getLocation) {
  WiFiClientSecure client = ConnectSecure(firebaseUrl, firebaseFingerprint);
  String request = String("GET ") + getLocation + "?access_token=" + accessToken + " HTTP/1.1\r\n" +
               "Host: " + firebaseUrl + "\r\n\r\n";
  Serial.println("====Sending====");
  Serial.println(request);
  Serial.println("======END======");
  client.print(request);
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  while(client.available()){
    String line = client.readStringUntil('\n');
    if(line.startsWith("{")){
      Serial.println("Got res");
      Serial.println(line);
    }
  }
  Serial.println("==========");
  Serial.println("closing connection");
}

void Firebase::PutDb(String const &putLocation, String const &putJson) {
  WiFiClientSecure client = ConnectSecure(firebaseUrl, firebaseFingerprint);

  String request = String("PUT ") + putLocation + "?access_token=" + accessToken + " HTTP/1.1\r\n" +
               "Host: " + firebaseUrl + "\r\n" +
               "Content-Length: " + putJson.length() + "\r\n\r\n" +
               putJson + "\r\n\r\n";
  Serial.println("====Sending====");
  Serial.println(request);
  Serial.println("======END======");
  client.print(request);
  while (client.connected()) {
   String line = client.readStringUntil('\n');
   if (line == "\r") {
     Serial.println("headers received");
     break;
   }
  }
  while(client.available()){
   String line = client.readStringUntil('\n');
   if(line.startsWith("{")){
     Serial.println("Got res");
     Serial.println(line);
   }
  }
  Serial.println("==========");
  Serial.println("closing connection");
}

WiFiClientSecure Firebase::ConnectSecure(String const &url, String const &fingerprint)
{
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(url);
  if (!client.connect(url.c_str(), httpsPort)) {
    Serial.println("connection failed");
    return client;
  }

  if (client.verify(fingerprint.c_str(), url.c_str())) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
  return client;
}
