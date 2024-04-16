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
        <xsl:text>/**&#xa;</xsl:text>
        <xsl:text> * GENERATED FILE: </xsl:text>
        <xsl:value-of select="$template.param.generated"/>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text> */&#xa;</xsl:text>
        <xsl:text>#include &lt;inc/core/Core.h&gt;&#xa;</xsl:text>
        <xsl:text>#include &lt;</xsl:text>
        <xsl:value-of select="/Schema/@path"/>
        <xsl:value-of select="@name"/>
        <xsl:text>.h&gt;&#xa;</xsl:text>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>namespace </xsl:text>
        <xsl:value-of select="/Schema/@namespace"/>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>{&#xa;</xsl:text>
        
        <!-- Metadata -->
        <xsl:text>    static struct </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Metadata : </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLMetadata&#xa;</xsl:text>
        <xsl:text>    {&#xa;</xsl:text>
        <xsl:for-each select="Attribute">
            <xsl:text>        </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>StringRC _</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <!-- Metadata Ctor -->        
        <xsl:text>        </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Metadata() :&#xa;</xsl:text>
        <xsl:text>            </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLMetadata("</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>")</xsl:text>
        <xsl:for-each select="Attribute[@type = 'value']">
            <xsl:text>,&#xa;</xsl:text>
            <xsl:text>            _</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>("</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>")</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'atomic' or @type = 'list' or @type = 'cdata']">
            <xsl:text>,&#xa;</xsl:text>
            <xsl:text>            _</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>("</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>")</xsl:text>
        </xsl:for-each>
        <xsl:text>&#xa;</xsl:text>
        <xsl:text>        {&#xa;</xsl:text>
        <xsl:for-each select="Attribute[@type != '']">
            <xsl:text>            attributes[_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>] = </xsl:text>
            <xsl:choose>
            <xsl:when test="@type = 'value'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLType::value()</xsl:text>
            </xsl:when>
            <xsl:when test="@type = 'atomic'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLType::atomic()</xsl:text>
            </xsl:when>
            <xsl:when test="@type = 'list'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLType::list()</xsl:text>
            </xsl:when>
            <xsl:when test="@type = 'cdata'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLType::cdata()</xsl:text>
            </xsl:when>
            <xsl:otherwise>"<xsl:value-of select="@type"/>"</xsl:otherwise>
            </xsl:choose>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@constraint != '']">
            <xsl:text>            constraints[_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>] = </xsl:text>
            <xsl:choose>
            <xsl:when test="@constraint = 'required'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLConstraint::required()</xsl:text>
            </xsl:when>
            <xsl:when test="@constraint = 'optional'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLConstraint::optional()</xsl:text>
            </xsl:when>
            <xsl:otherwise>"<xsl:value-of select="@constraint"/>"</xsl:otherwise>
            </xsl:choose>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'value' and @values]">
            <xsl:text>            values[_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>] = </xsl:text>
            <xsl:choose>
            <xsl:when test="@values = '{boolean}'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLValue::boolean()</xsl:text>
            </xsl:when>
            <xsl:when test="@values = '{number}'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLValue::number()</xsl:text>
            </xsl:when>
            <xsl:when test="@values = '{isodate}'">
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLValue::isodate()</xsl:text>
            </xsl:when>
            <xsl:otherwise>"<xsl:value-of select="@values"/>"</xsl:otherwise>
            </xsl:choose>
            <xsl:text>;&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:text>        };&#xa;</xsl:text>
        <xsl:text>    } k_metadata;&#xa;</xsl:text>
        
        <!-- Ctor -->
        <xsl:text>    </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value::</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value()</xsl:text>
        <xsl:choose>
        <xsl:when test="Attribute/@valueDefault or Attribute[@type = 'atomic' and @constraint = 'required']">
            <xsl:text>&#xa;    {&#xa;</xsl:text>
        </xsl:when>
        <xsl:otherwise>
            <xsl:text> {}&#xa;</xsl:text>
        </xsl:otherwise>
        </xsl:choose>
        <xsl:for-each select="Attribute">
            <xsl:choose>
            <xsl:when test="@type = 'value' and @valueDefault">
                <xsl:text>        </xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text> = "</xsl:text>
                <xsl:value-of select="@valueDefault"/>
                <xsl:text>";&#xa;</xsl:text>
            </xsl:when>
            <xsl:when test="@type = 'atomic' and @constraint = 'required'">
                <xsl:text>        </xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>.reset(new </xsl:text>
                <xsl:value-of select="@as"/>
                <xsl:text>Value);&#xa;</xsl:text>
            </xsl:when>
            </xsl:choose>
        </xsl:for-each>
        <xsl:if test="Attribute/@valueDefault or Attribute[@type = 'atomic' and @constraint = 'required']">
            <xsl:text>    }&#xa;</xsl:text>
        </xsl:if>
        
        <!-- toXML -->
        <xsl:text>    void </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value::</xsl:text>
        <xsl:text>toXML(</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>DOMWriter&amp; w) throw (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException)&#xa;</xsl:text>
        <xsl:text>    {&#xa;</xsl:text>
        <xsl:text>        try&#xa;</xsl:text>
        <xsl:text>        {&#xa;</xsl:text>
        <xsl:text>            w.start(k_metadata.type);&#xa;</xsl:text>
        <xsl:for-each select="Attribute[@type = 'value']">
            <xsl:text>            w.attr(k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>, </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'atomic']">
            <xsl:text>            if (</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text> != NULL) </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>->toXML(w);&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'list']">
            <xsl:text>            for (</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>List::iterator i = </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.begin(); i != </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.end(); i++) </xsl:text>
            <xsl:text>(*i)->toXML(w);&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'cdata']">
            <xsl:text>            </xsl:text>
            <xsl:text>w.start(k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
            <xsl:text>            </xsl:text>
            <xsl:text>w.cdata(</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
            <xsl:text>            </xsl:text>
            <xsl:text>w.end(k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:text>            w.end(k_metadata.type);&#xa;</xsl:text>
        <xsl:text>        }&#xa;</xsl:text>
        <xsl:text>        catch (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>Exception&amp; e)&#xa;</xsl:text>
        <xsl:text>        {&#xa;</xsl:text>
        <xsl:text>            throw </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException(e.what());&#xa;</xsl:text>
        <xsl:text>        }&#xa;</xsl:text>
        <xsl:text>    }&#xa;</xsl:text>

        <!-- fromXML -->
        <xsl:text>    void </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value::</xsl:text>
        <xsl:text>fromXML(const </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>IDOMNode* node, </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>DOMReader&amp; r) throw (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException)&#xa;</xsl:text>
        <xsl:text>    {&#xa;</xsl:text>
        <xsl:text>        try&#xa;</xsl:text>
        <xsl:text>        {&#xa;</xsl:text>        
        <xsl:for-each select="Attribute[@type = 'value']">
            <xsl:text>            r.attrOp(node, k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>, </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'atomic']">
            <xsl:text>            </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.reset();&#xa;</xsl:text>
            <xsl:text>            const </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>IDOMNode* n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text> = r.nodeOp(node, k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
            <xsl:text>            if (n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text> != NULL)&#xa;</xsl:text>
            <xsl:text>            {&#xa;</xsl:text>
            <xsl:text>                </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.reset(new </xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>Value);&#xa;</xsl:text>
            <xsl:text>                </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>->fromXML(n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>, r);&#xa;</xsl:text>
            <xsl:text>            }&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'list']">
            <xsl:text>            </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.clear();&#xa;</xsl:text>
            <xsl:text>            </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>AutoPtr&lt;</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>IDOMNodeList&gt; n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>List(r.nodes(node, k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>));&#xa;</xsl:text>
            <xsl:text>            for (long i = 0; i &lt; n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>List->getLength(); i++)&#xa;</xsl:text>
            <xsl:text>            {&#xa;</xsl:text>        
            <xsl:text>                const </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>IDOMNode* n = n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>List->getItem(i);&#xa;</xsl:text>
            <xsl:text>                </xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text> o(new </xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>Value);&#xa;</xsl:text>
            <xsl:text>                o->fromXML(n, r);&#xa;</xsl:text>
            <xsl:text>                </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.push_back(o);&#xa;</xsl:text>
            <xsl:text>            }&#xa;</xsl:text>        
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'cdata']">
            <xsl:text>            </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.clear();&#xa;</xsl:text>
            <xsl:text>            const </xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>IDOMNode* n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text> = r.nodeOp(node, k_metadata._</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>);&#xa;</xsl:text>
            <xsl:text>            if (n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text> != NULL) </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text> = r.text(n</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>);&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:text>        }&#xa;</xsl:text>
        <xsl:text>        catch (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>Exception&amp; e)&#xa;</xsl:text>
        <xsl:text>        {&#xa;</xsl:text>
        <xsl:text>            throw </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException(e.what());&#xa;</xsl:text>
        <xsl:text>        }&#xa;</xsl:text>
        <xsl:text>    }&#xa;</xsl:text>

        <!-- validate -->
        <xsl:text>    void </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value::</xsl:text>
        <xsl:text>validate() throw (</xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLSerializeException)&#xa;</xsl:text>
        <xsl:text>    {&#xa;</xsl:text>
        <xsl:for-each select="Attribute[@type = 'value']">
            <xsl:if test="@constraint = 'required'">
                <xsl:text>        if (</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>.empty()) throw </xsl:text>
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLSerializeException("missing required attribute: </xsl:text>
                <xsl:value-of select="../@name"/>
                <xsl:text>/@</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>");&#xa;</xsl:text>
            </xsl:if>
            <xsl:if test="@values">
                <xsl:text>        if (!</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>.empty() &amp;&amp; !</xsl:text>
                <xsl:call-template name="namespace"/>
                <xsl:text>Strings::match(</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>, k_metadata.values[k_metadata._</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>])) throw </xsl:text>
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLSerializeException("illegal value for attribute: </xsl:text>
                <xsl:value-of select="../@name"/>
                <xsl:text>/@</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>");&#xa;</xsl:text>
            </xsl:if>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'atomic']">
            <xsl:if test="@constraint = 'required'">
                <xsl:text>        if (</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text> == NULL) throw </xsl:text>
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLSerializeException("missing required atomic: </xsl:text>
                <xsl:value-of select="../@name"/>
                <xsl:text>/</xsl:text>
                <xsl:value-of select="@as"/>
                <xsl:text>");&#xa;</xsl:text>
                <xsl:text>        </xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>->validate();&#xa;</xsl:text>
            </xsl:if>
            <xsl:if test="@constraint = 'optional'">
                <xsl:text>        if (</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text> != NULL) </xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>->validate();&#xa;</xsl:text>
            </xsl:if>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'list']">
            <xsl:if test="@constraint = 'required'">
                <xsl:text>        if (</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>.empty()) throw </xsl:text>
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLSerializeException("missing required list: </xsl:text>
                <xsl:value-of select="../@name"/>
                <xsl:text>/</xsl:text>
                <xsl:value-of select="@as"/>
                <xsl:text>");&#xa;</xsl:text>
            </xsl:if>
            <xsl:text>        for (</xsl:text>
            <xsl:value-of select="@as"/>
            <xsl:text>List::iterator i = </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.begin(); i != </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>.end(); i++) </xsl:text>
            <xsl:text>(*i)->validate();&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:for-each select="Attribute[@type = 'cdata']">
            <xsl:if test="@constraint = 'required'">
                <xsl:text>        if (</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>.empty()) throw </xsl:text>
                <xsl:call-template name="namespace"/>
                <xsl:text>XMLSerializeException("missing required attribute: </xsl:text>
                <xsl:value-of select="../@name"/>
                <xsl:text>/</xsl:text>
                <xsl:value-of select="@as"/>
                <xsl:text>");&#xa;</xsl:text>
            </xsl:if>
        </xsl:for-each>
        <xsl:text>    }&#xa;</xsl:text>

        <!-- Persistence -->
        <xsl:if test="@persist = 'true'">    
            <xsl:text>    void </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>Value::write(</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>DOMWriter&amp; w) throw (</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>XMLSerializeException)&#xa;</xsl:text>
            <xsl:text>    {&#xa;</xsl:text>
            <xsl:text>        validate();&#xa;</xsl:text>
            <xsl:text>        toXML(w);&#xa;</xsl:text>
            <xsl:text>    }&#xa;</xsl:text>
            <xsl:text>    void </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>Value::read(</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>DOMReader&amp; r) throw (</xsl:text>
            <xsl:call-template name="namespace"/>
            <xsl:text>XMLSerializeException)&#xa;</xsl:text>
            <xsl:text>    {&#xa;</xsl:text>
            <xsl:text>        fromXML(r.doc(k_metadata.type), r);&#xa;</xsl:text>
            <xsl:text>        validate();&#xa;</xsl:text>
            <xsl:text>    }&#xa;</xsl:text>
        </xsl:if>

        <!-- metadata -->
        <xsl:text>    const </xsl:text>
        <xsl:call-template name="namespace"/>
        <xsl:text>XMLMetadata&amp; </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>Value::metadata() { return k_metadata; }&#xa;</xsl:text>
        
        <!-- Footer -->
        <xsl:text>}&#xa;</xsl:text>
    </xsl:template>

    <xsl:template name="namespace"><xsl:if test="/Schema/@namespace != 'kcc'">kcc::</xsl:if></xsl:template>
</xsl:stylesheet>
