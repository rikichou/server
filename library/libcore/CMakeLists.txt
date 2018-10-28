# include all source file
aux_source_directory(. DIR_LIBCORE_SRCS)

# library
add_library (core ${DIR_LIBCORE_SRCS})