export NIGHTLY_DATE=$(ls /cvmfs/atlas-nightlies.cern.ch/repo/sw/${ATHENA_RELEASE}_Athena_${LCG_PLATFORM} | grep -e "^[0-9]\+" | tail -1 | sed -E 's/(.*)T([0-9]{2})([0-9]{2})/\1T\2:\3:00/g')
export NIGHTLY_COMMIT=$(git log --until="$NIGHTLY_DATE" -1 --first-parent --format=format:%H)
