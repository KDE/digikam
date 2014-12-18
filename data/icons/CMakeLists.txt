#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Install standard icons.

ecm_install_icons(ICONS
hi128-actions-adjusthsl.png       hi128-actions-viewimage.png      hi16-actions-redeyes.png         hi22-actions-raindrop.png        hi32-actions-lighttableadd.png    hi48-actions-charcoaltool.png    hi64-actions-adjusthsl.png       hi64-actions-viewimage.png
hi128-actions-charcoaltool.png    hi16-actions-adjusthsl.png       hi16-actions-viewimage.png       hi22-actions-redeyes.png         hi32-actions-lighttable.png       hi48-actions-colorfx.png         hi64-actions-charcoaltool.png
hi128-actions-colorfx.png         hi16-actions-charcoaltool.png    hi22-actions-adjusthsl.png       hi22-actions-viewimage.png       hi32-actions-oilpaint.png         hi48-actions-embosstool.png      hi64-actions-colorfx.png
hi128-actions-embosstool.png      hi16-actions-colorfx.png         hi22-actions-charcoaltool.png    hi32-actions-adjusthsl.png       hi32-actions-raindrop.png         hi48-actions-lensdistortion.png  hi64-actions-embosstool.png
hi128-actions-lensdistortion.png  hi16-actions-embosstool.png      hi22-actions-colorfx.png         hi32-actions-charcoaltool.png    hi32-actions-redeyes.png          hi48-actions-oilpaint.png        hi64-actions-lensdistortion.png
hi128-actions-oilpaint.png        hi16-actions-lensdistortion.png  hi22-actions-embosstool.png      hi32-actions-colorfx.png         hi32-actions-viewimage.png        hi48-actions-raindrop.png        hi64-actions-oilpaint.png
hi128-actions-raindrop.png        hi16-actions-oilpaint.png        hi22-actions-lensdistortion.png  hi32-actions-embosstool.png      hi32-actions-zoom-select-fit.png  hi48-actions-redeyes.png         hi64-actions-raindrop.png
hi128-actions-redeyes.png         hi16-actions-raindrop.png        hi22-actions-oilpaint.png        hi32-actions-lensdistortion.png  hi48-actions-adjusthsl.png        hi48-actions-viewimage.png       hi64-actions-redeyes.png
DESTINATION ${DATA_INSTALL_DIR}/digikam/icons)

add_subdirectory(apps)
add_subdirectory(digikam)
