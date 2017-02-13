#!/bin/bash
#
# Update all external projects (inside external) from their respective upstream
#

# only run git stash if the working directory actually has changes; otherwise
# we stash nothing and stash pop old stuff.
WD_HAS_CHANGES=$(git diff-index --quiet HEAD --; echo $?)

# we need a clean working directory for subtree
[ $WD_HAS_CHANGES == 1 ] && git stash

# lisnoc
git subtree pull -m "Update external/lisnoc" --prefix external/lisnoc https://github.com/tum-lis/lisnoc.git master --squash

# glip
git subtree pull -m "Update external/glip" --prefix external/glip https://github.com/tum-lis/glip.git master --squash

# extra_cores: additional shared cores (by wallento)
git subtree pull -m "Update external/extra_cores" --prefix external/extra_cores https://github.com/wallento/fusesoc_cores.git master --squash

# fusesoc: FuseSoC
git subtree pull -m "Update external/fusesoc" --prefix external/fusesoc https://github.com/wallento/fusesoc.git dev --squash

# fusesoc-ipyxact: Fusesoc dependency
git subtree pull -m "Update external/fusesoc-ipyxact" --prefix external/fusesoc-ipyxact https://github.com/olofk/ipyxact.git master --squash

# opensocdebug/software: OSD software
git subtree pull -m "Update external/opensocdebug/software" --prefix external/opensocdebug/software https://github.com/opensocdebug/software.git master --squash

# opensocdebug/hardware: OSD hardware
git subtree pull -m "Update external/opensocdebug/hardware" --prefix external/opensocdebug/hardware https://github.com/opensocdebug/hardware.git mam-sync --squash

# reapply our changes to the working directory
[ $WD_HAS_CHANGES == 1 ] && git stash pop

