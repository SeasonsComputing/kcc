<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text" encoding="UTF-8"/>
    <xsl:strip-space elements="*"/>
    <xsl:param name="template.param.type"/>
    <xsl:param name="template.param.generated"/>
    
    <xsl:template match="/">
        <xsl:apply-templates select="/Schema/Type[@name = $template.param.type]"/>
    </xsl:template>

    <xsl:template match="Type[@type = 'class']">
        <xsl:variable name="type" select="@name"/>
        
        <!-- Header -->
        <xsl:apply-templates select="." mode="header"/>
        
        <!-- Reference Type -->
        <xsl:text>    typedef </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>SharedPtr&lt;struct </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value&gt; </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>;&#xa;</xsl:text>
        
        <!-- Collection Type -->
        <xsl:if test="/Schema/Type/Attribute[@as = $type and @type = 'list']">
            <xsl:text>    typedef std::list&lt;</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>&gt; </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>List;&#xa;</xsl:text>
        </xsl:if>

        <!-- Value Type -->
        <xsl:text>    struct </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value : </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>IXMLSerializable&#xa;</xsl:text>
        <xsl:text>    {&#xa;</xsl:text>
        
        <!-- Attributes -->
        <xsl:for-each select="Attribute[@type = 'value']">
            <xsl:text>        </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>String </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'atomic']">
            <xsl:text>        </xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'list']">
            <xsl:text>        </xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>List </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'cdata']">
            <xsl:text>        </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>String </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        
        <!-- Ctor -->
        <xsl:text>        </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value();&#xa;</xsl:text>
        
        <!-- toXML -->
        <xsl:text>        virtual void toXML(</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>DOMWriter&amp; w) throw (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException);&#xa;</xsl:text>

        <!-- fromXML -->
        <xsl:text>        virtual void fromXML(const </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>IDOMNode* node, </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>DOMReader&amp; r) throw (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException);&#xa;</xsl:text>

        <!-- validate -->
        <xsl:text>        virtual void validate() throw (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException);&#xa;</xsl:text>

        <!-- Persistence -->
        <xsl:if test="@persist = 'true'">    
            <xsl:text>        virtual void write(</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>DOMWriter&amp; w) throw (</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>XMLSerializeException);&#xa;</xsl:text>
            <xsl:text>        virtual void read(</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>DOMReader&amp; r) throw (</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>XMLSerializeException);&#xa;</xsl:text>
        </xsl:if>
        
        <!-- metadata -->
        <xsl:text>        virtual const </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLMetadata&amp; metadata();&#xa;</xsl:text>

        <!-- Footer -->
        <xsl:text>    };&#xa;</xsl:text>
        <xsl:apply-templates select="." mode="footer"/>
    </xsl:template>

    <xsl:template match="Type[@type = 'enum']">
        <!-- Header -->
        <xsl:apply-templates select="." mode="header"/>
        <xsl:text>    struct </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>    {&#xa;</xsl:text>
        
        <!-- Attributes -->
        <xsl:for-each select="Attribute">
            <xsl:text>        static const </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>String&amp; </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>()&#xa;</xsl:text>
            <xsl:text>        {&#xa;</xsl:text>
            <xsl:text>            static const </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>String k_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>("</xsl:text>
            <xsl:value-of select="@values"/>
            <xsl:text>");&#xa;</xsl:text>
            <xsl:text>            return k_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>;&#xa;</xsl:text>
            <xsl:text>        }&#xa;</xsl:text>
        </xsl:for-each>
        
        <!-- Hidden Ctor -->
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>    private:&#xa;</xsl:text>
        <xsl:text>        </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>();&#xa;</xsl:text>
        
        <!-- Footer -->
        <xsl:text>    };&#xa;</xsl:text>
        <xsl:apply-templates select="." mode="footer"/>
    </xsl:template>

    <xsl:template match="Type" mode="header">
        <xsl:text>/**&#xa;</xsl:text>
        <xsl:text> * GENERATED FILE: </xsl:text>
        <xsl:value-of select="$template.param.generated"/>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text> */&#xa;</xsl:text>
        <xsl:text>#ifndef </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>_h&#xa;</xsl:text>
        <xsl:text>#define </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>_h&#xa;</xsl:text>
        <xsl:text>&#xa;</xsl:text>
        <xsl:if test="@type = 'class'">
            <xsl:text>#include &lt;inc/xml/IXMLSerializable.h&gt;&#xa;</xsl:text>
        </xsl:if>
        <xsl:for-each select="Attribute[@as != ../@name and (@type = 'atomic' or @type = 'list')]">
            <xsl:text>#include &lt;</xsl:text>
            <xsl:value-of select="/Schema/@path"/>
            <xsl:value-of select="@as"/>
            <xsl:text>.h&gt;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:if test="@type = 'class' or count(Attribute[@type = 'atomic' or @type = 'list']) != 0">
            <xsl:text>&#xa;</xsl:text>
        </xsl:if>
        <xsl:text>namespace </xsl:text>
        <xsl:value-of select="/Schema/@namespace"/>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>{&#xa;</xsl:text>
    </xsl:template>

    <xsl:template match="Type" mode="footer">
        <xsl:text>}&#xa;</xsl:text>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>#endif // </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>_h&#xa;</xsl:text>
    </xsl:template>
    
    <xsl:template name="namespace"><xsl:if test="/Schema/@namespace != 'kcc'">kcc::</xsl:if></xsl:template>
</xsl:stylesheet>
