#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(icons)
add_subdirectory(pics)
add_subdirectory(about)
add_subdirectory(database)
add_subdirectory(hotplug)
add_subdirectory(scripts)

install(FILES tips DESTINATION ${DATA_INSTALL_DIR}/digikam)
