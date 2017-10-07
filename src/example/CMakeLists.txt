CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (SOURCE_FILES ./main.c ../fsme/src/fsme.c)

include_directories("${PROJECT_SOURCE_DIR}/fsme/header")

add_executable(imachine_example ${SOURCE_FILES})

