<?xml version="1.0" encoding="UTF-8"?>
<!--
Copyright (c) 2013 by the author(s)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Author(s):
  Philipp Wagner <philipp.wagner@tum.de>

================================================================================

Convert a OpTiMSoC System Description XML into its generic representation

The OpTiMSoC System Description XML provides two way of describing a NoC:

a) A generic way, which describes each router and all NoC links separately.
   This form is rather verbose (to cover all edge cases).

b) A simplified form to describe a well-known interconnect structure, e.g.
   a mesh. This simplified form can then be converted automatically into the
   generic form. This way any application only needs to be able to read the
   generic form, all other forms are automatically converted.

This stylesheet does the conversion from the simplified form to the generic
form.
-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:optimsoc="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns:svg="http://www.w3.org/2000/svg">

  <xsl:output method="xml" indent="yes" encoding="utf-8"/>

  <xsl:include href="mesh-to-svg.xsl"/>

  <!--
  Identity transform

  Copy everything not overwritten below into the output document.
  -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

  <!--
  Autogenerate the NoC layout for known NoC types
  -->
  <xsl:template match="optimsoc:layout[@autogen]">
    <layout>
      <xsl:choose>
        <xsl:when test="/optimsoc:system/optimsoc:meshnoc">
          <xsl:call-template name="svg-mesh"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:message terminate="yes">
            Error: Unable to auto-generate NoC layout.
          </xsl:message>
        </xsl:otherwise>
      </xsl:choose>

    </layout>
  </xsl:template>

  <!--
  Mesh NoC to Generic NoC transformation

  This transformation replaces the optimsoc:meshnoc element, which is a
  convenience method for defining a mesh NoC, with a optimsoc:genericnoc
  element, which is the generic case of defining a network structure.
  Since the generic NoC is more verbose (since it needs to cover all special
  cases) the optimsoc:meshnoc element provides a quick alternative when
  manually creating a mesh.
  -->
  <xsl:template match="optimsoc:meshnoc">
    <optimsoc:genericnoc>
      <optimsoc:routers>
        <xsl:call-template name="insert-router">
          <xsl:with-param name="x" select="0"/>
          <xsl:with-param name="y" select="0"/>
          <xsl:with-param name="xdim" select="@xdim"/>
          <xsl:with-param name="ydim" select="@ydim"/>
        </xsl:call-template>
      </optimsoc:routers>

      <optimsoc:links>
        <xsl:call-template name="insert-router-links-horizontal">
          <xsl:with-param name="x" select="0"/>
          <xsl:with-param name="y" select="0"/>
          <xsl:with-param name="xdim" select="@xdim"/>
          <xsl:with-param name="ydim" select="@ydim"/>
        </xsl:call-template>

        <xsl:call-template name="insert-router-links-vertical">
          <xsl:with-param name="x" select="0"/>
          <xsl:with-param name="y" select="0"/>
          <xsl:with-param name="xdim" select="@xdim"/>
          <xsl:with-param name="ydim" select="@ydim"/>
        </xsl:call-template>

        <xsl:apply-templates select="optimsoc:locallinks"/>
      </optimsoc:links>
    </optimsoc:genericnoc>
  </xsl:template>

  <!--
  Insert a single mesh NoC router
  -->
  <xsl:template name="insert-router">
    <xsl:param name="x"/>
    <xsl:param name="y"/>
    <xsl:param name="xdim"/>
    <xsl:param name="ydim"/>

    <optimsoc:router>
      <xsl:attribute name="id"><xsl:value-of select="concat('r', $x, $y)"/></xsl:attribute>
      <optimsoc:ports>
        <xsl:if test="$x != 0">
          <optimsoc:port name="west"/>
        </xsl:if>
        <xsl:if test="$x != ($xdim - 1)">
          <optimsoc:port name="east"/>
        </xsl:if>
        <xsl:if test="$y != 0">
          <optimsoc:port name="north"/>
        </xsl:if>
        <xsl:if test="$y != ($ydim - 1)">
          <optimsoc:port name="south"/>
        </xsl:if>
        <optimsoc:port name="local"/>
      </optimsoc:ports>
    </optimsoc:router>

    <xsl:if test="$x &lt; ($xdim - 1)">
      <xsl:call-template name="insert-router">
        <xsl:with-param name="x" select="$x + 1"/>
        <xsl:with-param name="y" select="$y"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="$x = ($xdim - 1) and $y &lt; ($ydim - 1)">
      <xsl:call-template name="insert-router">
        <xsl:with-param name="x" select="0"/>
        <xsl:with-param name="y" select="$y + 1"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <!--
  Insert the horizontal inter-router links (east-west)
  -->
  <xsl:template name="insert-router-links-horizontal">
    <xsl:param name="x"/>
    <xsl:param name="y"/>
    <xsl:param name="xdim"/>
    <xsl:param name="ydim"/>

    <optimsoc:link>
      <optimsoc:con>
        <optimsoc:routercon port="east">
          <xsl:attribute name="router"><xsl:value-of select="concat('r', $x, $y)"/></xsl:attribute>
        </optimsoc:routercon>
      </optimsoc:con>
      <optimsoc:con>
        <optimsoc:routercon port="west">
          <xsl:attribute name="router"><xsl:value-of select="concat('r', $x + 1, $y)"/></xsl:attribute>
        </optimsoc:routercon>
      </optimsoc:con>
    </optimsoc:link>

    <xsl:if test="$x &lt; ($xdim - 2)">
      <xsl:call-template name="insert-router-links-horizontal">
        <xsl:with-param name="x" select="$x + 1"/>
        <xsl:with-param name="y" select="$y"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="$x = ($xdim - 2) and $y &lt; ($ydim - 1)">
      <xsl:call-template name="insert-router-links-horizontal">
        <xsl:with-param name="x" select="0"/>
        <xsl:with-param name="y" select="$y + 1"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <!--
  Insert the vertical inter-router links (north-south)
  -->
  <xsl:template name="insert-router-links-vertical">
    <xsl:param name="x"/>
    <xsl:param name="y"/>
    <xsl:param name="xdim"/>
    <xsl:param name="ydim"/>

    <optimsoc:link>
      <optimsoc:con>
        <optimsoc:routercon port="south">
          <xsl:attribute name="router"><xsl:value-of select="concat('r', $x, $y)"/></xsl:attribute>
        </optimsoc:routercon>
      </optimsoc:con>
      <optimsoc:con>
        <optimsoc:routercon port="north">
          <xsl:attribute name="router"><xsl:value-of select="concat('r', $x, $y + 1)"/></xsl:attribute>
        </optimsoc:routercon>
      </optimsoc:con>
    </optimsoc:link>

    <xsl:if test="$y &lt; ($ydim - 2)">
      <xsl:call-template name="insert-router-links-vertical">
        <xsl:with-param name="x" select="$x"/>
        <xsl:with-param name="y" select="$y + 1"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="$y = ($ydim - 2) and $x &lt; ($xdim - 1)">
      <xsl:call-template name="insert-router-links-vertical">
        <xsl:with-param name="x" select="$x + 1"/>
        <xsl:with-param name="y" select="0"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <!--
  Convert local links from the convenience connections to generic connections
  -->
  <xsl:template match="optimsoc:locallinks">
    <xsl:apply-templates select="optimsoc:locallink"/>
  </xsl:template>

  <xsl:template match="optimsoc:locallink">
    <optimsoc:link>
      <xsl:apply-templates select="optimsoc:con"/>
    </optimsoc:link>
  </xsl:template>

  <xsl:template match="optimsoc:meshroutercon">
    <optimsoc:routercon port="local">
      <xsl:attribute name="router"><xsl:value-of select="concat('r', @x, @y)"/></xsl:attribute>
    </optimsoc:routercon>
  </xsl:template>
</xsl:stylesheet>
