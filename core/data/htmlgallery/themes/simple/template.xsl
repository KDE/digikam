<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-05
 * Description : A Simple theme for the digiKam html gallery tool.
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

<xsl:template name="imagePage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="title"/></title>
		<link rel="stylesheet" type="text/css">
			<xsl:attribute name="href">../simple/<xsl:value-of select="$style"/></xsl:attribute>
		</link>
		<xsl:call-template name="linkTagsImagePage"/>
	</head>
	<body id="imagePage">
	<h1>
		<span id="nav">
			<xsl:choose>
				<xsl:when test="position() &gt; 1">
					<a href="{preceding-sibling::image[position()=1]/full/@fileName}.html">
						« <xsl:value-of select="$i18nPrevious"/>
					</a>
				</xsl:when>
				<xsl:otherwise>
					« <xsl:value-of select="$i18nPrevious"/>
				</xsl:otherwise>
			</xsl:choose>
			|
			<xsl:choose>
				<xsl:when test="position() &lt; last()">
					<a href="{following-sibling::image[position()=1]/full/@fileName}.html">
						<xsl:value-of select="$i18nNext"/> »
					</a>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="$i18nNext"/> »
				</xsl:otherwise>
			</xsl:choose>
		</span>
		<span id="caption">
			<xsl:choose>
				<xsl:when test="count(/collections/collection) &gt; 1">
					<a href="../index.html"><xsl:value-of select="$i18nCollectionList"/></a>
					&raquo;
					<a href="../{../fileName}.html"><xsl:value-of select="../name"/></a>
				</xsl:when>
				<xsl:otherwise>
					<a href="../index.html"><xsl:value-of select="../name"/></a>
				</xsl:otherwise>
			</xsl:choose>

			&raquo; <xsl:value-of select="title"/>
			(<xsl:value-of select="position()"/>/<xsl:value-of select="last()"/>)
		</span>
	</h1>

	<div id="content">
		<div id="image">
			<img src="{full/@fileName}" width="{full/@width}" height="{full/@height}" />
			<p>
			<xsl:value-of select="description"/>
			</p>
			<xsl:if test="original/@fileName != ''">
				<p>
				<a href="{original/@fileName}"><xsl:value-of select="$i18nOriginalImage"/></a>
				(<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)
				</p>
			</xsl:if>
		</div>
	</div>
	</body>
	</html>
</xsl:template>


<xsl:template name="collectionPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="name"/></title>
		<link rel="stylesheet" type="text/css">
			<xsl:attribute name="href">simple/<xsl:value-of select="$style"/></xsl:attribute>
		</link>
		<xsl:call-template name="linkTagsCollectionPage"/>
	</head>
	<body id="collectionPage">
	<h1>
		<xsl:if test="count(/collections/collection) &gt; 1">
			<a href="index.html"><xsl:value-of select="$i18nCollectionList"/></a>
			&raquo;
		</xsl:if>
		<xsl:value-of select="name"/>
	</h1>
	<div id="content">
		<ul>
			<xsl:variable name="folder" select='fileName'/>
			<xsl:for-each select="image">
				<li>
					<a href='{$folder}/{full/@fileName}.html'>
						<img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" />
						<br/>
						<xsl:value-of select="title"/>
					</a>
				</li>
				<exsl:document href='{$folder}/{full/@fileName}.html'>
					<xsl:call-template name="imagePage"/>
				</exsl:document>
			</xsl:for-each>
		</ul>
	</div> <!-- /content -->
	</body>
	</html>
</xsl:template>


<xsl:template name="collectionListPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="$i18nCollectionList"/></title>
		<link rel="stylesheet" type="text/css">
			<xsl:attribute name="href">simple/<xsl:value-of select="$style"/></xsl:attribute>
		</link>
	</head>
	<body id="collectionListPage">
	<h1><xsl:value-of select="$i18nCollectionList"/></h1>
	<div id="content">
		<ul>
			<xsl:for-each select="collections/collection">
				<xsl:sort select="name" order="ascending" data-type="text" />
				<li>
					<a href="{fileName}.html">
						<!-- Use first image as collection image -->
						<img src="{fileName}/{image[1]/thumbnail/@fileName}"
							width="{image[1]/thumbnail/@width}"
							height="{image[1]/thumbnail/@height}" />
						<br />
						<xsl:value-of select="name"/>
					</a>
				</li>
				<exsl:document href="{fileName}.html">
					<xsl:call-template name="collectionPage"/>
				</exsl:document>
			</xsl:for-each>
		</ul>
	</div> <!-- /content -->
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
