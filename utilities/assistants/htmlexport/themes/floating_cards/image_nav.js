// keyboard navigation
document.onkeydown = function(e) {
	e = e || window.event;
	link = null;
	switch (e.keyCode) {
		case 37:
			link = document.getElementById("prev");
		break;
		case 39:
			link = document.getElementById("next");
		break;
		case 38:
			link = document.getElementById("up");
		break;
	}
	if (link)
		location.href = link.href;
};

// swipe navigation
document.addEventListener('touchstart', handleTouchStart, false);
document.addEventListener('touchmove', handleTouchMove, false);
document.addEventListener('touchend', handleTouchEnd, false);

var x_start = null;
var y_start = null;

function handleTouchStart(evt) {
	x_start = evt.touches[0].clientX;
	y_start = evt.touches[0].clientY;
};

function handleTouchEnd(evt) {
	x_start = null;
	y_start = null;
};

function handleTouchMove(evt) {
	if (!x_start)
		return;

	var x = x_start - evt.touches[0].clientX;
	var y = y_start - evt.touches[0].clientY;
	var x_abs = Math.abs(x);
	var y_abs = Math.abs(y);
	var horizontal;
	var length;

	if (x_abs < y_abs) {
		horizontal = false;
		length = y_abs;
	}
	else {
		horizontal = true;
		length = x_abs;
	}

	if (horizontal && length > 100) {
		link = null;
		if (x < 0)
			link = document.getElementById("prev");
		else
			link = document.getElementById("next");

		if (link)
			location.href = link.href;
	}
}
