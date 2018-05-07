<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE xsl:stylesheet [<!ENTITY nbsp "&#160;">]>
<xsl:stylesheet version="2.0"
  xmlns:ebn="http://ebn.kde.org/krazy"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns="http://www.w3.org/1999/xhtml">

  <xsl:import href="functions.xsl" />

  <xsl:param name="component" as="xsd:string"/>
  <xsl:param name="module" as="xsd:string"/>
  <xsl:param name="submodule" as="xsd:string" />

  <xsl:output doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
              doctype-system="DTD/xhtml1-transitional.dtd"
              encoding="UTF-8"
              indent="yes"
              method="xhtml"
              omit-xml-declaration="yes"
              version="1.0" />

  <xsl:function name="ebn:createLexerLink" as="xsd:string">
    <xsl:param name="file" as="xsd:string"/>
    <xsl:param name="line" as="xsd:integer" />
    <xsl:param name="maxlines" as="xsd:integer" />

    <xsl:variable name="lexerComponent">
      <xsl:choose>
        <xsl:when test="matches($component, '^kde-\d\.x$')">
          <xsl:value-of select="'kde'"/>
        </xsl:when>
        <xsl:when test="$component eq $module" >
          <!-- Just leave out the component if both component and are the same -->
          <xsl:value-of select="''" />
        </xsl:when>
        <xsl:when test="matches($component, 'bundled-apps')">
          <xsl:value-of select="'calligra'"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$component" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="lexerModule">
      <xsl:choose>
        <xsl:when test="matches($module, 'kdebase-apps')">
          <xsl:value-of select="'kdebase/apps'"/>
        </xsl:when>
        <xsl:when test="matches($module, 'kdebase-runtime')">
          <xsl:value-of select="'kdebase/runtime'"/>
        </xsl:when>
        <xsl:when test="matches($module, 'kdebase-workspace')">
          <xsl:value-of select="'kdebase/workspace'"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$module" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="componentModule">
      <xsl:choose>
        <xsl:when test="$lexerComponent eq ''" >
          <xsl:value-of select="$module" />
        </xsl:when>
        <xsl:when test="matches($module, 'frameworks5')">
          <xsl:value-of select="$lexerComponent" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat($lexerComponent, '/', $lexerModule)" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$maxlines eq -1" >
        <xsl:value-of select="concat('http://lxr.kde.org/source/', $componentModule, '/', $submodule, '/', $file)" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="fline" select="ebn:formatLineNumber($line, $maxlines)" />
        <xsl:value-of
          select="concat('http://lxr.kde.org/source/', $componentModule, '/', $submodule, '/', $file, '#', $fline)" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:function>

  <xsl:template name="file">
    <xsl:variable name="lxrlink" select="ebn:createLexerLink(@name, @lines, -1)" />
    <xsl:variable name="filename" select="@name" />
    <xsl:variable name="maxlines" select="@lines" />
    <xsl:variable name="count" select="count(issues/line)" />

    <li><a href="{$lxrlink}"><xsl:value-of select="@name"/></a>:
      <xsl:choose>
        <xsl:when test="issues/line[1] eq '-1'">
          <xsl:value-of select="message" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="message" /> line#
          <xsl:for-each select="issues/line">
            <xsl:variable name="lxrlinelink" select="ebn:createLexerLink($filename, .,  $maxlines)" />
            <a href="{$lxrlinelink}"><xsl:value-of select="."/></a>
            <xsl:if test="@issue ne ''" >[<xsl:value-of select="@issue" />]</xsl:if>
            <xsl:if test="position() ne $count">,</xsl:if></xsl:for-each>
          (<xsl:value-of select="count(issues/line)" />)
        </xsl:otherwise>
      </xsl:choose>
    </li>
  </xsl:template>

  <xsl:template name="check">
    <xsl:param name="fileType" as="xsd:string" />
    <xsl:variable name="issueCount" as="xsd:integer" select="ebn:issueCount($fileType, @desc)" />

    <li>
      <span class="toolmsg">
        <xsl:choose>
          <xsl:when test="$issueCount > 0">
            <xsl:value-of select="@desc" />
            <b>OOPS! <xsl:value-of select="$issueCount"/> issues found!</b>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@desc" /><b>okay!</b>
          </xsl:otherwise>
        </xsl:choose>
      </span>
      <xsl:if test="$issueCount > 0" >
      <ul>
        <xsl:for-each select="file">
          <xsl:call-template name="file">
          </xsl:call-template>
        </xsl:for-each>
      </ul>
      <p class="explanation">
        <xsl:value-of select="explanation" disable-output-escaping="yes" />
      </p>
      </xsl:if>
    </li>
  </xsl:template>

  <xsl:template name="file-type" >
    <xsl:variable name="fileType" select="@value" />
    <li>
      <b><u>For File Type <xsl:value-of select="$fileType" /></u></b>
      <ol>
        <xsl:for-each select="check">
          <xsl:call-template name="check">
            <xsl:with-param name="fileType" select="$fileType" />
          </xsl:call-template>
        </xsl:for-each>
      </ol>
    </li>
  </xsl:template>

  <xsl:template match="file-types" mode="krazy2ebn" >
    <xsl:for-each select="file-type">
      <xsl:call-template name="file-type"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="global" mode="krazy2ebn" >
    <h1>Krazy2 Analysis of
    <xsl:value-of select="$module" />/<xsl:value-of select="$submodule" />
    (Revision = <xsl:value-of select="ebn:repoRev()" />)
    </h1>
    <p>Checksets = <xsl:value-of select="ebn:checkSets()" /><br />
    Checkers Run = <xsl:value-of select="ebn:checkerCount('all')" /><br />
    Files Processed = <xsl:value-of select="ebn:processedFilesCount()" /><br />
    Total Issues = <xsl:value-of select="ebn:issueCount('all','all')" />
    ...as of <xsl:value-of select="ebn:dateOfRun()" /></p>
  </xsl:template>

  <xsl:template match="/tool-result" mode="krazy2ebn">
    <html xml:lang="en" lang="en">
      <head>
        <title>krazy2 Analysis</title>
        <link rel="stylesheet" type="text/css" title="Normal"
          href="style.css" />
      </head>
      <body>
        <div id="title">
        </div>
        <div id="content">
          <div class="inside">
            <xsl:apply-templates select="global" mode="krazy2ebn" />
            <ul>
              <xsl:apply-templates select="file-types" mode="krazy2ebn" />
            </ul>
          </div>
        </div>
        <div id="footer">
        </div>
      </body>
     </html>
  </xsl:template>

</xsl:stylesheet>

<!-- kate:space-indent on; -->
