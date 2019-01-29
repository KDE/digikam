<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-05
 * Description : A classic theme for the digiKam html gallery tool.
 *
 * Copyright (C) 2007 by Aurélien Gâteau <aurelien dot gateau at free.fr>
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
	<xsl:if test="count(/collections/collection) &gt; 1">
		<link rel="up" href="index.html"></link>
	</xsl:if>
</xsl:template>

<xsl:template name="style">
	<style type='text/css'>
	body {
		color: <xsl:value-of select="$fgColor"/>;
		background: <xsl:value-of select="$bgColor"/>;
		font-family: /*param*/Helvetica, sans-serif;
		font-size: <xsl:value-of select="$fontSize"/>pt;
		margin: 4%;
	}

	h1 {
		color: <xsl:value-of select="$fgColor"/>;
	}

	table {
		text-align: center;
		margin-left: auto;
		margin-right: auto;
	}

	td {
		color: <xsl:value-of select="$fgColor"/>;
		padding: 1em;
	}

	img.photo {
		border: <xsl:value-of select="$imgBorderSize"/>px solid <xsl:value-of select="$imgBorderColor"/>;
	}

	a:link {
		color: <xsl:value-of select="$linkColor"/>;
	}

	a:visited {
		color: <xsl:value-of select="$visitedLinkColor"/>;
	}
	</style>
</xsl:template>

<xsl:template name="imagePage">
	<xsl:param name="index" select="1"/>
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="title"/></title>
		<xsl:call-template name="style"/>
		<xsl:call-template name="linkTagsImagePage"/>
	</head>
	<body>
		<div align="center">
			<xsl:if test="$index &gt; 1">
				<xsl:for-each select="../image[$index - 1]">
					<a href="{full/@fileName}.html">
						<img class="photo" src="{thumbnail/@fileName}" alt="{$i18nPrevious}" title="{$i18nPrevious}" height="{thumbnail/@width}" width="{thumbnail/@height}"/>
					</a>
				</xsl:for-each>
				|
			</xsl:if>

			<xsl:choose>
				<xsl:when test="count(/collections/collection) > 1">
					<a href="../{../fileName}.html"><img src="../classic/up.png" border="0" title="{../description}" alt="{../description}"/></a>
					| <a href="../index.html"><img src="../classic/gohome.png" border="0" alt="{$i18nCollectionList}" title="{$i18nCollectionList}"/></a>
				</xsl:when>
				<xsl:otherwise>
					<a href="../index.html"><img src="../classic/up.png" border="0" title="{$i18nCollectionList}" alt="{$i18nCollectionList}"/></a>
				</xsl:otherwise>
			</xsl:choose>

			<xsl:if test="$index &lt; count(../image)">
				|
				<xsl:for-each select="../image[$index + 1]">
					<a href="{full/@fileName}.html">
						<img class="photo" src="{thumbnail/@fileName}" alt="{$i18nNext}" title="{$i18nNext}" height="{thumbnail/@width}" width="{thumbnail/@height}"/>
					</a>
				</xsl:for-each>
			</xsl:if>
			<br/>
			<hr/>
			<br/>
			<br/>
			<img class="photo" src="{full/@fileName}" width="{full/@width}" height="{full/@height}" alt="{description}" title="{description}"/>
			<xsl:if test="original/@fileName != ''">
				<p>
				<a href="{original/@fileName}"><xsl:value-of select="$i18nOriginalImage"/></a>
				(<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)
				</p>
			</xsl:if>
			<!-- FIXME title="description, imageSize, fileSize" -->
			<br/>
			<br/>
		</div>
		<hr/>
		<!-- FIXME footer -->
	</body>
	</html>
</xsl:template>


<xsl:template name="collectionPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="name"/></title>
		<xsl:call-template name="style"/>
		<xsl:call-template name="linkTagsCollectionPage"/>
	</head>
	<body>
	<xsl:if test="count(/collections/collection) > 1">
		<a href="index.html"><img src="classic/gohome.png" border="0" alt="{$i18nCollectionList}" title="{$i18nCollectionList}"/></a>
	</xsl:if>
	<h1>&quot;<xsl:value-of select="name"/>&quot;</h1>
	<!-- FIXME: Album info -->

	<table>
		<xsl:call-template name="thumbnailTable"/>
	</table>
	<hr/>

	<!-- FIXME: Footer -->
	</body>
	</html>
</xsl:template>


<!-- Called only once per table, use recursion to generate every row -->
<xsl:template name="thumbnailTable">
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
	<xsl:param name="index" select="1"/>
	<xsl:param name="counter" select="1"/>

	<xsl:if test="$index &lt; count(image) + 1">
		<td align="center">
			<xsl:variable name="folder" select='fileName'/>
			<xsl:for-each select="image[$index]">
				<a href='{$folder}/{full/@fileName}.html'>
					<img class="photo" src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" />
					<div><xsl:value-of select="title"/></div>
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


<xsl:template name="collectionListPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="$i18nCollectionList"/></title>
		<xsl:call-template name="style"/>
	</head>
	<body>
	<h1><xsl:value-of select="$i18nCollectionList"/></h1>
	<hr/>
	<p>
	
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
	
	</p>
	<hr/>
	<!-- FIXME: Footer -->
	</body>
	</html>
</xsl:template>


<xsl:template match="/">
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
