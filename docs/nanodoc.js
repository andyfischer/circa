
function hidePackageContents() {
	var divs = document.getElementsByTagName("div");
	for(var i=0; i < divs.length; i++) {
		if(divs[i].className == "package_contents")
			divs[i].style.display = "none";
	}
}

function showPackageContents(package_id) {
	hidePackageContents();
	hideSubs();
	hideSubContents();
	document.getElementById(package_id).style.display = "block";
}

function hideSubs() {
	var divs = document.getElementsByTagName("div");
	for(var i=0; i < divs.length; i++) {
		if(divs[i].className == "modulelist")
			divs[i].style.display = "none";
	}
}

function showModuleContents(moduleid) {
	hideSubs();
	hideSubContents();
	document.getElementById(moduleid).style.display = "block";
}

function hideSubContents() {
	var divs = document.getElementsByTagName("div");
	for(var i=0; i < divs.length; i++) {
		if(divs[i].className == "moduledetail")
			divs[i].style.display = "none";
	}
}

function showSubContents(moduleid) {
	hideSubContents();
	document.getElementById(moduleid).style.display = "block";
}
