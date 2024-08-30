cmake_minimum_required(VERSION 2.6.4)
project(CFE_SAMPLE_TALKER C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)
# Add dependency to the racs2_bridge_client app (this environment variable is generated at the top level)
include_directories(${racs2_bridge_client_MISSION_DIR}/fsw/platform_inc)
# In cFS 7.0 there is a cmake function 'add_cfe_app_dependency' for this

aux_source_directory(fsw/src APP_SRC_FILES)

# Create the app module
add_cfe_app(event_talker ${APP_SRC_FILES})


target_link_libraries(event_talker
  ${PROTOBUF_LIBRARY}
  protobuf-c
)
