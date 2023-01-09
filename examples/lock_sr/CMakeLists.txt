# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Add these two lines if IDF component manager not available for you
# set(VENDOR_PATCHES ~/my_projects/idf-flash-vendor-patches)
# set(EXTRA_COMPONENT_DIRS ${VENDOR_PATCHES}/components/idf-flash-vendor-patches)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(sr_lock)
