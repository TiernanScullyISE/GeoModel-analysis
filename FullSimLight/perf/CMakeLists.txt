
project(FullSimLightPerf)

include(CTest)

find_program(PERF_EXECUTABLE NAMES perf)

if(NOT EXISTS ${PERF_EXECUTABLE})
  message(FATAL_ERROR "Could not find perf program")
endif()

if(TARGET fullSimLight)
  set(FULLSIMLIGHT $<TARGET_FILE:fullSimLight>)
else()
  if(DEFINED FULLSIMLIGHT)
    unset(FULLSIMLIGHT)
    unset(FULLSIMLIGHT CACHE)
  endif()
  find_program(FULLSIMLIGHT NAMES fullSimLight)
  if(NOT EXISTS ${FULLSIMLIGHT})
    message(FATAL_ERROR "Could not find fullSimLight program")
  endif()
endif()

unset(GEANT4_CONFIG CACHE)
find_program(GEANT4_CONFIG NAMES geant4-config HINTS ${Geant4_DIR})

if(NOT EXISTS ${GEANT4_CONFIG})
  message(FATAL_ERROR "Could not find geant4-config script")
endif()

execute_process(COMMAND sh -c "${GEANT4_CONFIG} --datasets | awk '{ printf \"%s=%s;\", \$2, \$3 }'"
  OUTPUT_VARIABLE GEANT4_DATASETS OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT DEFINED NCORES)
  cmake_host_system_information(RESULT NCORES QUERY NUMBER_OF_PHYSICAL_CORES)
endif()

if(NOT DEFINED EVENTS_PER_CORE)
  set(EVENTS_PER_CORE 4)
endif()

if(NOT DEFINED EVENTS)
  math(EXPR EVENTS "${EVENTS_PER_CORE} * ${NCORES}")
endif()

if(NOT DEFINED GEOMETRY)
  set(GEOMETRY geometry-ATLAS-R2-2016-01-02-01_with_EMEC_Wheel.db)
endif()

if(IS_ABSOLUTE ${GEOMETRY})
  if(NOT EXISTS ${GEOMETRY})
    message(FATAL_ERROR "Geometry file does not exist: ${GEOMETRY}")
  endif()
else()
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${GEOMETRY})
    file(DOWNLOAD https://gitlab.cern.ch/GeoModelATLAS/geometry-data/-/raw/master/geometry/${GEOMETRY}
      ${CMAKE_CURRENT_BINARY_DIR}/${GEOMETRY} STATUS GEOMETRY_DOWNLOADED)
    if(NOT GEOMETRY_DOWNLOADED)
      message(FATAL_ERROR "Failed to download ${GEOMETRY}")
    endif()
  endif()
endif()

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/bmagatlas_09_fullAsym20400.data)
  set(FIELD "/mydet/setField 4.0 tesla")
endif()

configure_file(perf.g4.in perf.g4)

if(NOT DEFINED MACRO)
  set(MACRO ${CMAKE_CURRENT_BINARY_DIR}/perf.g4)
endif()

if(NOT EXISTS ${MACRO})
  message(FATAL_ERROR "Macro file does not exist: ${MACRO}")
endif()

if(NOT DEFINED METRICS_CPU)
  set(METRICS_CPU {cycles,instructions,branches,branch-misses})
endif()

if(NOT DEFINED METRICS_CACHE)
  set(METRICS_CACHE {instructions,L1-dcache-loads,L1-dcache-load-misses,L1-icache-load-misses})
endif()

set(TEST_NAME fullsimlight)
set(TEST_COMMAND ${FULLSIMLIGHT} -p -P ttbar -g ${GEOMETRY} -m ${MACRO})

add_test(NAME ${TEST_NAME}-record-cpu
  COMMAND ${PERF_EXECUTABLE} record -e ${METRICS_CPU} -o ${TEST_NAME}-cpu.perf ${TEST_COMMAND})

add_test(NAME ${TEST_NAME}-record-cache
  COMMAND ${PERF_EXECUTABLE} record -e ${METRICS_CACHE} -o ${TEST_NAME}-cache.perf ${TEST_COMMAND})

add_test(NAME ${TEST_NAME}-record-callgraph
  COMMAND ${PERF_EXECUTABLE} record -e cycles -g -F100 -o ${TEST_NAME}.perf ${TEST_COMMAND})

foreach(METRIC cpu cache callgraph)
  set_tests_properties(${TEST_NAME}-record-${METRIC}
    PROPERTIES LABELS "perf;record" ENVIRONMENT "${GEANT4_DATASETS}" PROCESSORS ${NCORES} RUN_SERIAL TRUE)
endforeach()

add_test(NAME ${TEST_NAME}-report
  COMMAND sh -c "${CMAKE_CURRENT_SOURCE_DIR}/scripts/report.sh ${TEST_NAME} | tee ${TEST_NAME}.txt")
set_tests_properties(${TEST_NAME}-report PROPERTIES LABELS "perf;report"
  ATTACHED_FILES "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.svg"
  REQUIRED_FILES "${TEST_NAME}.perf;${TEST_NAME}-cpu.perf;${TEST_NAME}-cache.perf"
  DEPENDS "${TEST_NAME}-record-cpu;${TEST_NAME}-record-cache;${TEST_NAME}-record-callgraph")
