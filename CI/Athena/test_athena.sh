#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}"   )" &> /dev/null && pwd   )

export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase

export LCG_PLATFORM=x86_64-el9-gcc15-opt

export ATHENA_GIT_REPO=https://gitlab.cern.ch/atlas/athena.git
export ATHENA_RELEASE=main
export ATHENA_REF=main

_pwd=$PWD

tmp=/tmp/ath_build
rm -rf $tmp
mkdir -p $tmp
pushd $tmp
ln -s $SCRIPT_DIR/package_filters.txt package_filters.txt
$SCRIPT_DIR/athena_build.sh
popd
