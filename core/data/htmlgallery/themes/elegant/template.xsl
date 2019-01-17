<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-11
 * Description : A LyteBox based theme for the digiKam html gallery tool.
 *
 * Copyright (C) 2007 by Wojciech Jarosz <jiri at boha dot cz>
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
<xsl:variable name="numCollections" select="count(collections/collection)"/>
        
        
<!-- ##################### COLLECTION PAGES GENERATION ##################### -->
<xsl:template name="collectionPages">
  <xsl:call-template name="collectionPages.for.loop">
    <xsl:with-param name="i" select="1"/>
    <xsl:with-param name="count" select="ceiling(count(image) div $pageSize)"/>
  </xsl:call-template>
</xsl:template>

<!-- For loop used to generate collection pages -->
<xsl:template name="collectionPages.for.loop">
  <xsl:param name="i"/>
  <xsl:param name="count"/>
  
  <xsl:if test="$i &lt; $count">
    <xsl:variable name="pageFilename" select="concat(fileName, '_', $i, '.html')"/>
    <exsl:document href="{$pageFilename}"
      method="xml"
      indent="yes" 
      encoding="iso-8859-1" 
      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN">
      <xsl:call-template name="collectionPage">
        <xsl:with-param name="pageNum" select="$i"/>
        <xsl:with-param name="pageFilename" select="$pageFilename"/>
      </xsl:call-template>
    </exsl:document>

    <xsl:call-template name="collectionPages.for.loop">
      <xsl:with-param name="i" select="$i + 1"/>
      <xsl:with-param name="count" select="$count"/>
    </xsl:call-template>
  </xsl:if>
</xsl:template>
<!-- ##################### END COLLECTION PAGE GENERATION ################# -->


<!-- ##################### IMAGE PAGINATION LINK GENERATATION ############# -->
<xsl:template name="image.pagination">
<xsl:param name="indexPage"/>
  <div class="pagination">
    <ul>
      <xsl:choose>
        <xsl:when test="position() &gt; 1">
          <li>
            <a href="{preceding-sibling::image[position()=1]/full/@fileName}.html">
              « <xsl:value-of select="$i18nPrevious"/>
            </a>
          </li>
        </xsl:when>
        <xsl:otherwise>
          <li class="disabled">
            « <xsl:value-of select="$i18nPrevious"/>
          </li>
        </xsl:otherwise>
      </xsl:choose>

      <li>
        <a href="../{$indexPage}"><xsl:value-of select="../name"/></a>
      </li>

      <xsl:choose>
        <xsl:when test="position() &lt; last()">
          <li>
            <a href="{following-sibling::image[position()=1]/full/@fileName}.html">
              <xsl:value-of select="$i18nNext"/> »
            </a>
          </li>
        </xsl:when>
        <xsl:otherwise>
          <li class="disabled">
            <xsl:value-of select="$i18nNext"/> »
          </li>
        </xsl:otherwise>
      </xsl:choose>
    </ul>
  </div>
</xsl:template>


<!-- ##################### PAGINATION LINK GENERATATION ################### -->
<xsl:template name="pagination">
  <xsl:param name="numPages"/>
  <xsl:param name="pageNum"/>
  <xsl:if test="$numPages &gt; 1">
    <div class="pagination">
      <ul>

        <xsl:choose>
          <xsl:when test="number($pageNum) = 0">
            <li class="disabled">« <xsl:value-of select="$i18nPrevious"/></li>
          </xsl:when>
          <xsl:otherwise>
            <li>
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="pageLink">
                    <xsl:with-param name="collectionFilename" select="fileName"/>
                    <xsl:with-param name="pageNum" select="number($pageNum)-1"/>
                  </xsl:call-template>
                </xsl:attribute>
                « <xsl:value-of select="$i18nPrevious"/>
              </a>
            </li>
          </xsl:otherwise>
        </xsl:choose>

        <xsl:call-template name="pagination.for.loop">
          <xsl:with-param name="i" select="0"/>
          <xsl:with-param name="count" select="$numPages"/>
          <xsl:with-param name="currentPage" select="$pageNum"/>
        </xsl:call-template>

        <xsl:choose>
          <xsl:when test="number($pageNum) = number($numPages)-1">
            <li class="disabled"><xsl:value-of select="$i18nNext"/> »</li>
          </xsl:when>
          <xsl:otherwise>
            <li>
              <a>
                <xsl:attribute name="href">
                  <xsl:call-template name="pageLink">
                    <xsl:with-param name="collectionFilename" select="fileName"/>
                    <xsl:with-param name="pageNum" select="number($pageNum)+1"/>
                  </xsl:call-template>
                </xsl:attribute>
                <xsl:value-of select="$i18nNext"/> »
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
  <xsl:param name="i"/>
  <xsl:param name="count"/>
  <xsl:param name="currentPage"/>

  <xsl:if test="$i &lt; $count">
    <xsl:choose>
      <xsl:when test="number($currentPage) = $i">
        <li class="current"><xsl:value-of select="number($i)+1"/></li>
      </xsl:when>
      <xsl:otherwise>
        <li>
          <a>
            <xsl:attribute name="href">
              <xsl:call-template name="pageLink">
                <xsl:with-param name="collectionFilename" select="fileName"/>
                <xsl:with-param name="pageNum" select="$i"/>
              </xsl:call-template>
            </xsl:attribute>
            <xsl:value-of select="number($i)+1"/>
          </a>
        </li>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:call-template name="pagination.for.loop">
      <xsl:with-param name="i" select="$i + 1"/>
      <xsl:with-param name="count" select="$count"/>
      <xsl:with-param name="currentPage" select="$currentPage"/>
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<!-- Template which prints out the html url for a particular page -->
<xsl:template name="pageLink">
  <xsl:param name="collectionFilename"/>
  <xsl:param name="pageNum"/>
  <xsl:choose>
    <xsl:when test="($numCollections &gt; 1) and ($pageNum = 0)">
      <xsl:value-of select="$collectionFilename"/>.html
    </xsl:when>
    <xsl:when test="($numCollections &lt;= 1) and ($pageNum = 0)">
      index.html
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$collectionFilename"/>_<xsl:value-of select="number($pageNum)" />.html
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!-- ##################### END PAGINATION LINK GENERATATION ############### -->



<!-- ##################### CUSTOM CSS STYLESHEET GENERATATION ############# -->
<xsl:template name="customStyle">
  <style type='text/css'>
    body {
      color: <xsl:value-of select="$fgColor"/>;
      background-color: <xsl:value-of select="$bgColor"/>;
    }
    
    a {
        color: <xsl:value-of select="$linkColor"/>;
    }
    
    a:hover {
        color: <xsl:value-of select="$hoverLinkColor"/>;
    }
    
    a:visited {
        color: <xsl:value-of select="$visitedLinkColor"/>;
    }
    
    h1 {
        color: <xsl:value-of select="$fgColor"/>;
        background-color: <xsl:value-of select="$frameColor"/>;
        border-bottom: 1px solid <xsl:value-of select="$frameBorderColor"/>;
    }
    
    .footer {
        color: <xsl:value-of select="$fgColor"/>;
        background-color: <xsl:value-of select="$frameColor"/>;
        border-bottom: 1px solid <xsl:value-of select="$frameBorderColor"/>;
        border-top: 1px solid <xsl:value-of select="$frameBorderColor"/>;
    }
    
    #content li img {
        border: 1px solid <xsl:value-of select="$frameBorderColor"/>;
    }
    
    #content li a {
        background-color: <xsl:value-of select="$frameColor"/>;
        border: 1px solid <xsl:value-of select="$frameBorderColor"/>;
    }
    
    #content li a:hover {
        border: 1px solid <xsl:value-of select="$selectedFrameBorderColor"/>;
        background-color: <xsl:value-of select="$selectedFrameColor"/>;
    }
    
    /* PAGINATION */
    .pagination li a,
    .pagination .disabled {
        background-color: <xsl:value-of select="$frameColor"/>;
        border: 1px solid <xsl:value-of select="$frameBorderColor"/>;
    }
    
    .pagination .current,
    .pagination li a:hover {
        border: 1px solid <xsl:value-of select="$selectedFrameBorderColor"/> !important;
        background-color: <xsl:value-of select="$selectedFrameColor"/> !important;
    }
    
    /* Image page */
    #image {
        border: 1px solid <xsl:value-of select="$frameColor"/>;
        background-color: <xsl:value-of select="$frameBorderColor"/>;
    }
    
    #image img {
        border: 1px solid <xsl:value-of select="$frameBorderColor"/>;
    }
  </style>
</xsl:template>
<!-- ##################### END CUSTOM CSS STYLESHEET GENERATATION ######### -->
    

<!-- ##################### LYTEBOX CONFIG GENERATION ###################### -->
<!--
  Adds some javascript that sets the lytebox parameters and initializes 
  lytebox when the page loads.
-->
<xsl:template name="lyteboxConfig">
<script type="text/javascript">
  function initLytebox() { myLytebox = new LyteBox('<xsl:value-of select="$theme"/>',
                                                   <xsl:value-of select="$outerBorder"/>,
                                                   <xsl:value-of select="$resizeSpeed"/>,
                                                   <xsl:value-of select="$maxOpacity"/>,
                                                   <xsl:value-of select="$navType"/>,
                                                   <xsl:value-of select="$autoResize"/>,
                                                   <xsl:value-of select="$doAnimations"/>,
                                                   <xsl:value-of select="$slideInterval"/>,
                                                   <xsl:value-of select="$showNavigation"/>); }
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
<xsl:param name="pageNum"/>
<xsl:param name="pageFilename"/>
<!-- <xsl:variable name="pageFilename" select="concat(fileName, '_', $pageNum, '.html')"/> -->
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
    <meta name="KEYWORDS" content="photography,software,photos,digital darkroom,gallery,image,photographer"/>
    <meta name="generator" content="DigiKam"/>
    <title><xsl:value-of select="name"/></title>
    <xsl:if test="$useLyteBox = 'true'">
      <script type="text/javascript" src="elegant/resources/js/lytebox.js"/>
      <xsl:call-template name="lyteboxConfig"/>
      <link rel="stylesheet" href="elegant/resources/css/lytebox.css" type="text/css" media="screen"/>
    </xsl:if>
    <link rel="stylesheet" href="elegant/resources/css/master.css" type="text/css" media="screen"/>
    <xsl:choose>
      <xsl:when test="$style = 'custom'">
        <xsl:call-template name="customStyle"/>
      </xsl:when>
      <xsl:otherwise>
        <link rel="stylesheet" type="text/css" media="screen">
          <xsl:attribute name="href">elegant/resources/css/<xsl:value-of select="$style"/></xsl:attribute>
        </link>
      </xsl:otherwise>
    </xsl:choose>
  </head>
  <body>
    <h1>
      <xsl:if test="$numCollections &gt; 1">
        <a href="index.html"><xsl:value-of select="$i18nCollectionList"/></a> »
      </xsl:if>
      <xsl:value-of select="name"/>
    </h1>
    
    <xsl:variable name="numPages" select="ceiling(count(image) div $pageSize)"/>
    <xsl:variable name="folder" select='fileName'/>
    <xsl:variable name="pageName" select="name"/>
    
    <xsl:if test="($paginationLocation = 'top') or ($paginationLocation = 'both')">
      <xsl:call-template name="pagination">
        <xsl:with-param name="numPages" select="$numPages"/>
        <xsl:with-param name="pageNum" select="$pageNum"/>
      </xsl:call-template>
    </xsl:if>
    
    <div id="content">
    
      <!-- Add links to all images before the current page. -->
      <xsl:if test="$useLyteBox = 'true'">
        <xsl:for-each select="image[(position() &lt; ($pageNum * $pageSize) + 1)]">
          <xsl:variable name="imageCaption">
            <xsl:value-of select="description" />
            <xsl:if test="original/@fileName != ''">&lt;p class=&quot;caption&quot;&gt;&lt;a href=&quot;<xsl:value-of select='$folder'/>/<xsl:value-of select="original/@fileName"/>&quot;&gt;<xsl:value-of select="$i18nOriginalImage"/>&lt;/a&gt; (<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)&lt;/p&gt;</xsl:if>
          </xsl:variable>
          <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}"></a>
        </xsl:for-each>
      </xsl:if>
    
      <ul>
        
        <!-- Add thumbnails and links to all images for the current page. -->
        <xsl:for-each select="image[(position() &gt;= ($pageNum * $pageSize) + 1) and (position() &lt;= $pageSize + ($pageSize * $pageNum))]">
          
          <xsl:choose>
            <xsl:when test="$useLyteBox = 'true'">
            
              <xsl:variable name="imageCaption">
                <xsl:value-of select="description"/>
                <xsl:if test="original/@fileName != ''">&lt;p class=&quot;caption&quot;&gt;&lt;a href=&quot;<xsl:value-of select='$folder'/>/<xsl:value-of select="original/@fileName"/>&quot;&gt;<xsl:value-of select="$i18nOriginalImage"/>&lt;/a&gt; (<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)&lt;/p&gt;</xsl:if>
              </xsl:variable>
              <li>
                <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}">
                  <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" alt="{title}"/>
                </a>
              </li>
  
            </xsl:when>
            <xsl:otherwise>
              <li>
                <a href="{$folder}/{full/@fileName}.html">
                  <img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}" />
                </a>
              </li>
              <exsl:document href='{$folder}/{full/@fileName}.html'>
                <xsl:call-template name="imagePage">
                  <xsl:with-param name="indexPage"><xsl:value-of select="$pageFilename"/></xsl:with-param>
                </xsl:call-template>
              </exsl:document>
            </xsl:otherwise>
          </xsl:choose>
            
        </xsl:for-each>
      </ul>
      
      <!-- Add links to all images after the current page. -->
      <xsl:if test="$useLyteBox = 'true'">
        <xsl:for-each select="image[(position() &gt; $pageSize + ($pageSize * $pageNum))]">
          <xsl:variable name="imageCaption">
            <xsl:value-of select="description" />
            <xsl:if test="original/@fileName != ''">&lt;p class=&quot;caption&quot;&gt;&lt;a href=&quot;<xsl:value-of select='$folder'/>/<xsl:value-of select="original/@fileName"/>&quot;&gt;<xsl:value-of select="$i18nOriginalImage"/>&lt;/a&gt; (<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)&lt;/p&gt;</xsl:if>
          </xsl:variable>
          <a href="{$folder}/{full/@fileName}" rel="lyteshow[{$pageName}]" title="{$imageCaption}"></a>
        </xsl:for-each>
      </xsl:if>
      
    </div> <!-- /content -->
    
    <xsl:if test="($paginationLocation = 'bottom') or ($paginationLocation = 'both')">
      <xsl:call-template name="pagination">
        <xsl:with-param name="numPages" select="$numPages"/>
        <xsl:with-param name="pageNum" select="$pageNum"/>
      </xsl:call-template>
    </xsl:if>
    
    <xsl:if test="$author != ''">
      <div class="footer">
        All Images Copyright &#169; <xsl:value-of select="$author"/>
      </div>
    </xsl:if>
  </body>
</html>
<xsl:if test="$pageNum = 0">
  <!-- Generate all subsequent collection pages. -->
  <xsl:call-template name="collectionPages"/>
</xsl:if>
</xsl:template>
<!-- ##################### END COLLECTION PAGE GENERATION ################# -->


<!-- ##################### IMAGE PAGE GENERATION ########################## -->
<!--
  If lytebox is disabled then a webpage is generated to display the large
  version of each image.
-->
<xsl:template name="imagePage">
<xsl:param name="indexPage"/>
  <html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
    <meta name="KEYWORDS" content="photography,software,photos,digital darkroom,gallery,image,photographer"/>
    <meta name="generator" content="DigiKam"/>
    <title><xsl:value-of select="title"/></title>
    <link rel="stylesheet" href="../elegant/resources/css/master.css" type="text/css" media="screen"/>
    <xsl:choose>
      <xsl:when test="$style = 'custom'">
        <xsl:call-template name="customStyle"/>
      </xsl:when>
      <xsl:otherwise>
        <link rel="stylesheet" type="text/css" media="screen">
          <xsl:attribute name="href">../elegant/resources/css/<xsl:value-of select="$style"/></xsl:attribute>
        </link>
      </xsl:otherwise>
    </xsl:choose>
  </head>
  <body>
    <h1>
      <xsl:value-of select="title"/> (<xsl:value-of select="position()"/>/<xsl:value-of select="last()"/>)
    </h1>
  
    <xsl:if test="($paginationLocation = 'top') or ($paginationLocation = 'both')">
      <xsl:call-template name="image.pagination">
        <xsl:with-param name="indexPage" select="$indexPage"/>
      </xsl:call-template>
    </xsl:if>

    <div id="content">
      <div id="image">
        <a href="../{$indexPage}">
          <img src="{full/@fileName}" width="{full/@width}" height="{full/@height}" />
        </a>
        <p style="width: {full/@width};">
        <xsl:value-of select="description"/>
        </p>
        <xsl:if test="original/@fileName != ''">
          <p style="width: {full/@width};">
          <a href="{original/@fileName}"><xsl:value-of select="$i18nOriginalImage"/></a>
          (<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)
          </p>
        </xsl:if>
      </div>
    </div>
  
    <xsl:if test="($paginationLocation = 'bottom') or ($paginationLocation = 'both')">
      <xsl:call-template name="image.pagination">
        <xsl:with-param name="indexPage" select="$indexPage"/>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="$author != ''">
      <div class="footer">
        All Images Copyright &#169; <xsl:value-of select="$author"/>
      </div>
    </xsl:if>
  
  </body>
  </html>
</xsl:template>


<!-- ##################### COLLECTION LIST PAGE GENERATION ################ -->
<!--
  If more than one collection was selected for export then a collectionListPage
  is generated which provides a list of all the individual collections.
-->
<xsl:template name="collectionListPage">
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <meta name="KEYWORDS" content="photography,software,photos,digital darkroom,gallery,image,photographer"/>
    <meta name="generator" content="DigiKam"/>
    <title><xsl:value-of select="$i18nCollectionList"/></title>
    <link rel="stylesheet" href="elegant/resources/css/master.css" type="text/css" media="screen"/>
    <xsl:choose>
      <xsl:when test="$style = 'custom'">
        <xsl:call-template name="customStyle"/>
      </xsl:when>
      <xsl:otherwise>
        <link rel="stylesheet" type="text/css" media="screen">
          <xsl:attribute name="href">elegant/resources/css/<xsl:value-of select="$style"/></xsl:attribute>
        </link>
      </xsl:otherwise>
    </xsl:choose>
  </head>
  <body>
    <h1>
      <xsl:value-of select="$i18nCollectionList"/>
    </h1>
    <div id="content">
      <ul>
        <xsl:for-each select="collections/collection">
          <xsl:variable name="altName" select="name"/>
          <li>
            <a href="{fileName}.html">
              <!-- Use first image as collection image -->
              <img src="{fileName}/{image[1]/thumbnail/@fileName}" width="{image[1]/thumbnail/@width}" height="{image[1]/thumbnail/@height}" alt="{$altName}"/><br /><xsl:value-of select="name"/>
            </a>
          </li>
          <exsl:document href="{fileName}.html"
            method="xml"
            indent="yes" 
            encoding="iso-8859-1" 
            doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
            doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN">
            <xsl:call-template name="collectionPage">
              <xsl:with-param name="pageFilename"><xsl:value-of select="fileName"/>.html</xsl:with-param>
              <xsl:with-param name="pageNum" select="0"/>
            </xsl:call-template>
          </exsl:document>
        </xsl:for-each>
      </ul>
    </div>
    <!-- /content -->
    <xsl:if test="$author != ''">
      <div class="footer">
        All Images Copyright &#169; <xsl:value-of select="$author"/>
      </div>
    </xsl:if>
  </body>
</html>
</xsl:template>
<!-- ##################### END COLLECTION LIST PAGE GENERATION ############ -->


<!-- ##################### STARTING POINT ################################# -->
<!--
  Determines if we need to create a collectionListPage or just one
  collectionStartPage.
-->
<xsl:template match="/">
  <xsl:choose>
    <xsl:when test="$numCollections &gt; 1">
      <xsl:call-template name="collectionListPage"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:for-each select="collections/collection">
        <xsl:call-template name="collectionPage">
          <xsl:with-param name="pageFilename">index.html</xsl:with-param>
          <xsl:with-param name="pageNum" select="0"/>
        </xsl:call-template>
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!-- ##################### END STARTING POINT ############################# -->

</xsl:transform>
