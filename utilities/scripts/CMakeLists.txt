INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES(  FILES  digitaglinktree )



#original Makefile.am contents follow:

######## This script name should probably be more 'namespaced'. Think about distros putting everything in /usr/bin...
#bin_SCRIPTS = digitaglinktree
#man_MANS = digitaglinktree.1
#
