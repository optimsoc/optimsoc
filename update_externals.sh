#!/bin/bash
#
# Update all external projects (inside external) from their respective upstream
#

# we need a clean working directory for subtree
git stash

# lisnoc
git subtree pull --prefix external/lisnoc https://github.com/tum-lis/lisnoc.git master --squash

# glip
git subtree pull --prefix external/glip https://github.com/tum-lis/glip.git master --squash

# extra_cores: additional shared cores (by wallento)
git subtree pull --prefix external/extra_cores https://github.com/wallento/fusesoc_cores.git master --squash

# fusesoc: FuseSoC
git subtree pull --prefix external/fusesoc https://github.com/wallento/fusesoc.git dev --squash

# opensocdebug/software: OSD software
git subtree pull --prefix external/opensocdebug/software https://github.com/opensocdebug/software.git master --squash

# opensocdebug/hardware: OSD hardware
git subtree pull --prefix external/opensocdebug/hardware https://github.com/opensocdebug/hardware.git fusesoc --squash

# reapply our changes to the working directory
git stash pop
