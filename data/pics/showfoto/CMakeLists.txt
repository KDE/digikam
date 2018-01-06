#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

file(GLOB _files *.png)

install(FILES       ${_files}
        DESTINATION ${DATA_INSTALL_DIR}/showfoto/data
)
