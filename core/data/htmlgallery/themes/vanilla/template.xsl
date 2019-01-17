<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-27
 * Description : A based on the default Adobe Lighroom theme
 *               for the digiKam html gallery tool.
 *
 * Copyright (C) 2007 by Wojciech Jarosz <wjarosz at ucsd dot edu>
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

<xsl:transform version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:exsl="http://exslt.org/common" extension-element-prefixes="exsl">
<xsl:output
  method="xml"
  indent="yes" 
  encoding="iso-8859-1" 
  doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
  doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" />


<!-- ##################### VARIABLE INITILIZATION ######################### -->
<!-- Initialize some useful variables -->
<xsl:variable name="theme" select="'grey'" />
<xsl:variable name="outerBorder" select="boolean(0)" />
<xsl:variable name="resizeSpeed" select="10" />
<xsl:variable name="maxOpacity" select="80" />
<xsl:variable name="navType" select="1" />
<xsl:variable name="autoResize" select="boolean(1)" />
<xsl:variable name="doAnimations" select="boolean(1)" />
<xsl:variable name="showNavigation" select="boolean(1)" />
<xsl:variable name="numCollections" select="count(collections/collection)" />
<xsl:variable name="pageSize" select="$numRows * $numCols" />
<xsl:variable name="maxThumbWidth">
  <xsl:for-each select="collections/collection/image/thumbnail">
    <xsl:sort select="number(@width)" data-type="number" order="descending" />
    <xsl:if test="position()=1">
      <xsl:value-of select="number(@width)" />
    </xsl:if>
  </xsl:for-each>
</xsl:variable>
<xsl:variable name="maxThumbHeight">
  <xsl:for-each select="collections/collection/image/thumbnail">
    <xsl:sort select="number(@height)" data-type="number" order="descending" />
    <xsl:if test="position()=1">
      <xsl:value-of select="number(@height)" />
    </xsl:if>
  </xsl:for-each>
</xsl:variable>
<xsl:variable name="maxFullWidth">
  <xsl:for-each select="collections/collection/image/full">
    <xsl:sort select="number(@width)" data-type="number" order="descending" />
    <xsl:if test="position()=1">
      <xsl:value-of select="number(@width)" />
    </xsl:if>
  </xsl:for-each>
</xsl:variable>
<xsl:variable name="maxFullHeight">
  <xsl:for-each select="collections/collection/image/full">
    <xsl:sort select="number(@height)" data-type="number" order="descending" />
    <xsl:if test="position()=1">
      <xsl:value-of select="number(@height)" />
    </xsl:if>
  </xsl:for-each>
</xsl:variable>

        
<!-- ##################### COLLECTION PAGES GENERATION #################### -->
<xsl:template name="collectionPages">
  <xsl:call-template name="collectionPages.for.loop">
    <xsl:with-param name="i" select="1" />
    <xsl:with-param name="count" select="ceiling(count(image) div $pageSize)" />
  </xsl:call-template>
</xsl:template>

<!-- For loop used to generate collection pages -->
<xsl:template name="collectionPages.for.loop">
  <xsl:param name="i" />
  <xsl:param name="count" />
  
  <xsl:if test="$i &lt; $count">
    <xsl:variable name="pageFilename" select="concat(fileName, '_', $i, '.html')" />
    <exsl:document href="{$pageFilename}"
      method="xml"
      indent="yes" 
      encoding="iso-8859-1" 
      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN">
      <xsl:call-template name="collectionPage">
        <xsl:with-param name="pageNum" select="$i" />
        <xsl:with-param name="pageFilename" select="$pageFilename" />
      </xsl:call-template>
    </exsl:document>

    <xsl:call-template name="collectionPages.for.loop">
      <xsl:with-param name="i" select="$i + 1" />
      <xsl:with-param name="count" select="$count" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>
<!-- ##################### END COLLECTION PAGE GENERATION ################# -->


<!-- ##################### IMAGE PAGINATION LINK GENERATATION ############# -->
<xsl:template name="image.pagination">
<xsl:param name="indexPage" />
<div class="detailNav">
  <ul>
    <xsl:variable name="pos" select="1 + count(preceding-sibling::image)" />
    <xsl:variable name="numImages" select="1 + count(preceding-sibling::image) + count(following-sibling::image)" />
    <xsl:choose>
      <xsl:when test="$pos &gt; 1">
        <li class="previous">
          <a href="{preceding-sibling::image[1]/full/@fileName}.html">
            <xsl:value-of select="$i18nPrevious" />
          </a>
        </li>
      </xsl:when>
      <xsl:otherwise>
        <li class="previous">
          <xsl:value-of select="$i18nPrevious" />
        </li>
      </xsl:otherwise>
    </xsl:choose>
        
        
    <li class="index">
      <a href="../{$indexPage}"><xsl:value-of select="../name" /></a>
    </li>
    
    <xsl:choose>
      <xsl:when test="$pos &lt; $numImages">
        <li class="previous">
          <a href="{following-sibling::image[1]/full/@fileName}.html">
            <xsl:value-of select="$i18nNext" />
          </a>
        </li>
      </xsl:when>
      <xsl:otherwise>
        <li class="previous">
          <xsl:value-of select="$i18nNext" />
        </li>
      </xsl:otherwise>
    </xsl:choose>
  </ul>
</div>
</xsl:template>


<!-- ##################### PAGINATION LINK GENERATATION ################### -->
<xsl:template name="pagination">
  <xsl:param name="numPages" />
  <xsl:param name="pageNum" />
  <xsl:if test="$numPages &gt; 1">

    <div class="clear"></div>
    <div class="pagination">
      <ul>
      
        <xsl:call-template name="pagination.for.loop">
          <xsl:with-param name="i" select="0" />
          <xsl:with-param name="count" select="$numPages" />
          <xsl:with-param name="currentPage" select="$pageNum" />
        </xsl:call-template>

        <xsl:choose>
          <xsl:when test="number($pageNum) = 0">
            <li class="previous"><xsl:value-of select="$i18nPrevious" /></li>
          </xsl:when>
          <xsl:otherwise>
            <li class="previous">
              <a class="paginationLinks">
                <xsl:attribute name="href">
                  <xsl:call-template name="pageLink">
                    <xsl:with-param name="collectionFilename" select="fileName" />
                    <xsl:with-param name="pageNum" select="number($pageNum)-1" />
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:value-of select="$i18nPrevious" />
              </a>
            </li>
          </xsl:otherwise>
        </xsl:choose>

        <xsl:choose>
          <xsl:when test="number($pageNum) = number($numPages)-1">
            <li class="next"><xsl:value-of select="$i18nNext" /></li>
          </xsl:when>
          <xsl:otherwise>
            <li class="next">
              <a class="paginationLinks">
                <xsl:attribute name="href">
                  <xsl:call-template name="pageLink">
                    <xsl:with-param name="collectionFilename" select="fileName" />
                    <xsl:with-param name="pageNum" select="number($pageNum)+1" />
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:value-of select="$i18nNext" />
              </a>
            </li>
          </xsl:otherwise>
        </xsl:choose>
      </ul>
    </div>

  </xsl:if>
</xsl:template>

<!-- For loop used to generate pagination links -->
<xsl:template name="pagination.for.loop">
  <xsl:param name="i" />
  <xsl:param name="count" />
  <xsl:param name="currentPage" />
  
  <xsl:if test="$i &lt; $count">
    <xsl:choose>
      <xsl:when test="number($currentPage) = $i">
        <li class="current"><xsl:value-of select="number($i)+1" /></li>
      </xsl:when>
      <xsl:otherwise>
        <li>
          <a>
            <xsl:attribute name="href">
              <xsl:call-template name="pageLink">
                <xsl:with-param name="collectionFilename" select="fileName" />
                <xsl:with-param name="pageNum" select="$i" />
              </xsl:call-template>
            </xsl:attribute>
            <xsl:value-of select="number($i)+1" />
          </a>
        </li>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:call-template name="pagination.for.loop">
      <xsl:with-param name="i" select="$i + 1" />
      <xsl:with-param name="count" select="$count" />
      <xsl:with-param name="currentPage" select="$currentPage" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<!-- Template which prints out the html url for a particular page -->
<xsl:template name="pageLink">
  <xsl:param name="collectionFilename" />
  <xsl:param name="pageNum" />
  <xsl:choose>
    <xsl:when test="($numCollections &gt; 1) and ($pageNum = 0)">
      <xsl:value-of select="$collectionFilename" />.html
    </xsl:when>
    <xsl:when test="($numCollections &lt;= 1) and ($pageNum = 0)">
      index.html
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$collectionFilename" />_<xsl:value-of select="number($pageNum)" />.html
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!-- ##################### END PAGINATION LINK GENERATATION ############### -->


<!-- ##################### SIZING CSS STYLESHEET GENERATATION ############# -->
<xsl:template name="sizingStyle">
  <style type='text/css'>
    #previewFull {
      width: <xsl:value-of select="number(($maxFullWidth + 2*$thumbMargin))" />px !important;
    }
    
    #stage {
      width: <xsl:value-of select="number(($maxThumbWidth + 2*$thumbMargin)*$numCols + $numCols + 1)" />px !important;
    }
    
    .thumbnail {
      width: <xsl:value-of select="number($maxThumbWidth + 2*$thumbMargin)" />px;
      height: <xsl:value-of select="number($maxThumbHeight + 2*$thumbMargin)" />px;
    }
    
    .emptyThumbnail {
      width: <xsl:value-of select="number($maxThumbWidth + 2*$thumbMargin)" />px;
      height: <xsl:value-of select="number($maxThumbHeight + 2*$thumbMargin)" />px;
    }
    
    #stage2 {
      width: <xsl:value-of select="number(($maxFullWidth + 2*$thumbMargin) + 2)" />px !important;
    }
    
    #wrapper {
      width: <xsl:value-of select="number(($maxThumbWidth + 2*$thumbMargin)*$numCols + $numCols + 1)" />px !important;
    }
    
    #wrapper2 {
      width: <xsl:value-of select="number(($maxFullWidth + 2*$thumbMargin) + 2)" />px !important;
    }

    .itemNumber {
      display: <xsl:value-of select="$displayNumbers" /> !important;
    }
  </style>
</xsl:template>
<!-- ##################### END SIZING CSS STYLESHEET GENERATATION ######### -->


<!-- ##################### LYTEBOX CONFIG GENERATION ###################### -->
<!--
  Adds some javascript that sets the lytebox parameters and initializes 
  lytebox when the page loads.
-->
<xsl:template name="lyteboxConfig">
<script type="text/javascript">
  function initLytebox() { myLytebox = new LyteBox('<xsl:value-of select="$theme" />',
                                                   <xsl:value-of select="$outerBorder" />,
                                                   <xsl:value-of select="$resizeSpeed" />,
                                                   <xsl:value-of select="$maxOpacity" />,
                                                   <xsl:value-of select="$navType" />,
                                                   <xsl:value-of select="$autoResize" />,
                                                   <xsl:value-of select="$doAnimations" />,
                                                   <xsl:value-of select="$slideInterval" />,
                                                   <xsl:value-of select="$showNavigation" />); }
  if (window.addEventListener) {
      window.addEventListener("load",initLytebox,false);
  } else if (window.attachEvent) {
      window.attachEvent("onload",initLytebox);
  } else {
      window.onload = function() {initLytebox();}
  }
</script>
</xsl:template>
<!-- ##################### END LYTEBOX CONFIG GENERATION ################## -->


<!-- ##################### COLLECTION PAGE GENERATION ##################### -->
<!--
  The collectionPage is a page of thumbnails for a collection.
  If there are too many thumbnails to fit on one page then the content is split
  up into multiple pages.
-->
<xsl:template name="collectionPage">
<xsl:param name="pageNum" />
<xsl:param name="pageFilename" />
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <meta name="KEYWORDS" content="photography,software,photos,digital darkroom,gallery,image,photographer" />
    <meta name="generator" content="DigiKam" />
    <title><xsl:value-of select="name" /></title>
    <xsl:if test="$useLyteBox = 'true'">
      <script type="text/javascript" src="vanilla/resources/js/lytebox.js" />
      <xsl:call-template name="lyteboxConfig" />
      <link rel="stylesheet" href="vanilla/resources/css/lytebox.css" type="text/css" media="screen" />
    </xsl:if>
    <link rel="stylesheet" type="text/css" media="screen" href="vanilla/resources/css/master.css" />
    <link rel="stylesheet" type="text/css" media="screen">
      <xsl:attribute name="href">vanilla/resources/css/<xsl:value-of select="$style" /></xsl:attribute>
    </link>
    <xsl:call-template name="sizingStyle" />
    <xsl:comment><![CDATA[[if lt IE 7.]> <script defer type="text/javascript" src="vanilla/resources/js/pngfix.js"></script> <![endif]]]></xsl:comment>
    <xsl:comment><![CDATA[[if gt IE 6]> <link rel="stylesheet" href="vanilla/resources/css/ie7.css"></link> <![endif]]]></xsl:comment>
    <xsl:comment><![CDATA[[if lt IE 7.]> <link rel="stylesheet" href="vanilla/resources/css/ie6.css"></link> <![endif]]]></xsl:comment>
  </head>
  <body>
    <div id="wrapper">
      <div id="sitetitle">
        <h1 id="liveUpdateSiteTitle">
          <xsl:if test="$numCollections &gt; 1">
            <a href="index.html"><xsl:value-of select="$i18nCollectionList" /></a> »
          </xsl:if>
          <xsl:value-of select="name" />
        </h1>
      </div>
    
      <xsl:variable name="numPages" select="ceiling(count(image) div $pageSize)" />
      <xsl:variable name="folder" select="fileName"/>
      <xsl:variable name="pageName" select="name" />
      
      <div id="stage">
        <div id="index">
  
          <!-- Add links to all images before the current page. -->
          <xsl:if test="$useLyteBox = 'true'">
            <xsl:for-each select="image[(position() &lt; ($pageNum * $pageSize) + 1)]">
              <xsl:variable name="imageCaption">
                <xsl:value-of select="description" />
                <xsl:if test="original/@fileName != ''">&lt;p&gt;&lt;a href=&quot;<xsl:value-of select='$folder'/>/<xsl:value-of select="original/@fileName" />&quot;&gt;<xsl:value-of select="$i18nOriginalImage" />&lt;/a&gt; (<xsl:value-of select="original/@width" />x<xsl:value-of select="original/@height" />)&lt;/p&gt;</xsl:if>
              </xsl:variable>
              <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}"></a>
            </xsl:for-each>
          </xsl:if>
          
          <!-- Add thumbnails and links to all images for the current page. -->
          <xsl:for-each select="image[(position() &gt;= ($pageNum * $pageSize) + 1) and (position() &lt;= $pageSize + ($pageSize * $pageNum)) and (position() - $pageNum * $pageSize) mod $numCols = 1]">
            <xsl:variable name="numCurrentCols" select="count(.|following-sibling::image[position() &lt; $numCols])" />
            <xsl:variable name="numColsLeft" select="count(.|following-sibling::image)" />
            <xsl:variable name="isLastRow" select="position() mod $numRows = 0 or $numColsLeft &lt;= $numCols" />
            <xsl:for-each select=".|following-sibling::image[position() &lt; $numCols]">
              <div>
                <xsl:attribute name="class">
                  <xsl:choose>
                    <xsl:when test="position() = $numCols and $isLastRow">thumbnail borderTopLeft borderRight borderBottom</xsl:when>
                    <xsl:when test="$isLastRow">thumbnail borderTopLeft borderBottom</xsl:when>
                    <xsl:when test="position() = $numCols">thumbnail borderTopLeft borderRight</xsl:when>
                    <xsl:otherwise>thumbnail borderTopLeft</xsl:otherwise>
                  </xsl:choose>
                </xsl:attribute>
        
                <div class="itemNumber">
                  <xsl:value-of select="1 + count(preceding-sibling::image)" />
                </div>
                
                <xsl:choose>
                  <xsl:when test="$useLyteBox = 'true'">
                    <xsl:variable name="imageCaption">
                      <xsl:value-of select="description" />
                      <xsl:if test="original/@fileName != ''">&lt;p&gt;&lt;a href=&quot;<xsl:value-of select='$folder'/>/<xsl:value-of select="original/@fileName" />&quot;&gt;<xsl:value-of select="$i18nOriginalImage" />&lt;/a&gt; (<xsl:value-of select="original/@width" />x<xsl:value-of select="original/@height" />)&lt;/p&gt;</xsl:if>
                    </xsl:variable>
                    
                    <div>
                    <xsl:attribute name="style">margin-left:<xsl:value-of select="number((($maxThumbWidth + 2*$thumbMargin) - thumbnail/@width) div 2)" />px; margin-top:<xsl:value-of select="number((($maxThumbHeight + 2*$thumbMargin) - thumbnail/@height) div 2)" />px;</xsl:attribute>
                      <xsl:choose>
                        <xsl:when test="$dropShadow = 'true'">
                          <div class="dropShadow">
                            <div class="inner">
                              <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}">
                                <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" alt="{title}" class="thumb" />
                              </a>
                            </div>
                          </div>
                        </xsl:when>
                        <xsl:otherwise>
                          <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}">
                            <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" alt="{title}" class="thumb" />
                          </a>
                        </xsl:otherwise>
                      </xsl:choose>
                    </div>
                  </xsl:when>
                  <xsl:otherwise>
                    
                    <div>
                    <xsl:attribute name="style">margin-left:<xsl:value-of select="number((($maxThumbWidth + 2*$thumbMargin) - thumbnail/@width) div 2)" />px; margin-top:<xsl:value-of select="number((($maxThumbHeight + 2*$thumbMargin) - thumbnail/@height) div 2)" />px;</xsl:attribute>
                      <xsl:choose>
                        <xsl:when test="$dropShadow = 'true'">
                          <div class="dropShadow">
                            <div class="inner">
                              <a href="{$folder}/{full/@fileName}.html">
                                <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" alt="{title}" class="thumb" />
                              </a>
                            </div>
                          </div>
                        </xsl:when>
                        <xsl:otherwise>
                          <a href="{$folder}/{full/@fileName}.html">
                            <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" alt="{title}" class="thumb" />
                          </a>
                        </xsl:otherwise>
                      </xsl:choose>
                    </div>
                    <exsl:document href="{$folder}/{full/@fileName}.html"
                      method="xml"
                      indent="yes" 
                      encoding="iso-8859-1" 
                      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
                      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN">
                      <xsl:call-template name="imagePage">
                        <xsl:with-param name="indexPage"><xsl:value-of select="$pageFilename" /></xsl:with-param>
                      </xsl:call-template>
                    </exsl:document>
                  </xsl:otherwise>
                </xsl:choose>
                
              </div>
              <xsl:if test="position() = $numCols">
                <div class="clear"></div>
              </xsl:if>
              
            </xsl:for-each>

            <xsl:call-template name="emptyCell.for.loop">
              <xsl:with-param name="i" select="1" />
              <xsl:with-param name="count" select="$numCols - $numCurrentCols" />
            </xsl:call-template>

          </xsl:for-each>
          
          <!-- Add links to all images after the current page. -->
          <xsl:if test="$useLyteBox = 'true'">
            <xsl:for-each select="image[(position() &gt; $pageSize + ($pageSize * $pageNum))]">
              <xsl:variable name="imageCaption">
                <xsl:value-of select="description" />
                <xsl:if test="original/@fileName != ''">&lt;p&gt;&lt;a href=&quot;<xsl:value-of select='$folder'/>/<xsl:value-of select="original/@fileName" />&quot;&gt;<xsl:value-of select="$i18nOriginalImage" />&lt;/a&gt; (<xsl:value-of select="original/@width" />x<xsl:value-of select="original/@height" />)&lt;/p&gt;</xsl:if>
              </xsl:variable>
              <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}"></a>
            </xsl:for-each>
          </xsl:if>
          
        </div>
      </div>
      
      <xsl:call-template name="pagination">
        <xsl:with-param name="numPages" select="$numPages" />
        <xsl:with-param name="pageNum" select="$pageNum" />
      </xsl:call-template>
      
      <xsl:if test="$author != ''">
        <div id="contact">
          <a>
            <xsl:attribute name="href">mailto:<xsl:value-of select="$authorEmail" /></xsl:attribute>
            <span><xsl:value-of select="$author" /></span>
          </a>
        </div>
      </xsl:if>
      <div class="clear"></div>
    </div>
  </body>
</html>
<xsl:if test="$pageNum = 0">
  <!-- Generate all subsequent collection pages. -->
  <xsl:call-template name="collectionPages" />
</xsl:if>
</xsl:template>

<!-- For loop used to generate empty thumbnail cells -->
<xsl:template name="emptyCell.for.loop">
  <xsl:param name="i" />
  <xsl:param name="count" />
  
  <xsl:if test="$i &lt;= $count">
    <div>
      <xsl:attribute name="class">
        <xsl:choose>
          <xsl:when test="$i = $count">emptyThumbnail borderTopLeft borderRight borderBottom</xsl:when>
          <xsl:otherwise>emptyThumbnail borderTopLeft borderBottom</xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </div>

    <xsl:call-template name="emptyCell.for.loop">
      <xsl:with-param name="i" select="$i + 1" />
      <xsl:with-param name="count" select="$count" />
    </xsl:call-template>

    <div class="clear"></div>
  </xsl:if>
</xsl:template>
<!-- ##################### END COLLECTION PAGE GENERATION ################# -->


<!-- ##################### IMAGE PAGE GENERATION ########################## -->
<!--
  If lytebox is disabled then a webpage is generated to display the large
  version of each image.
-->
<xsl:template name="imagePage">
<xsl:param name="indexPage" />
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <meta name="KEYWORDS" content="photography,software,photos,digital darkroom,gallery,image,photographer" />
    <meta name="generator" content="DigiKam" />
    <title><xsl:value-of select="title" /></title>
    <link rel="stylesheet" type="text/css" media="screen" href="../vanilla/resources/css/master.css" />
    <link rel="stylesheet" type="text/css" media="screen">
      <xsl:attribute name="href">../vanilla/resources/css/<xsl:value-of select="$style" /></xsl:attribute>
    </link>
    <xsl:call-template name="sizingStyle" />
    <xsl:comment><![CDATA[[if lt IE 7.]> <script defer type="text/javascript" src="../vanilla/resources/js/pngfix.js"></script> <![endif]]]></xsl:comment>
    <xsl:comment><![CDATA[[if gt IE 6]> <link rel="stylesheet" href="../vanilla/resources/css/ie7.css"></link> <![endif]]]></xsl:comment>
    <xsl:comment><![CDATA[[if lt IE 7.]> <link rel="stylesheet" href="../vanilla/resources/css/ie6.css"></link> <![endif]]]></xsl:comment>
  </head>
  <body>
    <div id="wrapper2">
      <div id="sitetitle">
        <h1>
        <xsl:choose>
          <xsl:when test="count(/collections/collection) &gt; 1">
            <a href="../index.html"><xsl:value-of select="$i18nCollectionList" /></a>
            &#187;
            <a href="../{../fileName}.html"><xsl:value-of select="../name" /></a>
          </xsl:when>
          <xsl:otherwise>
            <a href="../index.html"><xsl:value-of select="../name" /></a>
          </xsl:otherwise>
        </xsl:choose>
        </h1>
      </div>
      <div id="collectionHeader">
        <h1>
           <xsl:value-of select="title" /> (<xsl:value-of select="1 + count(preceding-sibling::image)" />/<xsl:value-of select="1 + count(preceding-sibling::image) + count(following-sibling::image)" />)
        </h1>
      </div>
      <div id="stage2">
        <div id="previewFull" class="borderTopLeft borderBottomRight">
        
          <xsl:call-template name="image.pagination">
            <xsl:with-param name="indexPage" select="$indexPage" />
          </xsl:call-template>

          <div>
            <xsl:attribute name="style">margin-left:<xsl:value-of select="number((($maxFullWidth + 2*$thumbMargin) - full/@width) div 2)" />px;</xsl:attribute>
            <xsl:choose>
              <xsl:when test="$dropShadow = 'true'">
                <div class="dropShadow">
                  <div class="inner">
                    <a href="../{$indexPage}">
                      <img src="{full/@fileName}" width="{full/@width}" height="{full/@height}" class="thumb" />
                    </a>
                  </div>
                </div>
              </xsl:when>
              <xsl:otherwise>
                <a href="../{$indexPage}">
                  <img src="{full/@fileName}" width="{full/@width}" height="{full/@height}" class="thumb" />
                </a>
              </xsl:otherwise>
            </xsl:choose>
            <div style="clear:both;">
            </div>
          </div>
          <div id="detailCaption">
            <p><xsl:value-of select="description" /></p>
            <xsl:if test="original/@fileName != ''">
              <p><a href="{original/@fileName}"><xsl:value-of select="$i18nOriginalImage" /></a>
                  (<xsl:value-of select="original/@width" />x<xsl:value-of select="original/@height" />)
              </p>
            </xsl:if>
            <div class="clear"></div>
          </div>
        </div>
      </div>
      <div class="clear"></div>
      <xsl:if test="$author != ''">
        <div id="contact">
          <a>
            <xsl:attribute name="href">mailto:<xsl:value-of select="$authorEmail" /></xsl:attribute>
            <span><xsl:value-of select="$author" /></span>
          </a>
        </div>
      </xsl:if>
      <div class="clear"></div>
    </div>
  </body>
</html>
</xsl:template>
<!-- ##################### END IMAGE PAGE GENERATION ###################### -->


<!-- ##################### COLLECTION LIST PAGE GENERATION ################ -->
<!--
  If more than one collection was selected for export then a collectionListPage
  is generated which provides a list of all the individual collections.
-->
<xsl:template name="collectionListPage">
  <html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <meta name="KEYWORDS" content="photography,software,photos,digital darkroom,gallery,image,photographer" />
    <meta name="generator" content="DigiKam" />
    <title><xsl:value-of select="$i18nCollectionList" /></title>
    <link rel="stylesheet" type="text/css" media="screen" href="vanilla/resources/css/master.css" />
    <link rel="stylesheet" type="text/css" media="screen">
      <xsl:attribute name="href">vanilla/resources/css/<xsl:value-of select="$style" /></xsl:attribute>
    </link>
    <xsl:call-template name="sizingStyle" />
    <xsl:comment><![CDATA[[if lt IE 7.]> <script defer type="text/javascript" src="vanilla/resources/js/pngfix.js"></script> <![endif]]]></xsl:comment>
    <xsl:comment><![CDATA[[if gt IE 6]> <link rel="stylesheet" href="vanilla/resources/css/ie7.css"></link> <![endif]]]></xsl:comment>
    <xsl:comment><![CDATA[[if lt IE 7.]> <link rel="stylesheet" href="vanilla/resources/css/ie6.css"></link> <![endif]]]></xsl:comment>
  </head>
  <body>
    <div id="wrapper">
      <div id="sitetitle">
        <h1 id="liveUpdateSiteTitle">
          <xsl:value-of select="$i18nCollectionList" />
        </h1>
      </div>
      
      <div id="stage">
        <div id="index">
          <xsl:for-each select="collections/collection[position() mod $numCols = 1]">
            <xsl:variable name="numCurrentCols" select="count(.|following-sibling::collection[position() &lt; $numCols])" />
            <xsl:variable name="numColsLeft" select="count(.|following-sibling::collection)" />
            <xsl:variable name="isLastRow" select="position() mod $numRows = 0 or $numColsLeft &lt;= $numCols" />
            <xsl:for-each select=".|following-sibling::collection[position() &lt; $numCols]">
              <div>
                <xsl:attribute name="class">
                  <xsl:choose>
                    <xsl:when test="position() = $numCols and $isLastRow">thumbnail borderTopLeft borderRight borderBottom</xsl:when>
                    <xsl:when test="$isLastRow">thumbnail borderTopLeft borderBottom</xsl:when>
                    <xsl:when test="position() = $numCols">thumbnail borderTopLeft borderRight</xsl:when>
                    <xsl:otherwise>thumbnail borderTopLeft</xsl:otherwise>
                  </xsl:choose>
                </xsl:attribute>
                <xsl:attribute name="style">height:auto;</xsl:attribute>

                <div class="itemNumber">
                  <xsl:value-of select="1 + count(preceding-sibling::collection)" />
                </div>
                
                <xsl:variable name="leftMargin" select="number((($maxThumbWidth + 2*$thumbMargin) - image[1]/thumbnail/@width) div 2)" />
                <xsl:variable name="topMargin" select="number((($maxThumbHeight + 2*$thumbMargin) - image[1]/thumbnail/@height) div 2)" />
                <div>
                  <xsl:attribute name="style">margin-left:<xsl:value-of select="$leftMargin" />px; margin-top:<xsl:value-of select="$topMargin" />px;</xsl:attribute>
                  <xsl:choose>
                    <xsl:when test="$dropShadow = 'true'">
                      <div class="dropShadow">
                        <div class="inner">
                          <a href="{fileName}.html">
                            <!-- Use first image as collection image -->
                            <img src="{fileName}/{image[1]/thumbnail/@fileName}" width="{image[1]/thumbnail/@width}" height="{image[1]/thumbnail/@height}" alt="{name}" class="thumb" />
                          </a>
                        </div>
                      </div>
                    </xsl:when>
                    <xsl:otherwise>
                      <a href="{fileName}.html">
                        <!-- Use first image as collection image -->
                        <img src="{fileName}/{image[1]/thumbnail/@fileName}" width="{image[1]/thumbnail/@width}" height="{image[1]/thumbnail/@height}" alt="{name}" class="thumb" />
                      </a>
                    </xsl:otherwise>
                  </xsl:choose>
                  <div class="clear"></div>
                  <p>
                    <xsl:attribute name="style">text-align:center; padding-top: 1ex; margin-left: <xsl:value-of select="-$leftMargin+5" />px; margin-right:5px; height: 3em;</xsl:attribute>
                    <a href="{fileName}.html"><xsl:value-of select="name" /></a>
                  </p>
                </div>
                <exsl:document href="{fileName}.html"
                  method="xml"
                  indent="yes" 
                  encoding="iso-8859-1" 
                  doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
                  doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN">
                  <xsl:call-template name="collectionPage">
                    <xsl:with-param name="pageFilename"><xsl:value-of select="fileName" />.html</xsl:with-param>
                    <xsl:with-param name="pageNum" select="0" />
                  </xsl:call-template>
                </exsl:document>
                
              </div>
              <xsl:if test="position() = $numCols">
                <div class="clear"></div>
              </xsl:if>
              
            </xsl:for-each>

            <xsl:call-template name="emptyCollectionCell.for.loop">
              <xsl:with-param name="i" select="1" />
              <xsl:with-param name="count" select="$numCols - $numCurrentCols" />
            </xsl:call-template>

          </xsl:for-each>
        </div>
      </div>
      
      <xsl:if test="$author != ''">
        <div id="contact">
          <a>
            <xsl:attribute name="href">mailto:<xsl:value-of select="$authorEmail" /></xsl:attribute>
            <span><xsl:value-of select="$author" /></span>
          </a>
        </div>
      </xsl:if>
      <div class="clear"></div>
    </div>
  </body>
</html>
</xsl:template>

<!-- For loop used to generate empty collection list page cells -->
<xsl:template name="emptyCollectionCell.for.loop">
  <xsl:param name="i" />
  <xsl:param name="count" />
  
  <xsl:if test="$i &lt;= $count">
    <div>
      <xsl:attribute name="class">
        <xsl:choose>
          <xsl:when test="$i = $count">emptyThumbnail borderTopLeft borderRight borderBottom</xsl:when>
          <xsl:otherwise>emptyThumbnail borderTopLeft borderBottom</xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="style">height:auto;</xsl:attribute>
      
      <xsl:variable name="leftMargin" select="number((($maxThumbWidth + 2*$thumbMargin) - image[1]/thumbnail/@width) div 2)" />
      <xsl:variable name="topMargin" select="number((($maxThumbHeight + 2*$thumbMargin) - image[1]/thumbnail/@height) div 2)" />
      <div>
        <xsl:attribute name="style">margin-left:<xsl:value-of select="$leftMargin" />px; margin-top:<xsl:value-of select="$topMargin" />px;</xsl:attribute>
        <div class="thumb">
          <xsl:attribute name="style">border: 0; width:<xsl:value-of select="$maxThumbWidth+2" />px; height:<xsl:value-of select="$maxThumbHeight+2" />px;</xsl:attribute>
        </div>
        <div class="clear"></div>
        <p>
          <xsl:attribute name="style">text-align:center; padding-top: 1ex; margin-left: <xsl:value-of select="-$leftMargin+5" />px; margin-right:5px; height: 3em;</xsl:attribute>
        </p>
      </div>
    </div>

    <xsl:call-template name="emptyCollectionCell.for.loop">
      <xsl:with-param name="i" select="$i + 1" />
      <xsl:with-param name="count" select="$count" />
    </xsl:call-template>

    <div class="clear"></div>
  </xsl:if>
</xsl:template>
<!-- ##################### END COLLECTION LIST PAGE GENERATION ############ -->


<!-- ##################### STARTING POINT ################################# -->
<!--
  Determines if we need to create a collectionListPage or just one
  collectionPage.
-->
<xsl:template match="/">
  <xsl:choose>
    <xsl:when test="$numCollections &gt; 1">
      <xsl:call-template name="collectionListPage" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:for-each select="collections/collection">
        <xsl:call-template name="collectionPage">
          <xsl:with-param name="pageFilename">index.html</xsl:with-param>
          <xsl:with-param name="pageNum" select="0" />
        </xsl:call-template>
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!-- ##################### END STARTING POINT ############################# -->

</xsl:transform>
