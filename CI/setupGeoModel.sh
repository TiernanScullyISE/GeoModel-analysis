#!/bin/bash

echo "BUILD_DIR=\"${CI_PROJECT_DIR}/../build\""
echo "INSTALL_DIR=\"${CI_PROJECT_DIR}/install\""

export BUILD_DIR="${CI_PROJECT_DIR}/../build"
export INSTALL_DIR="${CI_PROJECT_DIR}/install"

export LD_LIBRARY_PATH="${INSTALL_DIR}/lib:${LD_LIBRARY_PATH}" 
export ROOT_INCLUDE_PATH="${INSTALL_DIR}/include:${ROOT_INCLUDE_PATH}"
export PATH="${INSTALL_DIR}/bin:${PATH}"

echo "export LD_LIBRARY_PATH=\"${INSTALL_DIR}/lib:${LD_LIBRARY_PATH}\""
echo "export ROOT_INCLUDE_PATH=\"${INSTALL_DIR}/include:${ROOT_INCLUDE_PATH}\""
echo "export PATH=\"${INSTALL_DIR}/bin:${PATH}\""
