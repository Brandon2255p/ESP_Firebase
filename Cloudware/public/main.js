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
  if (typeof(Storage) !== "undefined") {
      if(localStorage.devices){
        myFunction("Loading defaults");
        var devices = JSON.parse(localStorage.devices);
        console.log("pulling local storage:" + localStorage.devices);
        populateDevices(devices);
      }
      // document.getElementById("snackbar").innerHTML = localStorage.getItem("devices");
  } else {
      document.getElementById("snackbar").innerHTML = "Sorry, your browser does not support Web Storage...";
  }

  return firebase.database().ref('/devices').once('value').then(function(snapshot) {
    var devices = snapshot.val();
    var stringDev = JSON.stringify(devices);
    localStorage.setItem("devices", stringDev);
    myFunction("Sync'd all devices");
    populateDevices(devices);
  });
}

function myFunction(text) {
    // Get the snackbar DIV
    var x = document.getElementById("snackbar")
    x.innerHTML = text;
    // Add the "show" class to DIV
    x.className = "show";

    // After 3 seconds, remove the show class from DIV
    setTimeout(function(){ x.className = x.className.replace("show", ""); }, 3000);
}

function populateDevices(devices){
  for (var i = 0; i < devices.length; i++) {
    if(devices[i] != null){

      console.log("Found a node:: " + devices[i].name);
      console.log("with IP: " + devices[i].ip);
      var cardDom = $("<div id='card"+ i +"'></div>").addClass("card");
      var button  = $("<button></button>").addClass("button").data("url", "http://" + devices[i].ip).data("state",true).text("On");
      var button2 = $("<button></button>").addClass("button").data("url", "http://" + devices[i].ip).data("state",false).text("Off");
      var minutesInput = $("<input></input>").attr("type", "number").val(0).addClass("delay");
      var device = cardDom.append("<h2>"+ devices[i].name + "</h2>", button, button2, "Delay", minutesInput);

      if($("#card" + i).length){
        console.log("Node exists = " + i);
        $("#card" + i).replaceWith(device);
      }
      else{
        console.log("Node does not exist. Creating...");
        $("#devices").append(device);
      }

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
}
