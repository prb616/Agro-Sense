// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
    apiKey: "AIzaSyD1DZito8fL7Kpkrg27rY1TSOtasFm6weg",
    authDomain: "finalproject-8c54c.firebaseapp.com",
    projectId: "finalproject-8c54c",
    storageBucket: "finalproject-8c54c.appspot.com",
    messagingSenderId: "309171790624",
    appId: "1:309171790624:web:4265022df8da8cbe11ed45",
    measurementId: "G-GP7QF271NH",
    databaseURL: "https://finalproject-8c54c-default-rtdb.firebaseio.com/"
  };

firebase.initializeApp(firebaseConfig);

var firebaseRef = firebase.database().ref("sensorData");

function updateData(snapshot) {
    const data = snapshot.val();
    document.getElementById("temperature").textContent = data.temperature || "N/A";
    document.getElementById("humidity").textContent = data.humidity || "N/A";
    document.getElementById("nitrogen").textContent = data.nitrogen || "255";
    document.getElementById("phosphorus").textContent = data.phosphorus || "255";
    document.getElementById("potassium").textContent = data.potassium || "255";
    
}

firebaseRef.on("value", updateData);




