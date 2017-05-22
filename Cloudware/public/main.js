// Initialize Firebase
var config = {
  apiKey: "AIzaSyCXc06xgtGMf2HHOg2uX7SiO7N6xwjD93Y",
  authDomain: "homeauto-4075d.firebaseapp.com",
  databaseURL: "https://homeauto-4075d.firebaseio.com",
  projectId: "homeauto-4075d",
  storageBucket: "homeauto-4075d.appspot.com",
  messagingSenderId: "179012906839"
};
firebase.initializeApp(config);

var database = firebase.database();

function LoadStep(){
  return firebase.database().ref('/devices').once('value').then(function(snapshot) {
    var devices = snapshot.val();
    for (var i = 0; i < devices.length; i++) {
      if(devices[i] != null){
        console.log("Found a node:: " + devices[i].name);
        console.log("with IP: " + devices[i].ip);
        var cardDom = $("<div></div>").addClass("card");
        var button = $("<button></button>").addClass("button").attr("onclick",  "CommandDevice('http://" + devices[i].ip + "', true)").text("On");
        var button2 = $("<button></button>").addClass("button").attr("onclick", "CommandDevice('http://" + devices[i].ip + "', false)").text("Off");
        var device = cardDom.append("<h2>"+ devices[i].name + "</h2>", button, button2);
        $("#devices").append(device);
      }
    }
    $("#load").slideUp();
  });
}

function CommandDevice(url, command){
  relayPage = window.open(url + "?state=" + (command ? "on" : "off"), "MsgWindow", "");
}
