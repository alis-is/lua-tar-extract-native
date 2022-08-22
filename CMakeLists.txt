project (ltar)

file(GLOB ltar_sources ./src/**.c)
set(ltar ${ltar_sources})

add_library(ltar ${ltar})
target_link_libraries(ltar)