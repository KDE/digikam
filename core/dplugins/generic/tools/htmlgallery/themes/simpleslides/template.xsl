<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-10
 * Description : A slideshow theme for the digiKam html gallery tool.
 *
 * Copyright (C) 2018 by Simon Kuss <sjk281 at iinet dot net dot au>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================
 -->

<!DOCTYPE stylesheet>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output 
	method="html" 
	indent="yes"
	encoding="utf-8" />
	
<xsl:template match="/">

<xsl:text disable-output-escaping='yes'>&lt;!DOCTYPE html&gt;</xsl:text>
<html>
<head>
	<meta charset="UTF-8" />
	<title>Slideshow</title>
	<style>
	html {
		background-image: url(<xsl:value-of select=".//fileName"/>/<xsl:value-of select=".//image/full/@fileName" />);
		background-position: center; 
		background-size: cover;
		background-repeat: no-repeat;
		background-attachment: fixed;
		transition: background 3s;
		transform: translate3d(0,0,0);
	}
	#controls {
		position: absolute;
		left: 0;
		right: 0;
		text-align: center;
	<xsl:if test="$controls='bottom'">bottom: 1em;</xsl:if>
	<xsl:if test="$controls='none'">display: none;</xsl:if>
	}
	.control {
		cursor: pointer;
	}
	</style>
</head>
<body>
	<div id="controls"></div>
<script>
var controls = document.getElementById('controls');
var speed = <xsl:value-of select="$delay" />000; 
var currentImg = 0;
var img = [
<xsl:for-each select=".//collection">
	<xsl:variable name="dir" select=".//fileName" />
		<xsl:for-each select="./image">
		"<xsl:value-of select="$dir"/>/<xsl:value-of select="full/@fileName"/>",
		</xsl:for-each>
	</xsl:for-each>
    ];

function stopAtImg(index) {
    return function() {
        clearInterval(timer);
        buttons[currentImg].checked = false;
        currentImg = index;
        switchToImg();
    };
}

function switchToImg() {
    document.documentElement.style.backgroundImage = "url('" + img[currentImg] + "')";
    buttons[currentImg].checked = true;
    buttons[currentImg].focus();
}

if (controls) {
    for (var i = 0; i &lt; img.length; i++ ) {
        var el = document.createElement("input");
        el.type = 'radio';
        el.className = 'control';
        if ( i == 0 ) { el.checked = true; }
        controls.appendChild(el);
    }
    var buttons = controls.querySelectorAll('.control');
    for (var i = 0; i &lt; buttons.length; i++) {
        buttons[i].addEventListener("click", stopAtImg(i));
    }
    buttons[0].focus();
    var timer = setInterval(function() {
        buttons[currentImg].checked = false;
        currentImg++;
        if (currentImg == img.length) { currentImg = 0;}
        switchToImg(currentImg);
    }, speed);
}
</script>
</body>
</html> 

</xsl:template>
</xsl:stylesheet>

