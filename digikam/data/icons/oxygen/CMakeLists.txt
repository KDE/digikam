
set(oxygen_icon_dirs 128x128 16x16 22x22 32x32 48x48 64x64 scalable)

install(DIRECTORY ${oxygen_icon_dirs} DESTINATION ${ICON_INSTALL_DIR}/oxygen PATTERN .svn EXCLUDE)
