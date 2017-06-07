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
  state = FireState::Uninit;
}

Firebase::~Firebase(){
}

void Firebase::RequestJwt(){
  bool Expired = (millis() - JwtExpireTime) > (5 * 60 * 1000);
  if(Expired){
    Serial.println("##Expired!");
    state = FireState::Uninit;
  }
  if(state == FireState::TokenValid)
  {
    Serial.println("Token is valid already!");
    return;
  }
  WiFiClientSecure client;
  bool connected = ConnectSecure(client, jwtUrl, jwtFingerprint);

  String url = "/sendIP";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.println(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + jwtUrl.c_str() + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  Jwt = line;
  if(Jwt.length() > 50)
  {
    state = FireState::JwtValid;
  }
}
void Firebase::GetToken() {
  if(state == FireState::TokenValid)
  {
    Serial.println("Token is valid already!");
    return;
  }
  if(state != FireState::JwtValid)
  {
    Serial.println("Jwt is not valid. Can't get token.");
    return;
  }
  WiFiClientSecure client;
  bool connected = ConnectSecure(client, urlGoogleApi, googleApiFingerprint);
  if(!connected)
    return;
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
  JwtExpireTime = millis();
  return;
}

void Firebase::ParseAndStoreAccessToken(String const &accessTokenLine){
  int beginIndex = accessTokenLine.indexOf("access_token\": \"") + 16;
  int endIndex = accessTokenLine.lastIndexOf("\"");
  accessToken = accessTokenLine.substring(beginIndex, endIndex);
  Serial.println(accessToken);
  if(accessToken.length() > 20)
  {
    state = FireState::TokenValid;
  }
}

bool Firebase::TokenIsBearer(String const &bearerLine){
  Serial.println(bearerLine);
  bool isBearer = (bearerLine.indexOf("Bearer") > 0);
  return isBearer;
}

void Firebase::ReadDb(String const &getLocation) {
  if(state != FireState::TokenValid)
  {
    Serial.println("Token is not valid :(");
    return;
  }
  WiFiClientSecure client;
  bool connected = ConnectSecure(client, firebaseUrl, firebaseFingerprint);
  if(!connected)
    return;
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
  if(state != FireState::TokenValid)
  {
    Serial.println("Token is not valid :(");
    return;
  }
  WiFiClientSecure client;
  bool connected = ConnectSecure(client, firebaseUrl, firebaseFingerprint);
  if(!connected)
    return;
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

bool Firebase::ConnectSecure(WiFiClientSecure &client, String const &url, String const &fingerprint)
{
  Serial.print("connecting to ");
  Serial.println(url);
  if (!client.connect(url.c_str(), httpsPort)) {
    Serial.println("connection failed");

    return false;
  }

  if (client.verify(fingerprint.c_str(), url.c_str())) {
    Serial.println("certificate matches");
    return true;
  } else {
    Serial.println("certificate doesn't match");
    return false;
  }
}
