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
-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:optimsoc="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns:svg="http://www.w3.org/2000/svg">

  <xsl:output method="xml" indent="yes" encoding="utf-8"/>


  <xsl:include href="mesh-to-svg.xsl"/>

  <xsl:template match="/">
    <xsl:apply-templates select="//optimsoc:layout"/>
  </xsl:template>

  <!--
  Autogenerate NoC layout for known NoC types
  -->
  <xsl:template match="optimsoc:layout[@autogen]">
      <xsl:if test="/optimsoc:system/optimsoc:meshnoc">
        <xsl:call-template name="svg-mesh"/>
      </xsl:if>
  </xsl:template>

</xsl:stylesheet>
