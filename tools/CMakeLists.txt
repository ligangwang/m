cmake_minimum_required(VERSION 3.16)

include(GoogleTest)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM BIN ${LLVM_TOOLS_BINARY_DIR}")
execute_process(COMMAND llvm-config --libnames
                OUTPUT_VARIABLE llvm_libraries OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --system-libs
                OUTPUT_VARIABLE sys_libraries OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " -llibxml2.tbd" "" sys_libraries ${sys_libraries})
string(REPLACE " " ";" llvm_libraries ${llvm_libraries})
string(REPLACE " " ";" sys_libraries ${sys_libraries})
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Found LLVM libraries ${llvm_libraries}")
message(STATUS "Found LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found SYS libraries ${sys_libraries}") 
if(WIN32)
  set(clang_lib libclang)
  message(STATUS, "Found Windows Kit: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
  message(STATUS, "Found Windows Platform: ${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
else()
  set(clang_lib clang)
endif(WIN32)


include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(
  ${CMAKE_SOURCE_DIR}/include
)

link_directories(
  ${LLVM_INCLUDE_DIRS}/../lib
)

add_executable(m
  driver/ld.cc
  driver/m.cc
)

add_executable(c2m
  c2m/c2m.c
)

set_target_properties(m PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(m PRIVATE 
  mlr
  lldDriver
  lldCOFF
  lldELF
  lldMachO
  lldMachO2
  lldMinGW
  lldWasm
  lldYAML
  lldReaderWriter
  lldCommon
  lldCore
  ${llvm_libraries}
  ${sys_libraries}
)

message(STATUS "${CMAKE_BUILD_TYPE} COPY ${CMAKE_SOURCE_DIR}/mlib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mlib")

if(WIN32)
add_custom_command(
  TARGET m PRE_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/mlib" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/mlib"
)
else()
file(COPY ${CMAKE_SOURCE_DIR}/mlib/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mlib/)
endif(WIN32)

add_custom_command(
  TARGET m
  POST_BUILD
  COMMAND m ${CMAKE_SOURCE_DIR}/samples/sample_lib.m
  COMMAND m ${CMAKE_SOURCE_DIR}/samples/sample.m
  COMMAND m ${CMAKE_SOURCE_DIR}/samples/helloworld.m
  # MacOS COMMAND ld ${CMAKE_CURRENT_SOURCE_DIR}/sample.o ${CMAKE_CURRENT_SOURCE_DIR}/../build/src/libmlr.a /usr/lib/libc.dylib -o ${CMAKE_CURRENT_SOURCE_DIR}/sample
  # COMMAND ld ${CMAKE_CURRENT_SOURCE_DIR}/sample.o ${CMAKE_CURRENT_SOURCE_DIR}/../build/src/libmlr.a /usr/lib/x86_64-linux-gnu/libc.so -o ${CMAKE_CURRENT_SOURCE_DIR}/sample
)


target_link_libraries(c2m PRIVATE 
  mlr 
  ${clang_lib}
  ${sys_libraries}
)
