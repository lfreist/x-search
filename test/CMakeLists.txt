configure_file(files/dummy.txt files/dummy.txt COPYONLY)
configure_file(files/dummy.xs.meta files/dummy.xs.meta COPYONLY)
configure_file(files/dummy.xslz4 files/dummy.xslz4 COPYONLY)
configure_file(files/dummy.xslz4.meta files/dummy.xslz4.meta COPYONLY)
configure_file(files/dummy.xszst files/dummy.xszst COPYONLY)
configure_file(files/dummy.xszst.meta files/dummy.xszst.meta COPYONLY)

add_subdirectory(src)