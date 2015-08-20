<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0">

  <xsl:output method="text"/>
  <xsl:strip-space elements="*"/>

  <xsl:template match="SecDef">
    <xsl:for-each select="Instrmt">
      <xsl:value-of select="@Exch"/>
      <xsl:text>&#0009;</xsl:text>
      <xsl:value-of select="@Sym"/>
      <xsl:text>&#0009;</xsl:text>
      <xsl:value-of select="@MatDt"/>
      <xsl:text>&#0010;</xsl:text>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="text()"/>

</xsl:stylesheet>
