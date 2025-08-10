#!/bin/bash

echo "mv build ../"
echo "cd ../build"
mv build ../
cd ../build

echo "ctest --verbose > ${CI_PROJECT_DIR}/CTest.log"
ctest --verbose > ${CI_PROJECT_DIR}/CTest.log 
exit_code=$?
echo "exit_code=${exit_code}"

cat ${CI_PROJECT_DIR}/CTest.log

echo "pwd"
echo "ls -l"

pwd
ls -l

echo "exit ${exit_code}"

exit ${exit_code}
