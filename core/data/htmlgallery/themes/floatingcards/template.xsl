<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-30
 * Description : A dark theme with floating thumbnail and description
 *               cards for the digiKam html gallery tool.
 *
 * Copyright (C) 2009 by Jiří Boháč <jiri at boha dot cz>
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

<!DOCTYPE stylesheet [
]>

<xsl:transform version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:exsl="http://exslt.org/common"
	extension-element-prefixes="exsl">
<xsl:output method="xml" indent="yes" 
	doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
        doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" />

<!-- **** image page ******************************************************************* -->
<xsl:template name="imagePage">
<xsl:param name="max_thumb_height" />
<xsl:param name="max_thumb_width" />
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title><xsl:value-of select="title"/></title>
		<link rel="stylesheet" type="text/css" href="../floatingcards/style.css"/>
		<link rel="stylesheet" type="text/css" href="../thumb_size.css"/>
	</head>
	<body class="image">
	<xsl:if test="$keyboard_navigation = 1">
	<script type="text/javascript" src="../floatingcards/image_nav.js"/>
	</xsl:if>
    	
	<div class="fix" id="fix">

	<!-- navigation buttons at the top -->
	<div class="nav right">
		<xsl:if test="position() &lt; last()">
			<a href="{following-sibling::image[position()=1]/full/@fileName}.html" id="next">
				<img src="../floatingcards/next.png" alt="&gt;"/>
			</a>
		</xsl:if>
		<xsl:if test="position() &gt; 1">
			<a href="{preceding-sibling::image[position()=1]/full/@fileName}.html" id="prev">
				<img src="../floatingcards/prev.png" alt="&lt;"/>
			</a>
		</xsl:if>
	</div>
	
	<div class="nav left">
		<xsl:choose>
			<xsl:when test="count(/collections/collection) &gt; 1">
				<a href="../{../fileName}.html" id="up">
				<img src="../floatingcards/up.png" alt="&lt;&lt;&lt;"/></a>
			</xsl:when>
			<xsl:otherwise>
				<a href="../index.html" id="up">
				<img src="../floatingcards/up.png" alt="&lt;&lt;&lt;"/></a>
			</xsl:otherwise>
		</xsl:choose>
	</div>
	
	<!-- the image itself -->
	<div class="image"><div>
	<xsl:choose>
		<xsl:when test="original/@fileName != ''">
			<a href="{original/@fileName}">
			<img src="{full/@fileName}" alt=""/>
			</a>
		</xsl:when>
		<xsl:otherwise>
			<img src="{full/@fileName}" alt=""/>
		</xsl:otherwise>
	</xsl:choose>
	</div></div>

	<h1>
		(<xsl:value-of select="position()"/>/<xsl:value-of select="last()"/>)
		<xsl:value-of select="title"/>
	</h1>
	<h2>
		<xsl:value-of select="description"/>
	</h2>
	</div>
	
	<!-- back/forward selection controls -->
	
	<!-- the maximum expected window width to calculate the maximum number of thumbnails to display -->
	<xsl:variable name="max_screen_width" select="2000"/>

	<!-- the div has a max-width of 80%, margins + padding + border of 38px -->
	<xsl:variable name="select_count" select="floor($max_screen_width * 0.8 div ($max_thumb_width + 38))"/>

	<xsl:variable name="current" select='position()'/>

	<xsl:if test="position() &gt; 1">
		<div class="select back">
		<ul>
			<xsl:for-each select="../image[
					position() &gt;= $current - $select_count 
					and position() &lt; $current]"
			>
			<xsl:sort select="position()" data-type="number" order="descending"/>
				<xsl:call-template name="thumbnail">
					<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
					<xsl:with-param name="pre_description"><xsl:value-of select="$current - position()"/>: </xsl:with-param>
					<xsl:with-param name="img_path"><xsl:value-of select="thumbnail/@fileName"/></xsl:with-param>
					<xsl:with-param name="link_path"><xsl:value-of select="full/@fileName"/>.html</xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
		</ul>
		</div>
	</xsl:if>
	
	<xsl:if test="position() &lt; last()">
		<div class="select forward">
		<ul>
			<xsl:for-each select="../image[
					position() &gt; $current 
					and position() &lt;= $current + $select_count]"
			>
				<xsl:call-template name="thumbnail">
					<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
					<xsl:with-param name="pre_description"><xsl:value-of select="$current + position()"/>: </xsl:with-param>
					<xsl:with-param name="img_path"><xsl:value-of select="thumbnail/@fileName"/></xsl:with-param>
					<xsl:with-param name="link_path"><xsl:value-of select="full/@fileName"/>.html</xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
		</ul>
		</div>
	</xsl:if>
		
		<xsl:if test="$preload = 1">
			<img class="preload" src="{following-sibling::image[position()=1]/full/@fileName}" alt=""/>
		</xsl:if>

	</body>
	</html>
</xsl:template>

<!-- **** collection page *************************************************************** -->
<xsl:template name="collectionPage">
<xsl:param name="max_thumb_height" />
<xsl:param name="max_thumb_width" />
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title><xsl:value-of select="name"/></title>
		<link rel="stylesheet" type="text/css" href="floatingcards/style.css"/>
		<link rel="stylesheet" type="text/css" href="thumb_size.css"/>
	</head>
	<body class="collection">

	<!-- for multi-collection galleries, up is "index.html", otherwise "../" -->
	<xsl:variable name="up">
		<xsl:choose>
		<xsl:when test="count(/collections/collection) &gt; 1">index.html</xsl:when>
		<xsl:otherwise>../</xsl:otherwise>
		</xsl:choose>
	</xsl:variable>
	
	<!-- header and navigation buttons -->
	<div class="nav left">
		<a href="{$up}">
		<img src="floatingcards/up.png" alt="&lt;&lt;&lt;"/></a>
	</div>
	
	<h1><xsl:value-of select="name"/></h1>

	<!-- list of image thumbnails -->
	<ul>
		<xsl:variable name="folder" select='fileName'/>
		<xsl:for-each select="image">
			<xsl:call-template name="thumbnail">
				<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
				<xsl:with-param name="pre_description"><xsl:value-of select="position()"/>: </xsl:with-param>
				<xsl:with-param name="img_path"><xsl:value-of select="$folder"/>/<xsl:value-of select="thumbnail/@fileName"/></xsl:with-param>
				<xsl:with-param name="link_path"><xsl:value-of select="$folder"/>/<xsl:value-of select="full/@fileName"/>.html</xsl:with-param>
			</xsl:call-template>
			<exsl:document href='{$folder}/{full/@fileName}.html'
				method="xml" indent="yes" 
				doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
			 	octype-public="-//W3C//DTD XHTML 1.0 Strict//EN">			
				    	<xsl:call-template name="imagePage">
						<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
						<xsl:with-param name="max_thumb_width"><xsl:value-of select="$max_thumb_width"/></xsl:with-param>
					</xsl:call-template>
			</exsl:document>
		</xsl:for-each>
	</ul>

	<p class="footer">
		<a href="https://www.digikam.org/">digiKam</a> HTML Gallery (Floating Cards theme)
		<a href="http://validator.w3.org/check?uri=referer">Valid XHTML 1.0 Strict!</a>
		<a href="http://jigsaw.w3.org/css-validator/check/referer">Valid CSS!</a>
	</p>
	</body>
	</html>
</xsl:template>


<!-- **** collection list page ********************************************************** -->
<xsl:template name="collectionListPage">
<xsl:param name="max_thumb_height" />
<xsl:param name="max_thumb_width" />
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1.0" />
		<title><xsl:value-of select="$i18nCollectionList"/></title>
		<link rel="stylesheet" type="text/css" href="floatingcards/style.css"/>
		<link rel="stylesheet" type="text/css" href="thumb_size.css"/>
	</head>
	<body class="collectionlist">
	
	<!-- header and navigation buttons -->
	<div class="nav left">
		<a href="../">
		<img src="floatingcards/up.png" alt="&lt;&lt;&lt;"/></a>
	</div>
	<h1>
		<xsl:value-of select="$i18nCollectionList"/>
	</h1>
	
	<!-- list of collection thumbnails -->
	<ul>
		<xsl:for-each select="collections/collection">
			<xsl:sort select="name" order="ascending" data-type="text" />
			<!-- Use first image as collection image -->
			<xsl:for-each select="image[1]">
			<xsl:call-template name="thumbnail">
				<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
				<xsl:with-param name="force_description">
					<xsl:value-of select="../name"/> (<xsl:value-of select="count(../image)"/>)
				</xsl:with-param>
				<xsl:with-param name="img_path"><xsl:value-of select="../fileName"/>/<xsl:value-of select="thumbnail/@fileName"/></xsl:with-param>
				<xsl:with-param name="link_path"><xsl:value-of select="../fileName"/>.html</xsl:with-param>
			</xsl:call-template>
			</xsl:for-each>
			
			<exsl:document href="{fileName}.html"
				method="xml" indent="yes" 
				doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
				octype-public="-//W3C//DTD XHTML 1.0 Strict//EN">			
				<xsl:call-template name="collectionPage">
					<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
					<xsl:with-param name="max_thumb_width"><xsl:value-of select="$max_thumb_width"/></xsl:with-param>
				</xsl:call-template>
			</exsl:document>
		</xsl:for-each>
	</ul>
	
	<p class="footer">
		<a href="https://www.digikam.org/">digiKam</a> HTML Gallery (Floating Cards theme)
		<a href="http://validator.w3.org/check?uri=referer">Valid XHTML 1.0 Strict!</a>
		<a href="http://jigsaw.w3.org/css-validator/check/referer">Valid CSS!</a>
	</p>
	</body>
	</html>
</xsl:template>

<!-- **** thumbnail template **************************************************************** -->
<xsl:template name="thumbnail">
<xsl:param name="max_thumb_height" />
<xsl:param name="pre_description" />
<xsl:param name="force_description" />
<xsl:param name="link_path" />
<xsl:param name="img_path" />

	<!-- the image is automatically centered horizontally inside the card;
	     for vertical centering, calculate the padding -->
	<xsl:variable name="padding_top">
		<xsl:value-of select="floor(($max_thumb_height - thumbnail/@height) div 2)"/>
	</xsl:variable>
	
	<xsl:variable name="padding_bottom">
		<xsl:value-of select="$max_thumb_height - thumbnail/@height - $padding_top"/>
	</xsl:variable>
		
	<li>
		<a href="{$link_path}">
		<!-- don't pollute the HTML with unnecessary zero paddings -->
		<xsl:choose>
			<xsl:when test="$padding_top > 0">
				<img style="margin: {$padding_top}px 0 {$padding_bottom}px 0;" src="{$img_path}" alt=""/>
			</xsl:when>
			<xsl:otherwise>
				<img src="{$img_path}" alt=""/>
			</xsl:otherwise>
		</xsl:choose>
		<br/>
		<xsl:choose>
			<xsl:when test="string($force_description) != ''">
				<xsl:value-of select="$force_description"/>
			</xsl:when>
			<xsl:when test="string(description) != ''">
				<xsl:value-of select="$pre_description"/>
				<xsl:value-of select="description"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$pre_description"/>
				<xsl:value-of select="title"/>
			</xsl:otherwise>
		</xsl:choose>
		</a>
	</li>
</xsl:template>

<!-- **** main template **************************************************************** -->
<xsl:template match="/">

	<!-- determine the maximum thumbnail dimensions;
	     the height needs to be passed to all templates for
             proper padding of thumbnails -->
	<xsl:variable name="max_thumb_width">
		<xsl:for-each select="/collections/collection/image">
			<xsl:sort select="thumbnail/@width" data-type="number" order="descending"/>
			<xsl:if test="position()=1">
				<xsl:value-of select="thumbnail/@width"/>
			</xsl:if>
		</xsl:for-each>
	</xsl:variable>
	<xsl:variable name="max_thumb_height">
		<xsl:for-each select="/collections/collection/image">
			<xsl:sort select="thumbnail/@height" data-type="number" order="descending"/>
			<xsl:if test="position()=1">
				<xsl:value-of select="thumbnail/@height"/>
			</xsl:if>
		</xsl:for-each>
	</xsl:variable>
	

	<!-- create thumb_size.css specifying thumbnail width/height
	     based on the maximum thumbnail dimensions -->
	<exsl:document href='thumb_size.css' method="text">
li {
	width: <xsl:value-of select="$max_thumb_width + 20"/>px;
	height: <xsl:value-of select="$max_thumb_height + 40"/>px;
}

li a {
	min-height: <xsl:value-of select="$max_thumb_height + 21"/>px;
}

div.select {
	min-height: <xsl:value-of select="$max_thumb_height + 72"/>px;
}
	</exsl:document>

	<!-- If there is only one collection in the gallery, make
	     index.html the collection page; otherwise create a
	     collection list page.  -->
	<xsl:choose>
		<xsl:when test="count(collections/collection) &gt; 1">
			<xsl:call-template name="collectionListPage">
				<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
				<xsl:with-param name="max_thumb_width"><xsl:value-of select="$max_thumb_width"/></xsl:with-param>
			</xsl:call-template>

		</xsl:when>
		<xsl:otherwise>
			<xsl:for-each select="collections/collection">
				<xsl:call-template name="collectionPage">
					<xsl:with-param name="max_thumb_height"><xsl:value-of select="$max_thumb_height"/></xsl:with-param>
					<xsl:with-param name="max_thumb_width"><xsl:value-of select="$max_thumb_width"/></xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>


</xsl:transform>
