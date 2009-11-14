install( PROGRAMS 
            digitaglinktree/digitaglinktree 
            cleanup_digikamdb/cleanup_digikamdb 
         DESTINATION ${BIN_INSTALL_DIR} )

install( FILES 
            digitaglinktree/digitaglinktree.1 
            cleanup_digikamdb/cleanup_digikamdb.1 
         DESTINATION ${MAN_INSTALL_DIR}/man1 )

