<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output 
        encoding="UTF-8"
        method="html"
        doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
        doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>
    <xsl:param name="testParam">error: no xsl param</xsl:param>
    <xsl:template match="/Page">
        <html xmlns="http://www.w3.org/1999/xhtml">
        <head>
            <title>page</title>
        </head>
        <body>
            <b>Xform Param</b>
            <p><xsl:value-of select="$testParam"/></p>
            <b>Cookies Get</b>
            <ul>
                <xsl:choose>
                    <xsl:when test="CookiesGet/Cookie"><xsl:apply-templates select="CookiesGet/Cookie"/></xsl:when>
                    <xsl:otherwise><li>none</li></xsl:otherwise>
                </xsl:choose>
            </ul>
            <b>Cookies Set</b>
            <ul>
                <xsl:choose>
                    <xsl:when test="CookiesSet/Cookie"><xsl:apply-templates select="CookiesSet/Cookie"/></xsl:when>
                    <xsl:otherwise><li>none</li></xsl:otherwise>
                </xsl:choose>
            </ul>
            <a href="/">Get Cookies</a>
        </body>
        </html>
    </xsl:template>
    <xsl:template match="Cookie">
        <li><xsl:value-of select="@key"/> : <xsl:value-of select="@value"/></li>
    </xsl:template>
</xsl:stylesheet>
