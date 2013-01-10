ADD_SUBDIRECTORY(borderpatterns)
ADD_SUBDIRECTORY(texturepatterns)

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

INSTALL(FILES splash-digikam.png
              banner-digikam.png
              logo-digikam.png
              logo-gphoto.png
              logo-lcms.png
              logo-exiv2.png
              logo-cimg.png
              logo-beta.png
              image-full.png
              image-selection.png
              histogram-lin.png
              histogram-log.png
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
              assistant-collection.png
              assistant-rawimport.png
              assistant-tooltips.png
              assistant-metadata.png
              assistant-preview.png
              assistant-openfile.png
              assistant-scancollection.png
              colors-luma.png
              colors-chromablue.png
              colors-chromared.png
              sample-aix.png
         DESTINATION ${DATA_INSTALL_DIR}/digikam/data
       )

# Showfoto files ######################################################

INSTALL(FILES logo-showfoto.png
              banner-showfoto.png
              splash-showfoto.png
        DESTINATION ${DATA_INSTALL_DIR}/showfoto/data
       )

