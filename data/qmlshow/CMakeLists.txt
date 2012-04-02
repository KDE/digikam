ADD_SUBDIRECTORY(def)
ADD_SUBDIRECTORY(onpress)

INSTALL(FILES cross.png
              edit_icon.png
              empty_star.png
              filled_star.png
              flag.png
              flag2.png
              menu.svg
              qmlview.qml
              mapview.html
        DESTINATION ${DATA_INSTALL_DIR}/digikam/qmlshow
       )
