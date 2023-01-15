configure_file(files/dummy.txt files/dummy.txt COPYONLY)
configure_file(files/dummy.sf.meta files/dummy.sf.meta COPYONLY)
configure_file(files/dummy.sflz4 files/dummy.sflz4 COPYONLY)
configure_file(files/dummy.sflz4.meta files/dummy.sflz4.meta COPYONLY)
configure_file(files/dummy.sfzst files/dummy.sfzst COPYONLY)
configure_file(files/dummy.sfzst.meta files/dumm.sfzst.meta COPYONLY)

configure_file(test_sfgrep.sh test_sfgrep.sh COPYONLY)

add_subdirectory(src)