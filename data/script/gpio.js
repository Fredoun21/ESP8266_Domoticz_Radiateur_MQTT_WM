function onButton() {
	var xhttp = new XMLHttpRequest();
	xhttp.open("GET", "on", true);
	xhttp.send();
}

function offButton() {
	var xhttp = new XMLHttpRequest();
	xhttp.open("GET", "off", true);
	xhttp.send();
}

// function confort1() {
// 	var xhttp = new XMLHttpRequest();
// 	xhttp.open("GET", "SVALUE1=40", true);
// 	xhttp.send();
// }

function updateFilPilote(sValue1) {
	var xhttp = new XMLHttpRequest();
	console.log("updateFilPilote -> sValue1: ", sValue1);
	xhttp.open("GET", "SVALUE1", sValue1, true);
	xhttp.send();
}

// setInterval(function getData() {
// 	var xhttp = new XMLHttpRequest();
// 	xhttp.onreadystatechange = function () {
// 		if (this.readyState == 4 && this.status == 200) {
// 			document.getElementById("valeurTemperature").innerHTML =
// 				this.responseText;
// 		}
// 	};
// 	xhttp.open("GET", "lireTemperature", true);
// 	xhttp.send();
// }, 2000);
