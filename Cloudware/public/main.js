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
        var cardDom = $("<div id='card"+ i +"'></div>").addClass("card");
        var button  = $("<button></button>").addClass("button").data("url", "http://" + devices[i].ip).data("state",true).text("On");
        var button2 = $("<button></button>").addClass("button").data("url", "http://" + devices[i].ip).data("state",false).text("Off");
        console.log(button);
        var minutesInput = $("<input></input>").attr("type", "number").val(0).addClass("delay");
        var device = cardDom.append("<h2>"+ devices[i].name + "</h2>", button, button2, "Delay", minutesInput);
        $("#devices").append(device);
      }
    }
    $(".button").click(function(){
      var parent = $(this).closest(".card");
      var url = $(this).data("url");
      var state = $(this).data("state");
      var delay = $(parent).find(".delay").val() * 60;
      console.log("URL: " + url);
      console.log("State: " + state);
      console.log("Delay: " + (delay > 0 ? "&delay=" + delay : "none"));
      relayPage = window.open(url + "?state=" + (state ? "on" : "off") + (delay > 0 ? "&delay=" + delay : ""), "");
    });
  });
}
//
// function CommandDevice(url, command){
//   relayPage = window.open(url + "?state=" + (command ? "on" : "off"), "MsgWindow", "");
// }
