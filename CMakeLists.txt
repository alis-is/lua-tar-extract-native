cmake_minimum_required (VERSION 2.6)
project (ltar)

file(GLOB ltar_sources ./src/**.c)
SET(ltar ${ltar_sources})

ADD_LIBRARY (ltar ${ltar})
TARGET_LINK_LIBRARIES (ltar)