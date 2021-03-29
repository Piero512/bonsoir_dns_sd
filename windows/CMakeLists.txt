cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "bonsoir_dns_sd")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "bonsoir_dns_sd_plugin")

add_library(${PLUGIN_NAME} SHARED
  "bonsoir_dns_sd_plugin.cpp"
  "service/service.cpp"
  "utils.cpp"
  "actions/action.cpp"
  "actions/broadcast.cpp"
  "actions/discovery.cpp"
)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Bonjour)
if(NOT BONJOUR_FOUND)
  message(FATAL_ERROR "Bonjour was not found! Please install Bonjour SDK!")
else()
  message("Bonjour was found at ${BONJOUR_LIBRARIES}")
  message("Bonjour has this include dirs: ${BONJOUR_INCLUDE_DIR}")
endif()
apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include" )
target_include_directories(${PLUGIN_NAME} PUBLIC ${BONJOUR_INCLUDE_DIR})
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin ${BONJOUR_LIBRARIES})

# List of absolute paths to libraries that should be bundled with the plugin
set(bonsoir_dns_sd_bundled_libraries
  ""
  PARENT_SCOPE
)
