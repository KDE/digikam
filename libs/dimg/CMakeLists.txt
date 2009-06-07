
ADD_SUBDIRECTORY( filters )

INSTALL( FILES dimg.h 
               dcolor.h 
               dcolorpixelaccess.h 
               dcolorcomposer.h 
               dcolorblend.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam COMPONENT Devel 
       )
