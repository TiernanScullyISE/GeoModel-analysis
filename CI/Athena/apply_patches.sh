#!/bin/bash
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}"  )" &> /dev/null && pwd  )

columns=80
if [ -t 0  ]; then
    columns="$(tput cols)"
fi

function fill_line() {
    sep=${1:-=}
    printf "=%.0s" $(seq 1 "$columns") | tr "=" "$sep"
    echo ""
}

source $SCRIPT_DIR/find_nightly_commit.sh

fill_line "-"
echo "Latest nightly for '${ATHENA_RELEASE}' was created at ${NIGHTLY_DATE}"
echo "Last commit before nightly determined to be $NIGHTLY_COMMIT"
echo "---- Now at ----"
git --no-pager log -1 $NIGHTLY_COMMIT
fill_line "-"

git checkout $NIGHTLY_COMMIT

if compgen -G "${SCRIPT_DIR}/patches/*.patch" > /dev/null; then
    for p in "$SCRIPT_DIR"/patches/*.patch; do
        echo "---- Applying ${p} ----"
        patch -p1 < ${p}
        #cat "${p}" | git am -3
        #git --no-pager log -1 $sha

        fill_line "-"
    done
fi


#git diff HEAD ${NIGHTLY_COMMIT}
git diff
