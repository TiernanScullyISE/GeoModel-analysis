#!/bin/bash

echo "Compile GeoModelCore with flags: "
echo "   **** CMAKE_CONFIG_FLAGS: ${CMAKE_CONFIG_FLAGS}"
echo "   **** CMAKE_EXTRA_FLAGS: ${CMAKE_EXTRA_FLAGS}"

if [ -z "${SOURCE_DIR}" ]; then
    SOURCE_DIR="${PWD}"
fi

echo "SOURCE_DIR=\"${SOURCE_DIR}\""
echo "BUILD_DIR=\"${CI_PROJECT_DIR}/../build\""
echo "INSTALL_DIR=\"${CI_PROJECT_DIR}/install\""

BUILD_DIR="${CI_PROJECT_DIR}/../build"
INSTALL_DIR="${CI_PROJECT_DIR}/install"

mkdir -p ${BUILD_DIR}
mkdir -p ${INSTALL_DIR}
ls -lh ${INSTALL_DIR}
echo "mkdir -p ${BUILD_DIR}"
echo "mkdir -p ${INSTALL_DIR}"

cd ${BUILD_DIR}
echo "cd ${BUILD_DIR}"

export LD_LIBRARY_PATH="${INSTALL_DIR}/lib:${LD_LIBRARY_PATH}" 
export ROOT_INCLUDE_PATH="${INSTALL_DIR}/include:${ROOT_INCLUDE_PATH}"
export PATH="${INSTALL_DIR}/bin:${PATH}"

echo "export LD_LIBRARY_PATH=\"${INSTALL_DIR}/lib:${LD_LIBRARY_PATH}\""
echo "export ROOT_INCLUDE_PATH=\"${INSTALL_DIR}/include:${ROOT_INCLUDE_PATH}\""
echo "export PATH=\"${INSTALL_DIR}/bin:${PATH}\""


echo "cmake --fresh ${CMAKE_CONFIG_FLAGS} \
    -DCMAKE_CXX_FLAGS=\"${CMAKE_EXTRA_FLAGS}\" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
     ${SOURCE_DIR}"

cmake --fresh ${CMAKE_CONFIG_FLAGS} \
    -DCMAKE_CXX_FLAGS="${CMAKE_EXTRA_FLAGS}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
     ${SOURCE_DIR}

result=$?
if [ ${result} -ne 0 ];then 
   echo "Cmake failed"
   exit 1
fi



make -j4

result=$?
if [ ${result} -ne 0 ];then 
   echo "Compilation failed"
   exit 1
fi

make install

result=$?
if [ ${result} -ne 0 ];then 
   echo "Installation failed"
   exit 1
fi

ls -lh ${INSTALL_DIR}

