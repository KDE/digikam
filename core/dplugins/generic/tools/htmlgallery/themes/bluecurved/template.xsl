<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-12-11
 * Description : A blue curved theme for the digiKam html gallery tool.
 *
 * Copyright (C) 2013 by Vincent Deroo Blanquart <vincent dot deroo at free dot fr>
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
<!ENTITY raquo "&#187;">
]>

<xsl:transform version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:exsl="http://exslt.org/common"
	extension-element-prefixes="exsl">


<xsl:template name="linkTagsImagePage">
	<!-- Used in the head area of the html page  -->
	<link rel="first" href="{../image[position()=1]/full/@fileName}.html"></link>
	<link rel="last" href="{../image[position()=last()]/full/@fileName}.html"></link>
	<xsl:if test="position() &gt; 1">
		<link rel="prev" href="{preceding-sibling::image[position()=1]/full/@fileName}.html"></link>
	</xsl:if>
	<xsl:if test="position() &lt; last()">
		<link rel="next" href="{following-sibling::image[position()=1]/full/@fileName}.html"></link>
	</xsl:if>
	<xsl:choose>
		<xsl:when test="count(/collections/collection) &gt; 1">
			<link rel="up" href="../{../fileName}.html"></link>
			<link rel="top" href="../index.html"></link>
		</xsl:when>
		<xsl:otherwise>
			<link rel="up" href="../index.html"></link>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template name="linkTagsCollectionPage">
	<!-- Used in the head area of the html page  -->
	<xsl:if test="count(/collections/collection) &gt; 1">
		<link rel="up" href="index.html"></link>
	</xsl:if>
</xsl:template>



<!-- ***********************************************************
 CSS Styles
************************************************************ -->
<xsl:template name="styleIndex">
	<!-- CSS style of the page -->
	<link rel="stylesheet" type="text/css" href="./bluecurved/style.css" />
</xsl:template>


<xsl:template name="stylePicture">
	<!-- CSS style of the page -->
	<link rel="stylesheet" type="text/css" href="../bluecurved/style.css" />
</xsl:template>







<!-- ***********************************************************
 Page with one picture
************************************************************ -->
<xsl:template name="imagePage">
	<xsl:param name="index" select="1"/>
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="title"/></title>
		<xsl:call-template name="stylePicture"/>
		<xsl:call-template name="linkTagsImagePage"/>
	</head>
	<body>

	<div class="rubrique"> 
		<xsl:value-of select="$TitleOfPage"/>
	</div>

	<!-- area with navigation buttons -->
	<div class="gauche">
		<xsl:choose>
			<xsl:when test="count(/collections/collection) > 1">
				<a href="../index.html"><img src="../bluecurved/gohome.png" border="0" alt="{$i18nCollectionList}" title="{$i18nCollectionList}"/></a><br/>
			</xsl:when>
		</xsl:choose>
		<br/>


		<xsl:choose>
			<xsl:when test="count(/collections/collection) > 1">
				<a href="../{../fileName}.html"><img src="../bluecurved/go-top.png" border="0" title="{../description}" alt="{../description}"/></a>
				<br/>
			</xsl:when>
			<xsl:otherwise>
				<a href="../index.html"><img src="../bluecurved/go-top.png" border="0" title="{$i18nCollectionList}" alt="{$i18nCollectionList}"/></a>
				<br/>
			</xsl:otherwise>
		</xsl:choose>


		<xsl:if test="$index = 1">
			 <img src="../bluecurved/vide.png" border="0"  alt="vide" title=""/>
			<br/>
		</xsl:if>


		<xsl:if test="$index &gt; 1">
			<xsl:for-each select="../image[$index - 1]">
				 <a href="{full/@fileName}.html">
					 <img src="../bluecurved/go-previous.png" border="0"  alt="{$i18nPrevious}" title="{$i18nPrevious}"/>
				</a>
				<br/>
			</xsl:for-each>
		</xsl:if>


		<xsl:if test="$index &lt; count(../image)">
			<xsl:for-each select="../image[$index + 1]">
				<a href="{full/@fileName}.html">
					<img src="../bluecurved/go-next.png" alt="{$i18nNext}" title="{$i18nNext}" />
				</a> 
				<br/>
			</xsl:for-each>
		</xsl:if>
	</div>

	<!-- area with picture -->
	<div align="center">
		<br/>
		<br/>

		<img class="photo" src="{full/@fileName}" width="{full/@width}" height="{full/@height}" alt="{description}" title="{description}"/>
		<!-- nom des photos -->
		<div class="titre"><xsl:value-of select="title"/></div>
	
		
		<xsl:if test="original/@fileName != ''">
			<p>
			<a href="{original/@fileName}"> <img src="../bluecurved/document-save.png" border="0"  alt="{$i18nPrevious}" /> </a>
			(<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)
			</p>
		</xsl:if>

		<!-- FIXME title="description, imageSize, fileSize" -->
		<br/>
		</div>

		<!-- FIXME footer -->
	</body>
	</html>
</xsl:template>


<!-- ***********************************************************
 Page with all thumbnails
************************************************************ -->
<xsl:template name="collectionPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="name"/></title>
		<xsl:call-template name="styleIndex"/>
		<xsl:call-template name="linkTagsCollectionPage"/>
	</head>
	<body>

	<div class="rubrique">
		<xsl:value-of select="$TitleOfPage"/>
	</div>



	<div class="gauche">
		<xsl:if test="count(/collections/collection) > 1">
			<a href="index.html"><img src="bluecurved/gohome.png" border="0" alt="{$i18nCollectionList}" title="{$i18nCollectionList}"/></a>
		</xsl:if>
	</div>



	<!-- FIXME: Album info -->

	<table>
		<xsl:call-template name="thumbnailTable"/>
	</table>

	<!-- FIXME: Footer -->
	</body>
	</html>
</xsl:template>





<!-- Called only once per table, use recursion to generate every row -->
<xsl:template name="thumbnailTable">
	<!-- -->
	<xsl:param name="index" select="1"/>

	<xsl:if test="$index &lt; count(image)+1">
		<tr>
			<xsl:call-template name="thumbnailTableCell">
				<xsl:with-param name="index" select="$index"/>
				<xsl:with-param name="counter" select="$index + $thumbnailPerRow - 1"/>
			</xsl:call-template>
		</tr>

		<xsl:call-template name="thumbnailTable">
			<xsl:with-param name="index" select="$index + $thumbnailPerRow"/>
		</xsl:call-template>
	</xsl:if>
</xsl:template>


<!-- Called for every cell, use recursion to generate every cell -->
<xsl:template name="thumbnailTableCell">
	<!-- -->
	<xsl:param name="index" select="1"/>
	<xsl:param name="counter" select="1"/>

	<xsl:if test="$index &lt; count(image) + 1">
		<td align="center">
			<xsl:variable name="folder" select='fileName'/>
			<xsl:for-each select="image[$index]">
				<a href='{$folder}/{full/@fileName}.html'>
					<img class="photo" src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" />

					<!-- nom des photos : mettre une option pour afficher ou non
					<div class="titre"><xsl:value-of select="title"/></div>
					 -->

					<!--FIXME image size and file size -->
				</a>

				<exsl:document href='{$folder}/{full/@fileName}.html'>
					<xsl:call-template name="imagePage">
						<xsl:with-param name="index" select="$index"/>
					</xsl:call-template>
				</exsl:document>
			</xsl:for-each>
		</td>

		<xsl:if test="$counter > $index">
			<xsl:call-template name="thumbnailTableCell">
				<xsl:with-param name="index" select="$index + 1"/>
				<xsl:with-param name="counter" select="$counter"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:if>
</xsl:template>


<!-- ***********************************************************
 Page with all collections
************************************************************ -->
<xsl:template name="collectionListPage">
	<!--  -->
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="$i18nCollectionList"/></title>
		<xsl:call-template name="styleIndex"/>
	</head>
	<body>
	
	<div class="rubrique"><xsl:value-of select="$i18nCollectionList"/></div>

	<div class="gauche">
		 <img src="./bluecurved/vide.png" border="0"  alt="vide" title=""/>
	</div>


	<br/>
	<center>
		<xsl:for-each select="collections/collection">
			<xsl:sort select="name" order="ascending" data-type="text" />
			<xsl:variable name="title" select='concat(name, " [", count(image), "]")'/>
				<a href="{fileName}.html">
					<!-- Use first image as collection image -->
					<img class="photo"
						src="{fileName}/{image[1]/thumbnail/@fileName}"
						width="{image[1]/thumbnail/@width}"
						height="{image[1]/thumbnail/@height}"
						alt="{$title}"
						title="{$title}"/>
				</a>
				<a href="{fileName}.html"><xsl:value-of select="$title"/></a>
				<br />
				<exsl:document href="{fileName}.html">
					<xsl:call-template name="collectionPage"/>
				</exsl:document>
		</xsl:for-each>
	</center>
	
	<!-- FIXME: Footer -->
	</body>
	</html>
</xsl:template>


<xsl:template match="/">
	<!-- -->
	<xsl:choose>
		<xsl:when test="count(collections/collection) &gt; 1">
			<xsl:call-template name="collectionListPage"/>
		</xsl:when>
		<xsl:otherwise>
			<xsl:for-each select="collections/collection">
				<xsl:call-template name="collectionPage"/>
			</xsl:for-each>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>


</xsl:transform>
