configure_file(files/dummy.txt files/dummy.txt COPYONLY)
configure_file(files/dummy.txt.sf.meta files/dummy.txt.sf.meta COPYONLY)
configure_file(files/dummy.txt.sflz4 files/dummy.txt.sflz4 COPYONLY)
configure_file(files/dummy.txt.sflz4.meta files/dummy.txt.sflz4.meta COPYONLY)
configure_file(files/dummy.txt.sfzst files/dummy.txt.sfzst COPYONLY)
configure_file(files/dummy.txt.sfzst.meta files/dummy.txt.sfzst.meta COPYONLY)

configure_file(test_sfgrep.sh test_sfgrep.sh COPYONLY)

add_subdirectory(src)