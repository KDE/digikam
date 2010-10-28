
ADD_SUBDIRECTORY( filters )

INSTALL( FILES dimg.h 
               dcolor.h 
               dcolorpixelaccess.h 
               dcolorcomposer.h 
               dcolorblend.h 
               imagehistory.h/dimagehistory.h
               imagehistory.h/historyimageid.h
               imagehistory.h/filteraction.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam COMPONENT Devel 
       )
