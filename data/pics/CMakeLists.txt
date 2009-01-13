# digiKam files ######################################################

INSTALL(FILES tag.png
              tag-new.png
              tag-properties.png
              tag-delete.png
              tag-reset.png
              tag-folder.png
              tag-events.png
              tag-people.png
              tag-places.png
              tag-addressbook.png
              tag-recents.png
              tag-assigned.png
              albumfolder-importimages.png 
              albumfolder-properties.png
              albumfolder-new.png
              albumfolder-importdir.png
              underexposure.png
              overexposure.png
        DESTINATION ${DATA_INSTALL_DIR}/digikam/icons/hicolor/32x32/actions
       )

INSTALL(FILES image-full.png
              image-selection.png
              histogram-lin.png
              histogram-log.png
              banner-digikam.png
              banner-showfoto.png
              logo-digikam.png
              logo-gphoto.png
              logo-lcms.png
              logo-exiv2.png
              logo-cimg.png
              centerwidth.png
              centerheight.png
              target.png
              bothvert.png
              bothhorz.png
              curvefree.png
              curvemooth.png
              original.png
              duplicatebothvert.png
              duplicatebothhorz.png
              togglemouseover.png
              indicator-gray.png
              indicator-green.png
              indicator-red.png
              menusidepix-digikam.png
              splash-digikam.png
         DESTINATION ${DATA_INSTALL_DIR}/digikam/data 
       )

# Showfoto files ######################################################

INSTALL(FILES logo-showfoto.png 
              menusidepix-showfoto.png
              splash-showfoto.png 
        DESTINATION ${DATA_INSTALL_DIR}/showfoto/data)
