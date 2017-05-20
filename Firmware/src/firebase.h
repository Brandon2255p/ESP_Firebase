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
  const char* googleApiFingerprint = "88 76 BF C5 1E 0D 1C 80 E1 A4 EB B8 00 9A BB 53 E8 FD 86 59";
  String Jwt;
  String accessToken;
  int JwtExpireTime = 0;

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
