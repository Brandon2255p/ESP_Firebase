#ifndef  _FIREBASE_
#define _FIREBASE_
#include <WString.h>

class String;
class WiFiClientSecure;

enum FireState {
  Uninit,
  JwtValid,
  TokenValid
};

class Firebase
{
private:
  const int httpsPort = 443;
  String jwtUrl;
  String jwtFingerprint;
  String firebaseUrl;
  String firebaseFingerprint;
  const char* urlGoogleApi = "www.googleapis.com";
  const char* googleApiFingerprint = "86 4E 7F 77 7C B4 31 8F 73 41 3E F9 20 72 CE 84 CE 6A B5 B3";
  String Jwt;
  String accessToken;
  long JwtExpireTime = 0;

  enum FireState state;
  void ParseAndStoreAccessToken(String const &accessTokenLine);
  bool TokenIsBearer(String const &bearerLine);
  bool ConnectSecure(WiFiClientSecure &client, String const &url, String const &fingerprint);

public:
  Firebase(char const * JwtHostUrl, char const* JwtFingerprint, char const * UrlFirebase, char const * FirebaseFingerprint);
  ~Firebase();
  void RequestJwt();
  void GetToken();
  void ReadDb(String const &getLocation);
  void PutDb(String const &putLocation, String const &putJson);

};


#endif /* end of include guard: _FIREBASE_ */
