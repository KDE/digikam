PROJECT(digikam)

# =======================================================
# Informations to update before to release this package.

# digiKam version
SET(DIGIKAM_MAJOR_VERSION "0")
SET(DIGIKAM_MINOR_VERSION "10")
SET(DIGIKAM_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-svn"   : alpha code unstable from svn. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(DIGIKAM_SUFFIX_VERSION "-svn")

# =======================================================
# Set env. variables accordinly.

SET(DIGIKAM_VERSION_STRING "${DIGIKAM_MAJOR_VERSION}.${DIGIKAM_MINOR_VERSION}.${DIGIKAM_PATCH_VERSION}${DIGIKAM_SUFFIX_VERSION}")

# =======================================================
# Depencies

macro_optional_find_package(KdepimLibs)
macro_optional_find_package(LCMS)
macro_optional_find_package(JPEG)
macro_optional_find_package(PNG)
macro_optional_find_package(TIFF)
macro_optional_find_package(Gphoto2)
macro_optional_find_package(Jasper)
macro_optional_find_package(Sqlite)
macro_optional_find_package(Kdcraw)
macro_optional_find_package(Kexiv2)
macro_optional_find_package(Kipi)

message("")
message("-- digiKam dependencies results -------------------")

if(QT_QTSQL_FOUND)
  message("-- Qt4 SQL module found........... YES")
else(SQLITE_FOUND)
  message("-- Qt4 SQL module found........... NO")
  message("")
  message("digiKam needs the SQL module of Qt4")
  message("You need to install a package containing the module with support for sqlite3.")
  message("")
endif(QT_QTSQL_FOUND)

if(SQLITE_FOUND)
  message("-- libsqlite3 library found....... YES")
else(SQLITE_FOUND)
  message("-- libsqlite3 library found....... NO")
  message("")
  message("digiKam needs the SQL module of Qt4 with support for (at least) sqlite3.")
  message("You need to install a package containing the module with support for sqlite3.")
  message("")
endif(SQLITE_FOUND)

if(GPHOTO2_FOUND)
 EXEC_PROGRAM(gphoto2-config ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPHOTO2_VERSION)
 STRING(REPLACE "libgphoto2" "" GPHOTO2_VERSION "${GPHOTO2_VERSION}")
 MACRO_ENSURE_VERSION( "2.4.0" "${GPHOTO2_VERSION}" VERSION_GPHOTO2)
 MESSAGE(STATUS "ensure version ${VERSION_OKK}")
  if( VERSION_GPHOTO2)
    message("-- libgphoto2 library found....... YES")
  else(VERSION_GPHOTO2)
    message("-- libgphoto2 library found....... NO")
    message("-- Install libghoto2 > 2.4.0")
    set(GPHOTO2_FOUND false)
  endif(VERSION_GPHOTO2)
else(GPHOTO2_FOUND)
  message("-- libgphoto2 library found....... NO")
  message("")
  message("digiKam needs libgphoto2. You need to install libgphoto2 first")
  message("libgphoto2 website is at http://www.gphoto.org")
  message("")
endif(GPHOTO2_FOUND)

if(KIPI_FOUND)
  message("-- libkipi library found.......... YES")
else(KIPI_FOUND)
  message("-- libkipi library found.......... NO")
  message("")
  message("digiKam needs libkipi. You need to install libkipi first")
  message("libkipi website is at http://www.kipi-plugins.org")
  message("")
endif(KIPI_FOUND)

if(TIFF_FOUND)
  message("-- libtiff library found.......... YES")
else(TIFF_FOUND)
  message("-- libtiff library found.......... NO")
  message("")
  message("digiKam needs libtiff. You need to install the libtiff development package.")
  message("libtiff website is at http://www.remotesensing.org/libtiff")
  message("")
endif(TIFF_FOUND)

if(PNG_FOUND)
  message("-- libpng library found........... YES")
else(PNG_FOUND)
  message("-- libpng library found........... NO")
  message("")
  message("digiKam needs libpng. You need to install the correct version (>= 1.2.7).")
  message("Libpng website is at http://www.libpng.org/pub/png/libpng.html")
  message("")
endif(PNG_FOUND)

if(JASPER_FOUND)
  message("-- libjasper library found........ YES")
else(JASPER_FOUND)
  message("-- libjasper library found........ NO")
  message("")
  message("digiKam needs libjasper to support JPEG2000. You need to install the correct version (>= 1.7.0).")
  message("Libjasper website is at http://www.ece.uvic.ca/~mdadams/jasper")
  message("")
endif(JASPER_FOUND)

if(LCMS_FOUND)
  message("-- liblcms library found.......... YES")
else(LCMS_FOUND)
  message("-- liblcms library found.......... NO")
  message("")
  message("digiKam needs Lcms library. You need to install the lcms development package.")
  message("Lcms website is at http://www.littlecms.com")
  message("")
endif(LCMS_FOUND)

if(KEXIV2_FOUND)
  message("-- libkexiv2 library found........ YES")
else(KEXIV2_FOUND)
  message("-- libkexiv2 library found........ NO")
  message("")
  message("digiKam needs libkexiv2 library >= 0.1.5. You need to install libkexiv2 first")
  message("libkexiv2 website is at http://www.kipi-plugins.org")
  message("")
endif(KEXIV2_FOUND)

if(KDCRAW_FOUND)
  message("-- libkdcraw library found........ YES")
else(KDCRAW_FOUND)
  message("-- libkdcraw library found........ NO")
  message("")
  message("digiKam needs libkdcraw library >= 0.2.0. You need to install libkdcraw first")
  message("libkdcraw website is at http://www.kipi-plugins.org")
  message("")
endif(KDCRAW_FOUND)

if(KDEPIMLIBS_FOUND)
  message("-- libkdepimlibs library found.... YES")
else(KDEPIMLIBS_FOUND)
  message("-- libkdepimlibs library found.... NO")
  message("")
  message("digiKam will be compiled without address book support.")
  message("")
endif(KDEPIMLIBS_FOUND)

if(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND GPHOTO2_FOUND AND JASPER_FOUND AND SQLITE_FOUND AND KDCRAW_FOUND AND KEXIV2_FOUND AND KIPI_FOUND)
  message("-- digiKam will be compiled...")
  set(DIGIKAM_CAN_BE_COMPILE true)
else(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND GPHOTO2_FOUND AND JASPER_FOUND AND SQLITE_FOUND AND KDCRAW_FOUND AND KEXIV2_FOUND AND KIPI_FOUND)
  message("-- digiKam will not be compiled! Look README file for more details about dependencies...")
  set(DIGIKAM_CAN_BE_COMPILE false)
endif(LCMS_FOUND AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND GPHOTO2_FOUND AND JASPER_FOUND AND SQLITE_FOUND AND KDCRAW_FOUND AND KEXIV2_FOUND AND KIPI_FOUND)

message("------------------------------------------------")
message("")

if(DIGIKAM_CAN_BE_COMPILE)

# =======================================================

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

SET(libimageproperties_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/navigatebarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/navigatebartab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiesmetadatatab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiescolorstab.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/worldmapwidget.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/gpstab.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailbasic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailcreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailloadthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/threadimageio/thumbnailtask.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dlogoaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/dpopupmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/squeezedcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/paniconwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/previewwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/curveswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/histogramwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/splashscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/common/searchtextbar.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/iptcwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/exifwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/makernotewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/metadatawidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/widgets/metadata/xmpwidget.cpp
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

# digiKam target parts ##########################################################

SET(libdialog_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/iccprofileinfodlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imagedlgbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/imageguidedlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/ctrlpaneldlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/dprogressdlg.cpp
   )

SET(libdeletedialog_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/dialogs/deletedialog.cpp
   )

SET(libthemeengine_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/theme.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/texture.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/themeengine/themeengine.cpp
   )

SET(libsetup_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupicc.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupcollections.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupalbumtype.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/setup/setupalbumview.cpp 
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
   )

SET(libimagepropertiesdigikam_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebarcamgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/cameraitempropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagepropertiessidebardb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/talbumlistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/imagedescedittab.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/worldmapwidget.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imageproperties/gpstab.cpp
   )

 SET(libdimgeditor_SRCS 
     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/editorwindow.cpp 
     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imageiface.cpp 
    )

 SET(libdimgimagewindow_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imagewindow.cpp
    )

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
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagescanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/collectionlocation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseaccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasewatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasebackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseparameters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasetransaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databaseurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/databasechangesets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfolist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageinfocache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelister.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagelisterreceiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagecomments.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imageposition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/downloadhistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/imagequerybuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/searchxml.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/namefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/database/schemaupdater.cpp
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

SET(liblighttable_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablepreview.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttableview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablebar.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/lighttable/lighttablewindow.cpp 
   )
SET(libcameragui_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/dkcamera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/umscamera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/gpcamera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/gpiteminfo.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/animwidget.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/renamecustomizer.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerainfodialog.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerafolderdialog.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerafolderitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/camerafolderview.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraiconitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraiconview.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameracontroller.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/cameraui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/freespacewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/capturewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/cameragui/capturedlg.cpp
   )
ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( libs )
ADD_SUBDIRECTORY( utilities )
ADD_SUBDIRECTORY( digikam )
ADD_SUBDIRECTORY( kioslave )
ADD_SUBDIRECTORY( showfoto )
ADD_SUBDIRECTORY( imageplugins )
ADD_SUBDIRECTORY( themedesigner )

INSTALL( FILES tips DESTINATION ${DATA_INSTALL_DIR}/digikam )

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/digikam.lsm.cmake ${CMAKE_CURRENT_BINARY_DIR}/digikam.lsm)
ENDIF(NOT WIN32)

endif(DIGIKAM_CAN_BE_COMPILE)

