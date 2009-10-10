install( PROGRAMS 
            digitaglinktree 
            cleanup_digikamdb 
         DESTINATION ${BIN_INSTALL_DIR} )

install( FILES 
            digitaglinktree.1 
            cleanup_digikamdb.1 
         DESTINATION ${MAN_INSTALL_DIR}/man1 )

