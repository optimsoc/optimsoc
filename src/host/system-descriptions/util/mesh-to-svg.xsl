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

Convert a OpTiMSoC System Description XML Mesh into a SVG representation

This XSLT stylesheet convert a tiled multicore system with a mesh interconnect,
as described in a OpTiMSoC System Description XML into a SVG graphic showing
the tiles and the interconnect.

Most drawing parameters can be set as configuration options below, e.g. the
box sizes, colors, the font size, etc. Much care has been taken to allow the
graphic rendering to be only dependant on those parameters, but still, not all
combinations will work and care needs to be taken not to produce wrong-looking
results.

For usage in the GUI each relevant component in the graphic is overlayed with a
transparent box with a optimsoc-idref attribute set to the ID of the component
it represents. The transparent box is required to prevent the SVG viewer to show
selectable text, etc.
-->
<xsl:stylesheet version="1.0"
                xmlns="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns:o="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns:optimsoc="http://www.optimsoc.org/xmlns/optimsoc-system"
                xmlns:svg="http://www.w3.org/2000/svg"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!--
  Configuration Parameters

  Note that while those parameters can be freely edited, not all combinations
  make sense. E.g. it does not make sense to make the boxes (e.g. SIZE_TILE)
  smaller than the BOX_PADDING.
  -->
  <!-- font size [px] -->
  <xsl:variable name="FONT_SIZE" select="12"/>

  <!-- box padding (distance between a box border and its contents) [px] -->
  <xsl:variable name="BOX_PADDING" select="10"/>

  <!-- edge length of a router [px] -->
  <xsl:variable name="SIZE_ROUTER" select="50"/>

  <!-- edge length of a tile [px] -->
  <xsl:variable name="SIZE_TILE" select="100"/>

  <!-- length of the tile to router link in x direction [px] -->
  <xsl:variable name="TILE_ROUTER_CONNECTION_X" select="20"/>
  <!-- length of the tile to router link in y direction [px] -->
  <xsl:variable name="TILE_ROUTER_CONNECTION_Y" select="20"/>

  <!-- box colors for memories (fill and stroke) -->
  <xsl:variable name="COLOR_MEMORY_FILL" select="'#95c0e0'"/>
  <xsl:variable name="COLOR_MEMORY_STROKE" select="'#64a0c8'"/>

  <!-- box colors for compute tile cores (fill and stroke) -->
  <xsl:variable name="COLOR_CORE_FILL" select="'#f08e46'"/>
  <xsl:variable name="COLOR_CORE_STROKE" select="'#e37222'"/>

  <!--
  Tile fill color

  This template is called to determine the fill (background) color of the
  different tile types. Adapt this template if you introduce new tile types.
  -->
  <xsl:template name="svg-get-tile-color-fill">
    <xsl:param name="tiletype"/>

    <xsl:choose>
      <xsl:when test="$tiletype = 'computetile'">#bec665</xsl:when>
      <xsl:otherwise>#ffffff</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  Tile stroke color

  This template is called to determine the stroke (border) color of the
  different tile types. Adapt this template if you introduce new tile types.
  -->
  <xsl:template name="svg-get-tile-color-stroke">
    <xsl:param name="tiletype"/>

    <xsl:choose>
      <xsl:when test="$tiletype = 'computetile'">#a2ad00</xsl:when>
      <xsl:otherwise>#ffffff</xsl:otherwise>
    </xsl:choose>
  </xsl:template>



  <!--
  Top-level: generate a SVG image for a mesh NoC

  This template creates a whole SVG image (including the svg:svg root node) for
  the mesh in a OpTiMSoC system description.
  -->
  <xsl:template name="svg-mesh">
    <svg:svg>
      <!-- whole image dimensions -->
      <xsl:attribute name="width">
        <xsl:value-of select="/o:system/o:meshnoc/@xdim * (2 * $SIZE_ROUTER + $SIZE_TILE + $TILE_ROUTER_CONNECTION_X)"/>
      </xsl:attribute>
      <xsl:attribute name="height">
        <xsl:value-of select="/o:system/o:meshnoc/@ydim * (2 * $SIZE_ROUTER + $SIZE_TILE + $TILE_ROUTER_CONNECTION_Y)"/>
      </xsl:attribute>

      <!-- insert all basic blocks -->
      <xsl:call-template name="svg-mesh-insert-basic-block">
        <xsl:with-param name="x" select="0"/>
        <xsl:with-param name="y" select="0"/>
        <xsl:with-param name="xdim" select="/o:system/o:meshnoc/@xdim"/>
        <xsl:with-param name="ydim" select="/o:system/o:meshnoc/@ydim"/>
      </xsl:call-template>
    </svg:svg>
  </xsl:template>

  <!--
  Draw a single basic block

  A basic block in the mesh consists of the tile, the router, the connection
  between tile and router, as well as the NoC links to the east and to the
  south (if applicable, i.e. if the tile is not at the right or bottom border).

  This template calls itself recursively to insert all blocks.
  -->
  <xsl:template name="svg-mesh-insert-basic-block">
    <xsl:param name="x"/>
    <xsl:param name="y"/>
    <xsl:param name="xdim"/>
    <xsl:param name="ydim"/>

    <xsl:param name="tileid">
      <xsl:call-template name="svg-get-tileid">
        <xsl:with-param name="x" select="$x"/>
        <xsl:with-param name="y" select="$y"/>
      </xsl:call-template>
    </xsl:param>
    <xsl:param name="tiletype">
      <xsl:call-template name="svg-get-tile-type">
        <xsl:with-param name="tileid"><xsl:value-of select="$tileid"/></xsl:with-param>
      </xsl:call-template>
    </xsl:param>

    <!-- Origin (0,0) of the basic block we're drawing here -->
    <xsl:param name="bbOriginX" select="$x * ($SIZE_TILE + 2 * $SIZE_ROUTER)"/>
    <xsl:param name="bbOriginY" select="$y * ($SIZE_TILE + 2 * $SIZE_ROUTER)"/>

    <!-- Tile outline -->
    <svg:rect style="stroke-width:1px">
      <xsl:attribute name="width"><xsl:value-of select="$SIZE_TILE"/></xsl:attribute>
      <xsl:attribute name="height"><xsl:value-of select="$SIZE_TILE"/></xsl:attribute>
      <xsl:attribute name="x"><xsl:value-of select="$bbOriginX"/></xsl:attribute>
      <xsl:attribute name="y"><xsl:value-of select="$bbOriginY"/></xsl:attribute>
      <xsl:attribute name="fill">
        <xsl:call-template name="svg-get-tile-color-fill">
          <xsl:with-param name="tiletype"><xsl:value-of select="$tiletype"/></xsl:with-param>
        </xsl:call-template>
      </xsl:attribute>
      <xsl:attribute name="stroke">
        <xsl:call-template name="svg-get-tile-color-stroke">
          <xsl:with-param name="tiletype"><xsl:value-of select="$tiletype"/></xsl:with-param>
        </xsl:call-template>
      </xsl:attribute>
    </svg:rect>

    <!-- Tile label -->
    <svg:text text-anchor="middle" font-family="Sans">
      <xsl:attribute name="font-size"><xsl:value-of select="$FONT_SIZE"/></xsl:attribute>
      <xsl:attribute name="x"><xsl:value-of select="$bbOriginX + $SIZE_TILE div 2"/></xsl:attribute>
      <xsl:attribute name="y"><xsl:value-of select="$bbOriginY + 1.5 * $FONT_SIZE"/></xsl:attribute>
      <xsl:call-template name="svg-label-for-tile">
        <xsl:with-param name="x" select="$x"/>
        <xsl:with-param name="y" select="$y"/>
      </xsl:call-template>
    </svg:text>

    <!-- Tile: transparent box for UI -->
    <svg:rect style="opacity:0;fill:#ffffff;stroke:none">
      <xsl:attribute name="width"><xsl:value-of select="$SIZE_TILE"/></xsl:attribute>
      <xsl:attribute name="height"><xsl:value-of select="$SIZE_TILE"/></xsl:attribute>
      <xsl:attribute name="x"><xsl:value-of select="$bbOriginX"/></xsl:attribute>
      <xsl:attribute name="y"><xsl:value-of select="$bbOriginY"/></xsl:attribute>
      <xsl:attribute name="optimsoc-idref"><xsl:value-of select="$tileid"/></xsl:attribute>
    </svg:rect>

    <!-- tile contents -->
    <xsl:call-template name="svg-draw-tile-contents">
      <xsl:with-param name="tileid"><xsl:value-of select="$tileid"/></xsl:with-param>
      <xsl:with-param name="x0"><xsl:value-of select="$bbOriginX + $BOX_PADDING"/></xsl:with-param>
      <xsl:with-param name="y0"><xsl:value-of select="$bbOriginY + 2 * $FONT_SIZE"/></xsl:with-param>
      <xsl:with-param name="width"><xsl:value-of select="$SIZE_TILE - 2 * $BOX_PADDING"/></xsl:with-param>
      <xsl:with-param name="height"><xsl:value-of select="$SIZE_TILE - $BOX_PADDING - 2 * $FONT_SIZE"/></xsl:with-param>
    </xsl:call-template>

    <!-- Router -->
    <svg:rect style="fill:none;stroke:#000000;stroke-width:1px">
      <xsl:attribute name="width"><xsl:value-of select="$SIZE_ROUTER"/></xsl:attribute>
      <xsl:attribute name="height"><xsl:value-of select="$SIZE_ROUTER"/></xsl:attribute>
      <xsl:attribute name="x"><xsl:value-of select="$bbOriginX + $TILE_ROUTER_CONNECTION_X + $SIZE_TILE"/></xsl:attribute>
      <xsl:attribute name="y"><xsl:value-of select="$bbOriginY + $TILE_ROUTER_CONNECTION_Y + $SIZE_TILE"/></xsl:attribute>
    </svg:rect>

    <!-- NoC router local link between tile and router -->
    <svg:path style="fill:none;stroke:#000000;stroke-width:1px;">
      <xsl:attribute name="d">
        <!-- starting point (absolute coordinates) -->
        <xsl:text>M</xsl:text>
        <xsl:value-of select="$bbOriginX + $SIZE_TILE"/><xsl:text> </xsl:text>
        <xsl:value-of select="$bbOriginY + $SIZE_TILE"/>
        <!-- length of the line (relative coordinates) -->
        <xsl:text> l</xsl:text>
        <xsl:value-of select="$TILE_ROUTER_CONNECTION_X"/><xsl:text> </xsl:text>
        <xsl:value-of select="$TILE_ROUTER_CONNECTION_Y"/>
      </xsl:attribute>
    </svg:path>

    <!-- NoC link horizontal -->
    <xsl:if test="$x != $xdim - 1">
      <svg:path style="fill:none;stroke:#000000;stroke-width:2px;">
        <xsl:attribute name="d">
          <!-- starting point (absolute coordinates) -->
          <xsl:text>M</xsl:text>
          <xsl:value-of select="$bbOriginX + $SIZE_TILE + $TILE_ROUTER_CONNECTION_X + $SIZE_ROUTER"/><xsl:text> </xsl:text>
          <xsl:value-of select="$bbOriginY + $SIZE_TILE + $TILE_ROUTER_CONNECTION_Y + $SIZE_ROUTER div 2"/>
          <!-- length of the line (relative coordinates) -->
          <xsl:text> l</xsl:text>
          <xsl:value-of select="$SIZE_TILE + $SIZE_ROUTER"/><xsl:text> </xsl:text>
          <xsl:value-of select="0"/>
        </xsl:attribute>
      </svg:path>
    </xsl:if>

    <!-- NoC link vertical -->
    <xsl:if test="$y != $ydim - 1">
      <svg:path style="fill:none;stroke:#000000;stroke-width:2px;">
        <xsl:attribute name="d">
          <!-- starting point (absolute coordinates) -->
          <xsl:text>M</xsl:text>
          <xsl:value-of select="$bbOriginX + $SIZE_TILE + $TILE_ROUTER_CONNECTION_X + $SIZE_ROUTER div 2"/><xsl:text> </xsl:text>
          <xsl:value-of select="$bbOriginY + $SIZE_TILE + $TILE_ROUTER_CONNECTION_Y + $SIZE_ROUTER"/>
          <!-- length of the line (relative coordinates) -->
          <xsl:text> l</xsl:text>
          <xsl:value-of select="0"/><xsl:text> </xsl:text>
          <xsl:value-of select="$SIZE_TILE + $SIZE_ROUTER"/>
        </xsl:attribute>
      </svg:path>
    </xsl:if>

    <!-- recurse to the next basic block -->
    <xsl:if test="$x &lt; ($xdim - 1)">
      <xsl:call-template name="svg-mesh-insert-basic-block">
        <xsl:with-param name="x" select="$x + 1"/>
        <xsl:with-param name="y" select="$y"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="$x = ($xdim - 1) and $y &lt; ($ydim - 1)">
      <xsl:call-template name="svg-mesh-insert-basic-block">
        <xsl:with-param name="x" select="0"/>
        <xsl:with-param name="y" select="$y + 1"/>
        <xsl:with-param name="xdim" select="$xdim"/>
        <xsl:with-param name="ydim" select="$ydim"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <!--
  Get the label for a tile at mesh position (x,y)
  -->
  <xsl:template name="svg-label-for-tile">
    <xsl:param name="x"/>
    <xsl:param name="y"/>

    <xsl:param name="tileid">
      <xsl:call-template name="svg-get-tileid">
        <xsl:with-param name="x" select="$x"/>
        <xsl:with-param name="y" select="$y"/>
      </xsl:call-template>
    </xsl:param>
    <xsl:param name="tiletype">
      <xsl:call-template name="svg-get-tile-type">
        <xsl:with-param name="tileid"><xsl:value-of select="$tileid"/></xsl:with-param>
      </xsl:call-template>
    </xsl:param>

    <xsl:if test="$tiletype = 'computetile'">
      <xsl:text>compute tile </xsl:text>
      <xsl:value-of select="/o:system/o:tiles/*[@id = $tileid]/@computetileid"/>
    </xsl:if>
  </xsl:template>

  <!--
  Get the label for a memory with a given memoryid
  -->
  <xsl:template name="svg-label-for-memory">
    <xsl:param name="memoryid"/>

    <xsl:text>memory </xsl:text>
    <xsl:value-of select="$memoryid"/>
  </xsl:template>

  <!--
  Get the tile ID for a tile in a given (x,y) mesh position
  -->
  <xsl:template name="svg-get-tileid">
    <xsl:param name="x"/>
    <xsl:param name="y"/>
    <xsl:value-of select="/o:system/o:meshnoc/o:locallinks/o:locallink[o:con/o:meshroutercon/@x=$x and o:con/o:meshroutercon/@y = $y]/o:con/o:tilecon/@tile"/>
  </xsl:template>

  <!--
  Get the tile type for a given tile ID
  -->
  <xsl:template name="svg-get-tile-type">
    <xsl:param name="tileid"/>
    <xsl:value-of select="local-name(/o:system/o:tiles/*[@id = $tileid])"/>
  </xsl:template>

  <!--
  Draw the contents of a tile

  If you introduce a new tile type, edit this template to call your content
  template, like it is done for the compute tile. If your tile type does not
  need any inner contents you can simply leave this template as is.
  -->
  <xsl:template name="svg-draw-tile-contents">
    <xsl:param name="tileid"/>
    <xsl:param name="x0"/>
    <xsl:param name="y0"/>
    <xsl:param name="width"/>
    <xsl:param name="height"/>
    <xsl:param name="tiletype">
      <xsl:call-template name="svg-get-tile-type">
        <xsl:with-param name="tileid"><xsl:value-of select="$tileid"/></xsl:with-param>
      </xsl:call-template>
    </xsl:param>

    <xsl:choose>
      <xsl:when test="$tiletype = 'computetile'">
        <xsl:call-template name="svg-draw-tile-contents-computetile">
          <xsl:with-param name="tileid"><xsl:value-of select="$tileid"/></xsl:with-param>
          <xsl:with-param name="x0"><xsl:value-of select="$x0"/></xsl:with-param>
          <xsl:with-param name="y0"><xsl:value-of select="$y0"/></xsl:with-param>
          <xsl:with-param name="width"><xsl:value-of select="$width"/></xsl:with-param>
          <xsl:with-param name="height"><xsl:value-of select="$height"/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!--
  Draw the inner contents of a compute tile

  A compute tile contains a box for each core, as well as a box for a
  distributed memory (if applicable).
  -->
  <xsl:template name="svg-draw-tile-contents-computetile">
    <xsl:param name="tileid"/>
    <xsl:param name="x0"/>
    <xsl:param name="y0"/>
    <xsl:param name="width"/>
    <xsl:param name="height"/>
    <xsl:param name="corecount"><xsl:value-of select="count(/o:system/o:tiles/*[@id = $tileid]/o:cores/o:core)"/></xsl:param>
    <xsl:param name="coreboxwidth">
      <xsl:value-of select="$width div $corecount"/>
    </xsl:param>
    <xsl:param name="innerboxheight">
      <xsl:choose>
        <xsl:when test="/o:system/o:tiles/*[@id = $tileid]/o:memory">
          <xsl:value-of select="$height div 2 - $BOX_PADDING div 2"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$height"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:param>

    <!-- cores -->
    <xsl:for-each select="/o:system/o:tiles/*[@id = $tileid]/o:cores/o:core">
      <!-- box -->
      <svg:rect style="stroke-width:1px">
        <xsl:attribute name="width"><xsl:value-of select="$coreboxwidth"/></xsl:attribute>
        <xsl:attribute name="height"><xsl:value-of select="$innerboxheight"/></xsl:attribute>
        <xsl:attribute name="x"><xsl:value-of select="$x0 + (position() - 1) * $coreboxwidth"/></xsl:attribute>
        <xsl:attribute name="y"><xsl:value-of select="$y0"/></xsl:attribute>
        <xsl:attribute name="fill"><xsl:value-of select="$COLOR_CORE_FILL"/></xsl:attribute>
        <xsl:attribute name="stroke"><xsl:value-of select="$COLOR_CORE_STROKE"/></xsl:attribute>
      </svg:rect>

      <!-- label -->
      <svg:text text-anchor="middle" font-family="Sans">
        <xsl:attribute name="font-size"><xsl:value-of select="$FONT_SIZE"/></xsl:attribute>
        <xsl:attribute name="x"><xsl:value-of select="$x0 + (position() - 1) * $coreboxwidth + $coreboxwidth div 2"/></xsl:attribute>
        <xsl:attribute name="y"><xsl:value-of select="$y0 + 1.5 * $FONT_SIZE"/></xsl:attribute>
        <xsl:value-of select="concat('c', @coreid)"/>
      </svg:text>

      <!-- transparent box for user interface -->
      <svg:rect style="opacity:0;fill:#ffffff;stroke:none">
        <xsl:attribute name="width"><xsl:value-of select="$coreboxwidth"/></xsl:attribute>
        <xsl:attribute name="height"><xsl:value-of select="$innerboxheight"/></xsl:attribute>
        <xsl:attribute name="x"><xsl:value-of select="$x0 + (position() - 1) * $coreboxwidth"/></xsl:attribute>
        <xsl:attribute name="y"><xsl:value-of select="$y0"/></xsl:attribute>
        <xsl:attribute name="optimsoc-idref"><xsl:value-of select="@id"/></xsl:attribute>
      </svg:rect>
    </xsl:for-each>

    <!-- tile distributed memory -->
    <xsl:if test="/o:system/o:tiles/*[@id = $tileid]/o:memory">
      <!-- box -->
      <svg:rect style="stroke-width:1px">
        <xsl:attribute name="width"><xsl:value-of select="$width"/></xsl:attribute>
        <xsl:attribute name="height"><xsl:value-of select="$innerboxheight"/></xsl:attribute>
        <xsl:attribute name="x"><xsl:value-of select="$x0"/></xsl:attribute>
        <xsl:attribute name="y"><xsl:value-of select="$y0 + $innerboxheight + $BOX_PADDING"/></xsl:attribute>
        <xsl:attribute name="fill"><xsl:value-of select="$COLOR_MEMORY_FILL"/></xsl:attribute>
        <xsl:attribute name="stroke"><xsl:value-of select="$COLOR_MEMORY_STROKE"/></xsl:attribute>
      </svg:rect>

      <!-- label -->
      <svg:text text-anchor="middle" font-family="Sans">
        <xsl:attribute name="font-size"><xsl:value-of select="$FONT_SIZE"/></xsl:attribute>
        <xsl:attribute name="x"><xsl:value-of select="$x0 + $width div 2"/></xsl:attribute>
        <xsl:attribute name="y"><xsl:value-of select="$y0 + $innerboxheight + $BOX_PADDING + 1.5 * $FONT_SIZE"/></xsl:attribute>
        <xsl:call-template name="svg-label-for-memory">
          <xsl:with-param name="memoryid" select="/o:system/o:tiles/*[@id = $tileid]/o:memory/@memoryid"/>
        </xsl:call-template>
      </svg:text>

      <!-- transparent box for user interface -->
      <svg:rect style="opacity:0;fill:#ffffff;stroke:none">
        <xsl:attribute name="width"><xsl:value-of select="$width"/></xsl:attribute>
        <xsl:attribute name="height"><xsl:value-of select="$innerboxheight"/></xsl:attribute>
        <xsl:attribute name="x"><xsl:value-of select="$x0"/></xsl:attribute>
        <xsl:attribute name="y"><xsl:value-of select="$y0 + $innerboxheight + $BOX_PADDING"/></xsl:attribute>
        <xsl:attribute name="optimsoc-idref"><xsl:value-of select="/o:system/o:tiles/*[@id = $tileid]/o:memory/@id"/></xsl:attribute>
      </svg:rect>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
