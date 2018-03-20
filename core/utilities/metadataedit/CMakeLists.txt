#
# Copyright (c) 2010-2016, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libmetadataedit_SRCS exif/exifeditwidget.cpp
                         exif/exifcaption.cpp
                         exif/exifdatetime.cpp
                         exif/exifadjust.cpp
                         exif/exiflens.cpp
                         exif/exifdevice.cpp
                         exif/exiflight.cpp

                         iptc/iptceditwidget.cpp
                         iptc/iptccontent.cpp
                         iptc/iptcsubjects.cpp
                         iptc/iptckeywords.cpp
                         iptc/iptccategories.cpp
                         iptc/iptccredits.cpp
                         iptc/iptcproperties.cpp
                         iptc/iptcstatus.cpp
                         iptc/iptcorigin.cpp
                         iptc/iptcenvelope.cpp

                         xmp/xmpeditwidget.cpp
                         xmp/xmpcontent.cpp
                         xmp/xmpkeywords.cpp
                         xmp/xmpcategories.cpp
                         xmp/xmpsubjects.cpp
                         xmp/xmporigin.cpp
                         xmp/xmpcredits.cpp
                         xmp/xmpstatus.cpp
                         xmp/xmpproperties.cpp

                         dialog/metadatacheckbox.cpp
                         dialog/timezonecombobox.cpp
                         dialog/altlangstringedit.cpp
                         dialog/multistringsedit.cpp
                         dialog/multivaluesedit.cpp
                         dialog/objectattributesedit.cpp
                         dialog/metadataedit.cpp
   )

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(metadataedit_src OBJECT ${libmetadataedit_SRCS})

add_library(metadataedit STATIC $<TARGET_OBJECTS:metadataedit_src>)

target_link_libraries(metadataedit
                      PRIVATE
                      Qt5::Gui
)
