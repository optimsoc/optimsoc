#!/bin/bash
#
# Output the OpTiMSoC version number of the current source tree
#
# The script tries to give the version number as accurate as possible to 
# enable others to reproduce the exact same state of the source code.
#
# Rules: 
# - If the repository state equals a tagged release version, return that
#   version number.
# - If modifications have been made on top of the released version, the
#   version number is appended with "-git<COMMITID>"
# - If the working directory has local modifications, it is further appended
#   with a "+" sign.
# - If no tagged release can be found, only the commit ID is returned.
# - Finally, if the code is not inside a git repository, "unknown" is returned.
#

# get the most recent git version tag
GIT_LATEST_TAG=$(git describe --abbrev=0 --match 'v*' 2>/dev/null)
if [ $? -eq 0 ]; then
    # we're inside a git repository and at least one annotated tag was found
    GIT_LATEST_VERSION=$(echo $GIT_LATEST_TAG | tr -d 'v')

    git describe --exact-match --match 'v*' >/dev/null 2>&1 && IS_RELEASE=1
    if [ $IS_RELEASE ]; then
        # if the current commit equals the version tag we have a release version
        echo $GIT_LATEST_VERSION
        exit
    else
        # otherwise we have modifications on top of the release
        GIT_COMMIT=$(git rev-parse --short=12 HEAD)

        # if the working directory has local modifications, we append an 
        # "+" to the end of the commit hash
        if ! git diff-index --quiet HEAD --; then
            echo ${GIT_LATEST_VERSION}-git${GIT_COMMIT}+
        else
            echo ${GIT_LATEST_VERSION}-git${GIT_COMMIT}
        fi
        exit
    fi
else
    # If we don't have a tag, let's try the latest revision
    GIT_COMMIT=$(git rev-parse --short=12 HEAD)
    if [ $? -eq 0 ]; then
        echo git$GIT_COMMIT
        exit
    fi
fi

# We were unable to determine a version number, all checks failed
echo "unknown"
exit 255

