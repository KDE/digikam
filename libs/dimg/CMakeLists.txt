
ADD_SUBDIRECTORY( filters )

INSTALL( FILES dimg.h 
               dcolor.h 
               dcolorpixelaccess.h 
               dcolorcomposer.h 
               dcolorblend.h 
               imagehistory/dimagehistory.h
               imagehistory/historyimageid.h
               imagehistory/filteraction.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam COMPONENT Devel 
       )
