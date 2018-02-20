#!/bin/bash
set -e

[ -f /etc/os-release ] || (echo "/etc/os-release doesn't exist."; exit 1)
. /etc/os-release

SUDO_CMD=""
if [ $(id -u) -ne 0 ]; then
  SUDO_CMD="sudo "
fi

INSTALL_DOC_DEPS=${INSTALL_DOC_DEPS:-yes}

case "$ID" in
  ubuntu)
    $SUDO_CMD apt-get -y install curl git build-essential autoconf automake \
        libtool pkg-config tcl libusb-1.0-0-dev libboost-dev python \
        python3 python3-pip python3-venv libffi-dev python3-packaging \
        python3-yaml
    if [ "$INSTALL_DOC_DEPS" == "yes" ]; then
        $SUDO_CMD apt-get -y install doxygen latexmk texlive texlive-latex-extra \
            texlive-fonts-extra
    fi
    ;;
  *)
    echo Unknown distribution "$ID". Please extend this script!
    exit 1
    ;;
esac

# Install build dependencies for osd-sw
./external/opensocdebug/software/install-build-deps.sh
