#!/bin/bash
#
# Install build dependencies for OpTiMSoC
#
# Environment variables provide control over dependencies which are not
# required in all scenarios.

# INSTALL_DOC_DEPS=yes|no
#   Install all dependencies required to build the documentation.
#   This includes texlive, a rather large dependency.
#
# INSTALL_BUILDROOT_DEPS=yes|no
#   Install all dependencies to create a Linux boot image using buildroot.
#   "make test" does this as part of a systemtest.
#
set -e

[ -f /etc/os-release ] || (echo "/etc/os-release doesn't exist."; exit 1)
. /etc/os-release

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

SUDO_CMD=""
if [ $(id -u) -ne 0 ]; then
  SUDO_CMD="sudo "
fi

INSTALL_DOC_DEPS=${INSTALL_DOC_DEPS:-yes}
INSTALL_BUILDROOT_DEPS=${INSTALL_BUILDROOT_DEPS:-yes}

case "$ID" in
  ubuntu)
    PKGLIST="curl git build-essential autoconf automake \
      libtool pkg-config tcl libusb-1.0-0-dev libboost-dev python \
      python3 python3-pip python3-venv libffi-dev python3-packaging \
      python3-yaml uuid-dev libpopt-dev"
    if [ "$INSTALL_DOC_DEPS" == "yes" ]; then
      PKGLIST="$PKGLIST doxygen latexmk texlive texlive-latex-extra \
        texlive-fonts-extra"
    fi
    if [ "$INSTALL_BUILDROOT_DEPS" == "yes" ]; then
      # All required dependencies + ncurses5 for "make menuconfig"
      # https://buildroot.org/downloads/manual/manual.html#requirement
      PKGLIST="$PKGLIST sed make binutils build-essential gcc g++ bash \
        patch gzip bzip2 perl tar cpio python unzip rsync file bc wget \
        libncurses5-dev"
    fi

    $SUDO_CMD apt-get -y install $PKGLIST
    ;;
  *)
    echo Unknown distribution "$ID". Please extend this script!
    exit 1
    ;;
esac

# Install build dependencies for osd-sw
$SCRIPT_DIR/../external/opensocdebug/software/install-build-deps.sh
