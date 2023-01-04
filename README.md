# idf-flash-vendor-patches

This repository is for some vendor specific flash patches that cannot be directly integrated into IDF.

Currently there is patch for:

1. XMC SR issue (See Readme of Example for the supported targets and branches)


# Adding component to your project

You can use one of the following ways to include the component into your project

## Using `EXTRA_COMPONENT_DIRS`

1. Clone this repository to somewhere in your PC. For example `VENDOR_PATCHES=~/my_projects/idf-flash-vendor-patches`
2. Add following lines into your project CMakeLists.txt file. For examples

```
cmake_minimum_required(VERSION 3.16)

# Add these two lines
set(VENDOR_PATCHES ~/my_projects/idf-flash-vendor-patches)
set(EXTRA_COMPONENT_DIRS ${VENDOR_PATCHES}/components/idf-flash-vendor-patches)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hello_world)
```

3. Build your project

## Using IDF component manager

Supported in IDF v4.2.4+, v4.3.3+, v4.4 and later.

Add `idf_component.yml` file to one of your project's components (suggest to the component where this patch is included and called), with the following contents:

```
dependencies:
  idf-flash-vendor-patches:
    path: components/idf-flash-vendor-patches
    git: https://github.com/espressif/idf-flash-vendor-patches.git
```
