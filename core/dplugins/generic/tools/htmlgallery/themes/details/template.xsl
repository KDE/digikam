<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-06-22
 * Description : A clean look theme for the digiKam html gallery tool.
 *
 * Copyright (C) 2008 by Gianluca Urgese <g dot urgese at jaone dot it>
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

<!DOCTYPE stylesheet [<!ENTITY raquo "&#187;">]>

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
        <link rel="stylesheet" type="text/css" href="../details/style.css"/>
	<xsl:call-template name="linkTagsImagePage"/>
    </head>
    <body id="imagePage">

        <h1>
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
        </h1>


    <div id="content">
	<div class="colsx">
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
	<div class="coldx">
	    <br /><b><xsl:value-of select="$i18nexifimagemake"/>:</b> <xsl:value-of select="exif/exifimagemake"/>
	    <br /><b><xsl:value-of select="$i18nexifimagemodel"/>:</b> <xsl:value-of select="exif/exifimagemodel"/>
	    <br /><b><xsl:value-of select="$i18nexifimageorientation"/>:</b> <xsl:value-of select="exif/exifimageorientation"/>
	    <br /><b><xsl:value-of select="$i18nexifimagexresolution"/>:</b> <xsl:value-of select="exif/exifimagexresolution"/>
	    <br /><b><xsl:value-of select="$i18nexifimageyresolution"/>:</b> <xsl:value-of select="exif/exifimageyresolution"/>
	    <br /><b><xsl:value-of select="$i18nexifimageresolutionunit"/>:</b> <xsl:value-of select="exif/exifimageresolutionunit"/>
	    <br /><b><xsl:value-of select="$i18nexifimagedatetime"/>:</b> <xsl:value-of select="exif/exifimagedatetime"/>
	    <br /><b><xsl:value-of select="$i18nexifimageycbcrpositioning"/>:</b> <xsl:value-of select="exif/exifimageycbcrpositioning"/>
	    <br /><b><xsl:value-of select="$i18nexifphotoexposuretime"/>:</b> <xsl:value-of select="exif/exifphotoexposuretime"/>
	    <br /><b><xsl:value-of select="$i18nexifphotofnumber"/>:</b> <xsl:value-of select="exif/exifphotofnumber"/>
	    <br /><b><xsl:value-of select="$i18nexifphotoexposureprogram"/>:</b> <xsl:value-of select="exif/exifphotoexposureprogram"/>
	    <br /><b><xsl:value-of select="$i18nexifphotoisospeedratings"/>:</b> <xsl:value-of select="exif/exifphotoisospeedratings"/>
	    <br /><b><xsl:value-of select="$i18nexifphotoshutterspeedvalue"/>:</b> <xsl:value-of select="exif/exifphotoshutterspeedvalue"/>
	    <br /><b><xsl:value-of select="$i18nexifphotoaperturevalue"/>:</b> <xsl:value-of select="exif/exifphotoaperturevalue"/>
	    <br /><b><xsl:value-of select="$i18nexifphotofocallength"/>:</b> <xsl:value-of select="exif/exifphotofocallength"/>
	    <br /><b><xsl:value-of select="$i18nexifgpsaltitude"/>:</b> <xsl:value-of select="exif/exifgpsaltitude"/>
	    <br /><b><xsl:value-of select="$i18nexifgpslatitude"/>:</b> <xsl:value-of select="exif/exifgpslatitude"/>
	    <br /><b><xsl:value-of select="$i18nexifgpslongitude"/>:</b> <xsl:value-of select="exif/exifgpslongitude"/>
	</div>
    </div>
    <div id="navigation">
        <div id="previous">
            <xsl:choose>
                <xsl:when test="position() &gt; 1">
                    <a href="{preceding-sibling::image[position()=1]/full/@fileName}.html">
                        <img src="../details/previous.png" alt="{$i18nPrevious}" title="{$i18nPrevious}" />
                    </a>
                </xsl:when>
                <xsl:otherwise>
                    <img src="../details/previous_disabled.png" alt="{$i18nPrevious}" title="{$i18nPrevious}" />
                </xsl:otherwise>
            </xsl:choose>
        </div>
        <div id="up">
                <a href="../index.html">
                        <img src="../details/up.png" alt="{$i18nUp}" title="{$i18nUp}" />
                </a>
        </div>
        <div id="next">
            <xsl:choose>
                <xsl:when test="position() &lt; last()">
                    <a href="{following-sibling::image[position()=1]/full/@fileName}.html">
                        <img src="../details/next.png" alt="{$i18nNext}" title="{$i18nNext}" />
                    </a>
                </xsl:when>
                <xsl:otherwise>
                    <img src="../details/next_disabled.png" alt="{$i18nNext}" title="{$i18nNext}" />
                </xsl:otherwise>
            </xsl:choose>
        </div>
    </div>
    <br /><br />
    <div id="footer">
	  <xsl:value-of select="$footer"/>
    </div>
    </body>
    </html>
</xsl:template>


<xsl:template name="collectionPage">
    <html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <title><xsl:value-of select="name"/></title>
        <link rel="stylesheet" type="text/css" href="details/style.css"/>
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
            <xsl:variable name="folder" select='fileName'/>
            <xsl:for-each select="image">
                <span class="thumbnail">
                    <a href='{$folder}/{full/@fileName}.html'>
                        <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" />
                    </a>
                    <a href='{$folder}/{full/@fileName}.html'>
                        <xsl:value-of select="title"/>
                    </a>
                </span>
                <exsl:document href='{$folder}/{full/@fileName}.html'>
                    <xsl:call-template name="imagePage"/>
                </exsl:document>
            </xsl:for-each>
    </div> <!-- /content -->
    </body>
    </html>
</xsl:template>


<xsl:template name="collectionListPage">
    <html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <title><xsl:value-of select="$i18nCollectionList"/></title>
        <link rel="stylesheet" type="text/css" href="details/style.css"/>
    </head>
    <body>
    <h1><xsl:value-of select="$i18nCollectionList"/></h1>
    <div id="content">
            <xsl:for-each select="collections/collection">
                <xsl:sort select="name" order="ascending" data-type="text" />
                <span class="thumbnail">
                    <a href="{fileName}.html">
                        <!-- Use first image as collection image -->
                        <img src="{fileName}/{image[1]/thumbnail/@fileName}"
                        	width="{image[1]/thumbnail/@width}"
                        	height="{image[1]/thumbnail/@height}" />
                        <xsl:value-of select="name"/>
                    </a>
                </span>
                <exsl:document href="{fileName}.html">
                    <xsl:call-template name="collectionPage"/>
                </exsl:document>
            </xsl:for-each>
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
