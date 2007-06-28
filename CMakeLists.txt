PROJECT(digikam)

ADD_DEFINITIONS(${QT_DEFINITIONS}
                ${KDE4_DEFINITIONS} 
                ${KDCRAW_DEFINITIONS} 
                ${KEXIV2_DEFINITIONS}
               )

INCLUDE_DIRECTORIES(${KIPI_INCLUDE_DIR}
                    ${KEXIV2_INCLUDE_DIR} 
                    ${KDCRAW_INCLUDE_DIR}
                    ${KDE4_INCLUDE_DIR}
                    ${QT_INCLUDES}
                    ${GPHOTO2_INCLUDE_DIR}
                   )

ADD_DEFINITIONS(-DQT3_SUPPORT 
                -DQT3_SUPPORT_WARNINGS
               )

# Common targets parts ##########################################################

SET(libcurves_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/curves/imagecurves.cpp 
   )

SET(libdimg_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimgscale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorcomposer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/ddebug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/fastscale.cpp
   )

SET(libdimgloaders_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/dimgloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/rawloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/ppmloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/qimageloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/iccjpeg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2kloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jpegsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/pngsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/tiffsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/loaders/jp2ksettings.cpp
   )

SET(libdimgfilters_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bcgmodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/hslmodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icctransform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgimagefilters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/colormodifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimggaussianblur.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgsharpen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/dimgthreadedfilter.cpp
   )

SET(libdmetadata_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dmetadata/dmetadata.cpp
   )

SET(libgreycstoration_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/greycstoration/greycstorationiface.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/greycstoration/greycstorationwidget.cpp
   )

SET(libhistogram_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/histogram/imagehistogram.cpp
   )

SET(libimagepropertiesshowfoto_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/navigatebarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/navigatebartab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesmetadatatab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiescolorstab.cpp
   )

SET(libjpegutils_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/jpegutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/transupp.cpp
   )

SET(liblevels_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/levels/imagelevels.cpp
   )

SET(liblprof_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmshull.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmslm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmslnr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsmatn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsmkmsh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsmntr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsoutl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmspcoll.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsprf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsreg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmsscn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/lprof/cmssheet.cpp
   )

SET(libthreadimageio_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadsavethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/managedloadsavethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/sharedloadsavethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadingdescription.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadingcache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadingcacheinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/loadsavetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/previewloadthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/previewtask.cpp
   )

SET(libthumbbar_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/thumbbar/thumbbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/thumbbar/thumbnailjob.cpp
   )

SET(libwhitebalance_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/whitebalance/whitebalance.cpp
   )

SET(libcommonwidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/colorgradientwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dcursortracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/filesaveoptionsbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statuszoombar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusprogressbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/statusnavigatebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/sidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dpopupmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/squeezedcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/paniconwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/previewwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/curveswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/histogramwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/splashscreen.cpp
   )

SET(libiccprofileswidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccpreviewwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/iccprofilewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/iccprofiles/cietonguewidget.cpp
   )

SET(libimagepluginswidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/listboxpreviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imagepaniconwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imageregionwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imageguidewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imagewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/imageplugins/imagepannelwidget.cpp
   )

SET(libmetadatawidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/mdkeylistviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatalistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatalistviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/worldmapwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/iptcwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/exifwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/makernotewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/gpswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatawidget.cpp
   )

SET(libslideshow_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/toolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/slideshow/slideshow.cpp
   )

SET(libsetupshowfoto_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupiofiles.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdcraw.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupicc.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupslideshow.cpp
   )

SET(libdimgcanvas_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/imageplugin.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/imagepluginloader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undocache.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undoaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/undomanager.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/colorcorrectiondlg.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/dimginterface.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/canvas/canvas.cpp 
   )

SET(libdimgeditortools_SRCS 
     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/tools/imageprint.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/tools/imageresize.cpp
   )

# Showfoto target parts ##########################################################

SET(libshowfotosetup_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setuptooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setupeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/showfoto/setup/setup.cpp 
   )

SET(libshowfoto_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imageiface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorwindow.cpp
   )

SET(libdialogshowfoto_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/iccprofileinfodlg.cpp
   )

# digiKam target parts ##########################################################

SET(libdialog_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/iccprofileinfodlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dprogressdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/deletedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imagedlgbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imageguidedlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/ctrlpaneldlg.cpp
   )

SET(libthemeengine_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/theme.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/texture.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/themeengine.cpp
   )

SET(libsetup_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupicc.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcollections.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupgeneral.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setuptooltip.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupidentity.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupmime.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupslideshow.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupmetadata.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupmisc.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setuplighttable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupiofiles.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupplugins.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupdcraw.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcamera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/cameraselection.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/gpiface.cpp 
   )

SET(libimagepropertiescamgui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebarcamgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/cameraitempropertiestab.cpp
   )

SET(libimagepropertiesdigikam_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/navigatebarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/navigatebartab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesmetadatatab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiescolorstab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebardb.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagedescedittab.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/talbumlistview.cpp
   )

# SET(libdimgeditor_SRCS 
#     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorwindow.cpp 
#     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imageiface.cpp 
#     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imagewindow.cpp
#    )

SET(libbatch_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/imageinfojob.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/imageinfoalbumsjob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/batchthumbsgenerator.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/batchalbumssyncmetadata.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/batch/batchsyncmetadata.cpp
   )

SET(libdatabase_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/albumdb_sqlite2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionscanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionlocation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseaccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseattributeswatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasebackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseparameters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasetransaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfolist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfocache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelister.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelisterreceiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagequerybuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/namefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/schemaupdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/backendsqlite3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/upgradedb_sqlite2tosqlite3.cpp
   )

SET(libsqlite2_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/attach.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/auth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/btree.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/btree_rb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/build.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/copy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/date.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/delete.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/expr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/func.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/insert.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/opcodes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/os.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/pager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/parse.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/pragma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/printf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/random.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/select.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/shell.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/tokenize.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/trigger.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/update.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/vacuum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/vdbe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/vdbeaux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/sqlite2/where.c
   )

ADD_SUBDIRECTORY( libs )
ADD_SUBDIRECTORY( utilities )
ADD_SUBDIRECTORY( showfoto )
ADD_SUBDIRECTORY( digikam )
ADD_SUBDIRECTORY( kioslave )
#ADD_SUBDIRECTORY( imageplugins )
#ADD_SUBDIRECTORY( themedesigner )
ADD_SUBDIRECTORY( data )

INSTALL( FILES tips DESTINATION ${DATA_INSTALL_DIR}/digikam )
