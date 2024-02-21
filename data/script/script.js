$(document).ready(function () {
	let btnPowerValid = document.getElementById("btnPower");
	let btnHorsGelValid = document.getElementById("btnHorsGel");
	let btnEcoValid = document.getElementById("btnEco");
	let btnConfort2Valid = document.getElementById("btnConfort2");
	let btnConfort1Valid = document.getElementById("btnConfort1");
	let btnConfortValid = document.getElementById("btnConfort");

	$(btnPowerValid).click(function () {
		let valeurEtat = this.getAttribute("etat");
		console.log("Chargement etat: " + valeurEtat);
		switch (valeurEtat) {
			case "activate":
				// this.setAttribute("etat", "non_activate");
				// btnHorsGelValid.setAttribute("etat", "activate");
				// btnEcoValid.setAttribute("etat", "activate");
				// btnConfort2Valid.setAttribute("etat", "activate");
				// btnConfort1Valid.setAttribute("etat", "activate");
				// btnConfortValid.setAttribute("etat", "activate");
				console.log("Désactivation: " + this.getAttribute("etat"));
				break;
			case "non_activate":
				this.setAttribute("etat", "activate");
				console.log("Activation: " + this.getAttribute("etat"));
				btnHorsGelValid.setAttribute("etat", "non_activate");
				btnEcoValid.setAttribute("etat", "non_activate");
				btnConfort2Valid.setAttribute("etat", "non_activate");
				btnConfort1Valid.setAttribute("etat", "non_activate");
				btnConfortValid.setAttribute("etat", "non_activate");
				break;
			default:
				break;
		}
	});
	$(btnHorsGelValid).click(function () {
		let valeurEtat = this.getAttribute("etat");
		console.log("Chargement etat: " + valeurEtat);
		switch (valeurEtat) {
			case "activate":
				// this.setAttribute("etat", "non_activate");
				// btnPowerValid.setAttribute("etat", "activate");
				// btnEcoValid.setAttribute("etat", "activate");
				// btnConfort2Valid.setAttribute("etat", "activate");
				// btnConfort1Valid.setAttribute("etat", "activate");
				// btnConfortValid.setAttribute("etat", "activate");
				console.log("Désactivation: " + this.getAttribute("etat"));
				break;
			case "non_activate":
				this.setAttribute("etat", "activate");
				btnPowerValid.setAttribute("etat", "non_activate");
				btnEcoValid.setAttribute("etat", "non_activate");
				btnConfort2Valid.setAttribute("etat", "non_activate");
				btnConfort1Valid.setAttribute("etat", "non_activate");
				btnConfortValid.setAttribute("etat", "non_activate");
				console.log("Activation: " + this.getAttribute("etat"));
				break;
			default:
				break;
		}
	});
	$(btnEcoValid).click(function () {
		let valeurEtat = this.getAttribute("etat");
		console.log("Chargement etat: " + valeurEtat);
		switch (valeurEtat) {
			case "activate":
				// this.setAttribute("etat", "non_activate");
				// btnPowerValid.setAttribute("etat", "activate");
				// btnHorsGelValid.setAttribute("etat", "activate");
				// btnConfort2Valid.setAttribute("etat", "activate");
				// btnConfort1Valid.setAttribute("etat", "activate");
				// btnConfortValid.setAttribute("etat", "activate");
				console.log("Désactivation: " + this.getAttribute("etat"));
				break;
			case "non_activate":
				this.setAttribute("etat", "activate");
				btnPowerValid.setAttribute("etat", "non_activate");
				btnHorsGelValid.setAttribute("etat", "non_activate");
				btnConfort2Valid.setAttribute("etat", "non_activate");
				btnConfort1Valid.setAttribute("etat", "non_activate");
				btnConfortValid.setAttribute("etat", "non_activate");
				console.log("Activation: " + this.getAttribute("etat"));
				break;
			default:
				break;
		}
	});
	$(btnConfort2Valid).click(function () {
		let valeurEtat = this.getAttribute("etat");
		console.log("Chargement etat: " + valeurEtat);
		switch (valeurEtat) {
			case "activate":
				// this.setAttribute("etat", "non_activate");
				// btnPowerValid.setAttribute("etat", "activate");
				// btnHorsGelValid.setAttribute("etat", "activate");
				// btnEcoValid.setAttribute("etat", "activate");
				// btnConfort1Valid.setAttribute("etat", "activate");
				// btnConfortValid.setAttribute("etat", "activate");
				console.log("Désactivation: " + this.getAttribute("etat"));
				break;
			case "non_activate":
				this.setAttribute("etat", "activate");
				btnPowerValid.setAttribute("etat", "non_activate");
				btnHorsGelValid.setAttribute("etat", "non_activate");
				btnEcoValid.setAttribute("etat", "non_activate");
				btnConfort1Valid.setAttribute("etat", "non_activate");
				btnConfortValid.setAttribute("etat", "non_activate");
				console.log("Activation: " + this.getAttribute("etat"));
				break;
			default:
				break;
		}
	});
	$(btnConfort1Valid).click(function () {
		let valeurEtat = this.getAttribute("etat");
		console.log("Chargement etat: " + valeurEtat);
		switch (valeurEtat) {
			case "activate":
				// this.setAttribute("etat", "non_activate");
				// btnPowerValid.setAttribute("etat", "activate");
				// btnHorsGelValid.setAttribute("etat", "activate");
				// btnEcoValid.setAttribute("etat", "activate");
				// btnConfort2Valid.setAttribute("etat", "activate");
				// btnConfortValid.setAttribute("etat", "activate");
				console.log("Désactivation: " + this.getAttribute("etat"));
				break;
			case "non_activate":
				this.setAttribute("etat", "activate");
				btnPowerValid.setAttribute("etat", "non_activate");
				btnHorsGelValid.setAttribute("etat", "non_activate");
				btnEcoValid.setAttribute("etat", "non_activate");
				btnConfort2Valid.setAttribute("etat", "non_activate");
				btnConfortValid.setAttribute("etat", "non_activate");
				console.log("Activation: " + this.getAttribute("etat"));
				break;
			default:
				break;
		}
	});
	$(btnConfortValid).click(function () {
		let valeurEtat = this.getAttribute("etat");
		console.log("Chargement etat: " + valeurEtat);
		switch (valeurEtat) {
			case "activate":
				// this.setAttribute("etat", "non_activate");
				// btnPowerValid.setAttribute("etat", "activate");
				// btnHorsGelValid.setAttribute("etat", "activate");
				// btnEcoValid.setAttribute("etat", "activate");
				// btnConfort2Valid.setAttribute("etat", "activate");
				// btnConfort1Valid.setAttribute("etat", "activate");
				console.log("Désactivation: " + this.getAttribute("etat"));
				break;
			case "non_activate":
				this.setAttribute("etat", "activate");
				btnPowerValid.setAttribute("etat", "non_activate");
				btnHorsGelValid.setAttribute("etat", "non_activate");
				btnEcoValid.setAttribute("etat", "non_activate");
				btnConfort2Valid.setAttribute("etat", "non_activate");
				btnConfort1Valid.setAttribute("etat", "non_activate");
				console.log("Activation: " + this.getAttribute("etat"));
				break;
			default:
				break;
		}
	});
});
