#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

install(FILES main.html
        DESTINATION ${DATA_INSTALL_DIR}/digikam/about
)

install(DIRECTORY css/
        DESTINATION ${DATA_INSTALL_DIR}/digikam/about/css
)

install(DIRECTORY images/
        DESTINATION ${DATA_INSTALL_DIR}/digikam/about/images
)

install(DIRECTORY js/
        DESTINATION ${DATA_INSTALL_DIR}/digikam/about/js
)
