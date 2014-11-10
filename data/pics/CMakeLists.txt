#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(borderpatterns)
add_subdirectory(texturepatterns)

# digiKam files ######################################################

install(FILES tag-new.png
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

install(FILES tag.png
        DESTINATION ${DATA_INSTALL_DIR}/digikam/icons/hicolor/128x128/actions
       )

install(FILES splash-digikam.png
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
              colors-luma.png
              colors-chromablue.png
              colors-chromared.png
              sample-aix.png
         DESTINATION ${DATA_INSTALL_DIR}/digikam/data
       )

# Showfoto files ######################################################

install(FILES logo-showfoto.png
              banner-showfoto.png
              splash-showfoto.png
        DESTINATION ${DATA_INSTALL_DIR}/showfoto/data
       )

