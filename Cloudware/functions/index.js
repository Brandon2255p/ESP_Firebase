var functions = require('firebase-functions');
var nJwt = require('njwt');

var priv = fs('firebase_private_key.json');

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
