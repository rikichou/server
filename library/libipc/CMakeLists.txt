# include all source file
aux_source_directory(. DIR_LIBIPC_SRCS)

# library
add_library (ipc ${DIR_LIBIPC_SRCS})