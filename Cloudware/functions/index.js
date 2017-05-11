var functions = require('firebase-functions');
var nJwt = require('njwt');

var priv = {
  "type": "service_account",
  "project_id": "**REDACTED**",
  "private_key_id": "**REDACTED**",
  "private_key": "-----BEGIN PRIVATE KEY-----**REDACTED**-----END PRIVATE KEY-----\n",
  "client_email": "**REDACTED**.iam.gserviceaccount.com",
  "client_id": "**REDACTED**",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://accounts.google.com/o/oauth2/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/**REDACTED**.iam.gserviceaccount.com"
}

// // Create and Deploy Your First Cloud Functions
// // https://firebase.google.com/docs/functions/write-firebase-functions
//
exports.sendIP = functions.https.onRequest((request, response) => {
  var claims = {
    iss: priv.client_email,  // The URL of your service
    scope: "https://www.googleapis.com/auth/firebase.database https://www.googleapis.com/auth/userinfo.email",
    aud: "https://www.googleapis.com/oauth2/v4/token",
  }
  var expiration = new Date();
  expiration.setSeconds(expiration.getSeconds() + 3600);
  // var jwt = nJwt.create(claims,signingKey);
  var token = new nJwt.Jwt(claims)
        .setSigningAlgorithm('RS256')
        .setExpiration(expiration)
        .setSigningKey(priv.private_key)
        .compact();
  response.send(token);
});
