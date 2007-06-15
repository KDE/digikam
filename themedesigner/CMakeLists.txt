
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(digikamthemedesigner_SRCS 
   main.cpp 
   mainwindow.cpp 
   themedicongroupitem.cpp 
   themediconitem.cpp 
   themediconview.cpp )

kde4_automoc(${digikamthemedesigner_SRCS})

kde4_add_executable(digikamthemedesigner ${digikamthemedesigner_SRCS})

target_link_libraries(digikamthemedesigner  ${KDE4_KDEUI_LIBS} digikam ${KDE4_KUTILS_LIBS} )

install(TARGETS digikamthemedesigner DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  digikamthemedesigner.desktop DESTINATION ${XDG_APPS_DIR} )
