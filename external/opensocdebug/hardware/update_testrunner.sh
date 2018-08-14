#!/bin/bash
#
# Update the cocotb_testrunner
#

# Delete old testrunner files
find test/cocotb -maxdepth 1 -type f -exec rm -f {} \;

# Clone testrunner repository
git clone --depth=1 https://github.com/imphil/cocotb-testrunner.git tmp

# Delete repository and copy files to test/cocotb
rm -rf tmp/.git
mv tmp/* test/cocotb/
rmdir tmp

echo "cocotb_testrunner is now up to date"
