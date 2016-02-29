#!/bin/bash
# Copyright (c) 2013 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author(s):
#   Philipp Wagner <philipp.wagner@tum.de>

#
# Utility script for the OpTiMSoC System Description XML
#

UTIL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ACTION=$1
XML_FILE=$2

function display_help {
  echo "Usage: $0 ACTION XML_FILE"
  echo
  echo "Arguments:"
  echo "ACTION:   what should I do? One of validate|convert-generic|convert-svg"
  echo "XML_FILE: the input XML file"
}

if [ $# != 2 ]; then
  display_help
  exit 1
fi

if [ ! -f "$XML_FILE" ]; then
  echo "'$XML_FILE' is not a valid input file." >&2
  echo 
  display_help
  exit 1
fi

case "$ACTION" in
  "validate")
    xmllint --noout --schema "$UTIL_DIR/optimsoc-system.xsd" "$XML_FILE"
    ;;
  "convert-generic")
    xsltproc "$UTIL_DIR/convert-to-generic.xsl" "$XML_FILE" | xmllint --format -
    ;;
  "convert-svg")
    xsltproc "$UTIL_DIR/convert-to-svg.xsl" "$XML_FILE"
    ;;
  *)
    display_help
    ;;    
esac

