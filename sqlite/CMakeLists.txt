INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} 
                     ${CMAKE_CURRENT_BINARY_DIR}) 

ADD_DEFINITIONS("-w")

SET(libsqlite2_STAT_SRCS
    attach.c
    auth.c
    btree.c
    btree_rb.c
    build.c
    copy.c
    date.c
    delete.c
    encode.c
    expr.c
    func.c
    hash.c
    insert.c
    main.c
    opcodes.c
    os.c
    pager.c
    parse.c
    pragma.c
    printf.c
    random.c
    select.c
    shell.c
    table.c
    tokenize.c
    trigger.c
    update.c
    util.c
    vacuum.c
    vdbe.c
    vdbeaux.c
    where.c
   )

TARGET_LINK_LIBRARIES(libsqlite2 ${LIBPTHREAD})

KDE4_ADD_LIBRARY(libsqlite2 STATIC ${libsqlite2_STAT_SRCS})
