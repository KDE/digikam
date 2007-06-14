
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  digitaglinktree DESTINATION  )



#original Makefile.am contents follow:

######## This script name should probably be more 'namespaced'. Think about distros putting everything in /usr/bin...
#bin_SCRIPTS = digitaglinktree
#man_MANS = digitaglinktree.1
#
