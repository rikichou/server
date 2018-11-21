# cmake version
cmake_minimum_required (VERSION 2.8)

# project name
project (entryd)

# add include dir
include_directories(include library)
# define src dir
AUX_SOURCE_DIRECTORY(src DIR_SRCS)
AUX_SOURCE_DIRECTORY(src/device DIR_SRCS)

# --------------------------------library/libcommon-----------------------------------------#
# to make use of the library
include_directories ("${PROJECT_SOURCE_DIR}/library/lib")

# to let makefile build the library
add_subdirectory (library/lib) 
#---------------------------------end library/libcommon-------------------------------------#

# --------------------------------library/libcore-----------------------------------------#
# to make use of the library
include_directories ("${PROJECT_SOURCE_DIR}/library/libcore")

# to let makefile build the library
add_subdirectory (library/libcore) 
#---------------------------------end library/libcore-------------------------------------#

# --------------------------------library/libipc-----------------------------------------#
# to make use of the library
include_directories ("${PROJECT_SOURCE_DIR}/library/libipc")

# to let makefile build the library
add_subdirectory (library/libipc) 
#---------------------------------end library/libipc-------------------------------------#

# --------------------------------library/libcjson-----------------------------------------#
# to make use of the library
include_directories ("${PROJECT_SOURCE_DIR}/library/libcjson")

# to let makefile build the library
add_subdirectory (library/libcjson) 
#---------------------------------end library/libipc-------------------------------------#

# excutable name
add_executable(entryd ${DIR_SRCS})

# ---------------------------------link library------------------------------------#
target_link_libraries (entryd ipc common core cjson -lm)