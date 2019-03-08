#!/bin/bash
#
# Update all external projects (inside external) from their respective upstream
#

# only run git stash if the working directory actually has changes; otherwise
# we stash nothing and stash pop old stuff.
WD_HAS_CHANGES=$(git diff-index --quiet HEAD --; echo $?)

# we need a clean working directory for subtree
[ $WD_HAS_CHANGES == 1 ] && git stash

# glip
git subtree pull -m "Update external/glip" --prefix external/glip https://github.com/tum-lis/glip.git master --squash

# mor1kx
git subtree pull -m "Update external/mor1kx" --prefix external/mor1kx https://github.com/openrisc/mor1kx.git master --squash

# opensocdebug/software: OSD software
git subtree pull -m "Update external/opensocdebug/osd-sw" --prefix external/opensocdebug/software https://github.com/opensocdebug/osd-sw.git master --squash

# opensocdebug/hardware: OSD hardware
git subtree pull -m "Update external/opensocdebug/osd-hw" --prefix external/opensocdebug/hardware https://github.com/opensocdebug/osd-hw.git master --squash

# reapply our changes to the working directory
[ $WD_HAS_CHANGES == 1 ] && git stash pop

