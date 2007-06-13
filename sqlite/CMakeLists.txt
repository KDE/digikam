INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(sqlite_STAT_SRCS
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

KDE3_AUTOMOC(${sqlite_STAT_SRCS})

ADD_LIBRARY(sqlite STATIC ${sqlite_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

##stolen Makefile.am from amarok
#
#noinst_LTLIBRARIES = libsqlite.la
#
#INCLUDES = $(all_includes)
#    
#libsqlite_la_CFLAGS = -w
#
#libsqlite_la_LDFLAGS = $(LIBPTHREAD)
#  
#libsqlite_la_SOURCES = \
#    attach.c \
#    auth.c \
#    btree.c \
#    btree_rb.c \
#    build.c \
#    copy.c \
#    date.c \
#    delete.c \
#    encode.c \
#    expr.c \
#    func.c \
#    hash.c \
#    insert.c \
#    main.c \
#    opcodes.c \
#    os.c \
#    pager.c \
#    parse.c \
#    pragma.c \
#    printf.c \
#    random.c \
#    select.c \
#    shell.c \
#    table.c \
#    tokenize.c \
#    trigger.c \
#    update.c \
#    util.c \
#    vacuum.c \
#    vdbe.c \
#    vdbeaux.c \
#    where.c
