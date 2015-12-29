#!/bin/bash -e

if [[ $# < 1 ]]; then
    echo "Usage: pack-dist.sh <version>"
fi

VERSION=$1

echo "Pack base"
tar -czf optimsoc-$VERSION-base.tgz --exclude $VERSION/examples $VERSION
echo "Pack examples"
tar -czf optimsoc-$VERSION-examples.tgz $VERSION/examples
