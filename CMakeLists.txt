cmake_minimum_required (VERSION 2.6)
project (eli_tar)

file(GLOB eli_tar_sources ./src/**.c)
SET(eli_tar ${eli_tar_sources})

ADD_LIBRARY (eli_tar ${eli_tar})
TARGET_LINK_LIBRARIES (eli_tar)