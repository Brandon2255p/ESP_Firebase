#ifndef  _FIREBASE_
#define _FIREBASE_
#include <WString.h>

class String;
class WiFiClientSecure;

class Firebase
{
private:
  const int httpsPort = 443;
  String jwtUrl;
  String jwtFingerprint;
  String firebaseUrl;
  String firebaseFingerprint;
  const char* urlGoogleApi = "www.googleapis.com";
  const char* googleApiFingerprint = "60 40 DB 92 30 6C C8 BC EB 31 CA CA C8 8D 10 74 30 B1 6A FF";
  String accessToken;

  void ParseAndStoreAccessToken(String const &accessTokenLine);
  bool TokenIsBearer(String const &bearerLine);
  WiFiClientSecure ConnectSecure(String const &url, String const &fingerprint);

public:
  Firebase(char const * JwtHostUrl, char const* JwtFingerprint, char const * UrlFirebase, char const * FirebaseFingerprint);
  ~Firebase();
  void RequestJwt();
  void GetToken();
  void TestDb(String const &getLocation);
  void PutDb(String const &putLocation, String const &putJson);

};


#endif /* end of include guard: _FIREBASE_ */
