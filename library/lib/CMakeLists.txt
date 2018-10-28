# include all source file
aux_source_directory(. DIR_LIB_SRCS)

# library
add_library (common ${DIR_LIB_SRCS})