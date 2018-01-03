#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

install(PROGRAMS digitaglinktree/digitaglinktree
                 cleanup_digikamdb/cleanup_digikamdb
        DESTINATION ${BIN_INSTALL_DIR}
)

install(FILES digitaglinktree/digitaglinktree.1
              cleanup_digikamdb/cleanup_digikamdb.1
        DESTINATION ${MAN_INSTALL_DIR}/man1
)

