install( PROGRAMS 
            digitaglinktree_new/digitaglinktree 
            cleanup_digikamdb_new/cleanup_digikamdb 
         DESTINATION ${BIN_INSTALL_DIR} )

install( FILES 
            digitaglinktree_new/digitaglinktree.1 
            cleanup_digikamdb_new/cleanup_digikamdb.1 
         DESTINATION ${MAN_INSTALL_DIR}/man1 )

