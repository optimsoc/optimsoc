#!/bin/bash
#
# Install a customized version of Eclipse for OpTiMSoC Development
#
# The installation and customization of Eclipse is performed through the
# mechanisms provided by Oomph. This script just downloads the installer and
# starts it.
#

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TEMPDIR="$(mktemp -d)"

INSTALLER_URL='https://www.eclipse.org/downloads/download.php?file=/oomph/products/eclipse-inst-linux64.tar.gz&r=1'

echo -n 'Downloading Eclipse installer...'
curl -Ls -o $TEMPDIR/eclipse-installer.tar.gz "$INSTALLER_URL"
echo ' done'

echo -n 'Extracting installer...'
tar -x -C $TEMPDIR -f $TEMPDIR/eclipse-installer.tar.gz
echo ' done'

# The Oomph installer uses a ResourceCopyTask to copy the optimsocide.desktop
# file to the user's applications directory. Since this task is not able to
# overwrite existing files, we delete a potentially existing old .desktop file
# here.
echo -n 'Deleting old .desktop file...'
rm -f $HOME/.local/share/applications/optimsocide.desktop 2>/dev/null
echo ' done'

echo 'Starting installation process...'
$TEMPDIR/eclipse-installer/eclipse-inst -vmargs "-Doomph.redirection.setups=http://git.eclipse.org/c/oomph/org.eclipse.oomph.git/plain/setups/->$THIS_DIR/eclipse-setups/"

rm -r "$TEMPDIR"

echo
echo "The OpTiMSoC IDE is now ready to be used."
echo "You can launch it from the the start menu by searching for 'OpTiMSoC IDE'."
echo "Enjoy!"
