<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="2.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  exclude-result-prefixes="xsl xsd">
  
  <!-- Mode: krazy2ebn -->
  <xsl:import href="krazy2ebn.xsl" />
  
  <xsl:template match="/krazy">
    <error>unsupported mode!</error>
  </xsl:template>
</xsl:stylesheet>
