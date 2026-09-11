cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(VitaTester VERSION 1.5.2 LANGUAGES C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "VitaTester")
set(VITA_TITLEID  "VITATESTR")
set(VITA_VERSION  "01.52")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -O2 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB VT_MODULE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/module_*.c")
set(VT_REGISTRY "#include \"module.h\"\n")
set(VT_ENTRIES "")
foreach(source IN LISTS VT_MODULE_SOURCES)
  get_filename_component(name "${source}" NAME_WE)
  string(APPEND VT_REGISTRY "extern const VtModule vt_${name};\n")
  string(APPEND VT_ENTRIES "&vt_${name},")
endforeach()
string(APPEND VT_REGISTRY "const VtModule *const vt_modules[]={${VT_ENTRIES}};\nconst unsigned vt_module_count=sizeof(vt_modules)/sizeof(vt_modules[0]);\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/module_registry.c" "${VT_REGISTRY}")
include_directories(src)

add_executable(VitaTester
  ${VT_MODULE_SOURCES}
  ${CMAKE_CURRENT_BINARY_DIR}/module_registry.c
  src/original.c
  src/navigation.c
  src/scan_model.c
  src/scanner.c
  src/main.c
  src/font.c
  src/capture.c
  src/ui.c
  src/diagnostic.c
  src/logger.c
  src/queue.c
  src/safety.c
  src/workers.c
)

target_link_libraries(VitaTester
  vita2d
  SceMotion_stub
  SceAudio_stub
  SceAudioIn_stub
  SceCamera_stub
  SceNet_stub
  SceNetCtl_stub
  SceKernelModulemgr_stub
  SceSysmem_stub
  SceVshBridge_stub
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  ScePower_stub
  SceAppMgr_stub
  SceIofilemgr_stub
  SceKernelThreadMgr_stub
  SceProcessmgr_stub
  SceCtrl_stub
  SceTouch_stub
  SceCommonDialog_stub
  freetype
  bz2
  png
  z
  m
)

vita_create_self(VitaTester.self VitaTester)
vita_create_vpk(VitaTester.vpk ${VITA_TITLEID} VitaTester.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE icons/analog.png icons/analog.png
  FILE icons/background.png icons/background.png
  FILE icons/circle.png icons/circle.png
  FILE icons/cross.png icons/cross.png
  FILE icons/dpad.png icons/dpad.png
  FILE icons/finger_blue.png icons/finger_blue.png
  FILE icons/finger_gray.png icons/finger_gray.png
  FILE icons/ltrigger.png icons/ltrigger.png
  FILE icons/rtrigger.png icons/rtrigger.png
  FILE icons/select.png icons/select.png
  FILE icons/square.png icons/square.png
  FILE icons/start.png icons/start.png
  FILE icons/triangle.png icons/triangle.png
)
