PROJECT(digikam)

ADD_DEFINITIONS(${QT_DEFINITIONS}
                ${KDE4_DEFINITIONS} 
                ${KDCRAW_DEFINITIONS} 
                ${KEXIV2_DEFINITIONS}
               )

INCLUDE_DIRECTORIES(${KEXIV2_INCLUDE_DIR} 
                    ${KDCRAW_INCLUDE_DIR}
                    ${KDE4_INCLUDE_DIR}
                    ${QT_INCLUDES}
                   )

ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( libs )
ADD_SUBDIRECTORY( utilities )
#ADD_SUBDIRECTORY( digikam )
#ADD_SUBDIRECTORY( showfoto )
ADD_SUBDIRECTORY( kioslave )
#ADD_SUBDIRECTORY( imageplugins )
#ADD_SUBDIRECTORY( themedesigner )

install( FILES tips DESTINATION ${DATA_INSTALL_DIR}/digikam )


