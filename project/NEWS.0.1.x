v 0.1.6
---------------------------------------------------------------------------

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 162687 : Make Flickr Export screen non-modal.
002 ==> 162683 : Export to picassaweb doesn't works.
003 ==> 164152 : Geotagging when exporting to Flickr.
004 ==> 166712 : Choosing the part which is cut off for printing only works for the first photo.
005 ==> 162994 : Picasa album list does not contain "not listed" albums, contains only "public" albums.
006 ==> 150912 : Uploader does not upload.
007 ==> 164908 : Private albums are not listed (and then not usable).
008 ==> 150979 : Picasa Export-Plugins does not work.

v 0.1.6 - beta1
---------------------------------------------------------------------------

NEW FEATURES:

FlickrExport   : dialog layout re-written to be more suitable.
FlickrExport   : list of item to upload is now display in dialog.
FlickrExport   : Support RAW files format to upload as JPEG preview.
PicasaWebExport: Support RAW files format to upload as JPEG preview.

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 149890 : Adjust date and time from Exif does not work.
002 ==> 154273 : Quality setting for jpegs saved by raw converter is insanely high.
003 ==> 157190 : Misleading description for geotagging images.
004 ==> 154289 : Cannot upload RAW images.
005 ==> 153758 : FlickrUploader fails to upload photos whose caption contains 
                                      accented characters or a trailing space.
006 ==> 128211 : "Ok" Button in "Add Photos" shouldn't fire upload directly.
007 ==> 159081 : Upload to Flickr not working.
008 ==> 162096 : When selecting all photos reverses the order.
009 ==> 158483 : Plugin cause Digikam crash.
010 ==> 160453 : Crashes when exporting to Picasaweb.
011 ==> 155270 : Tags with non-Latin characters dropped during image export.
012 ==> 153207 : Without using application tags: Flickr still reads embedded metadata into tags.
013 ==> 153206 : Provide option to remove spaces in tags during export.
014 ==> 145149 : Flickr uploading from DigiKam does not cache auth data.

v 0.1.5
---------------------------------------------------------------------------

NEW FEATURES:

SimpleViewerExport : tool is now compatible with SimpleViewer version 1.8.x.
SimpleViewerExport : tool now support RAW files format.

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 155231 : Timezones are assumed to be only in hourly increments/decrements of GMT.
002 ==> 154849 : Updates first find only, does not match other than GMT camera time.
003 ==> 144070 : Typing coordinates doesn't update the map.
004 ==> 152526 : Remember last display options like zoom and map/sattelite/hybrid view.
005 ==> 158176 : Linking errors in slideshow tool (missing -lXrandr).
006 ==> 150393 : XML file is corrupted if exif comment contains a "<".
007 ==> 134774 : Exported simpleviewer gallery fails to load images due to wrong xml file name.

v 0.1.5 - RC2
---------------------------------------------------------------------------

NEW FEATURES:

PrintWizard    : caption can contain more exif info 

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 151578 : Popup window says "Cannot run properly 'convert' program 
                 from 'ImageMagick' package" but it seems fine. (really fixed)
002 ==> 155084 : Printing comments and EXIF tags.
003 ==> 155371 : Add search feature to the embedded GPS Sync Google Maps based editor.
004 ==> 102021 : Pan and Zoom on Slideshow viewing (not a transition) a la iPhoto

v 0.1.5 - RC1
---------------------------------------------------------------------------

NEW FEATURES:

General        : Added the availability to disable tools we don't want to build.
               : This feature is very useful for source based distros (Matej Laitl)
PrintWizard    : Added raw file management, now raw files can be printed.
SlideShow      : Solved minor issue in filename printing (2D slideshow).
SlideShow      : New caching mechanism
SlideShow      : Added Ken Burns effect
Calendar       : Fixed recurring events not showing (only first date was showed)
                 Setting special events only once (before printing), instead of
                 one for every page

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 149666 : iPod Export tool cannot be disabled at compile 
                 time when libgpod is present on system.
002 ==> 151604 : Print Wizard does not recognize raw images.
003 ==> 102021 : Pan and Zoom on Slideshow viewing (not a transition) 
                 a la iPhoto.
004 ==> 151578 : Popup window says "Cannot run properly 'convert' program 
                 from 'ImageMagick' package" but it seems fine.
005 ==> 152210 : Metadata lost when converting from png to jpeg (IPTC 
                 thumbnail too big).
006 ==> 152215 : GPS correlator starts too many kio_thumbnail processes.
007 ==> 149491 : GPS correlator GPSSYNC do_not admit GMT+5h30 (India).
008 ==> 150114 : Max time gap is limited to 999 or 2000 seconds.
009 ==> 154244 : SHIFT triggers help dialog.

v 0.1.5 - Beta1
----------------------------------------------------------------------------

NEW FEATURES:

General        : Added configure options to allow disabling tools (--disable-XXXX)
                 provided by Matej Laitl (bug #149666)

PrintWizard    : Wizard GUI review
                 Added exif management.
                 Prints exif date-time info.
                 Font, color and size of captions.
                 Added an option to to print without margins (full-bleed).
                 Added 10x13.33cm into a4 (provided by Joerg Kuehne)
                 Added full size A4 printing (one photo per A4 page)
                 Added A6 size to get the old configuration for 10x15cm page
                 Changed page size to real 10x15cm instead of A6 (need to set up right page size on kprinter)
                 Added a new page size 13x18cm (need to set up right page size on kprinter)
                 Each photo can be printed more than once
PicasaWebExport: New tool to export pictures to Picasa web service (by Vardhman Jain).
SlideShow      : Dropped imlib2 dependency

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 133193 : Data on the photo.
002 ==> 111454 : Print photo's date into one corner like print assistant.
003 ==> 146457 : Rotation is not done correctly Exif.
004 ==> 138838 : Digikam Picasaweb export tool.
005 ==> 103152 : Improvement suggestions for printing wizard.
006 ==> 117085 : Have pictures fit a whole, single sheet.
007 ==> 100471 : Can't print the same image more than once on the same page.
008 ==> 148621 : Image rotation not working properly.
009 ==> 144388 : Cache is not updated after rotating pictures.
010 ==> 144604 : Rotation causes Exif data corruption.
011 ==> 150063 : Rotating JPEG produces error and truncates original file.

v 0.1.4
----------------------------------------------------------------------------

Note on release: 
Due to missing files on svn, docbook "pt" and "da" have been removed form this 
final release, apologize for that.

v 0.1.4 beta2
----------------------------------------------------------------------------

NEW FEATURES:

HTMLExport     : Option to specify whether the original images should be included
HTMLExport     : Support for theme variants
HTMLExport     : New theme: "frames", by Rüdiger Bente
HTMLExport     : New theme: "cleanframes", by Beth and Robert Marmorstein
HTMLExport     : New theme: "classic", simulating the output of the old HTML Gallery tool
SlideShow      : Skip to next or previous image by a right or left click
SlideShow      : Skip to next or previous image by mouse wheel scrolling
SlideShow      : Images can be sorted/added/removed manually.
SlideShow      : Progress indicator printing doesn't depend on file name printing anymore.

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 140477 : Ability to rename images being sent via email.
002 ==> 143450 : Skip to next or previous image by a right or left click
003 ==> 138880 : digiKam 0.9rc2 - 0.9.1rc1 setting file date to exif doesn't work.
004 ==> 140890 : The preview does not display date and time properly for Japanese locale.
005 ==> 144185 : Adjust date-time tool should remember previous fixed date.
006 ==> 146799 : digikam 0.9.2 crashes when exiting - slideshow error

v 0.1.4 beta1 
----------------------------------------------------------------------------

NEW FEATURES:

General        : Moved Exiv2Iface class to a new shared library named libkexiv2 used by 
                 kipi-tools and digiKam.
ImageViewer    : initial import of new OpenGL based image viewer.
RAWConverter   : Port tool to libkdcraw shared library.
Printwizard    : Printwizard can print 8 photos per page (A4)
MPEGEncoder    : Avoid to pass img2mpg script unmanaged file path.
GPSSync        : New tool to export GPS locations from pictures to Google Maps / Google Earth.

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 139264 : Prefer Exif DateTimeOriginal for image date/time (DateTimeDigitized and DateTime only used as fallback)
002 ==> 139074 : Format missmatch at sendimages.cpp ('int' vs. 'size_t').
003 ==> 140132 : Comments should sync to IPTC Caption First.
006 ==> 138241 : A patch that adds support for the Claws Mail MUA.
007 ==> 140865 : Plugin does not work (image can not be converted).
008 ==> 141528 : Remove confirmation dialog for image rotate.
009 ==> 141530 : Use Rotate left/right instead of degrees.
010 ==> 142848 : Timezone needs to go to GMT +13.
011 ==> 140297 : GPS tool truncates input coordinates, introducing inacuracy.
012 ==> 143594 : Bad Interpolation in correlate gpssync.
014 ==> 139793 : KML google export import.
015 ==> 142259 : Export to Gallery 2.2-RC-1 fails.
016 ==> 135945 : Tags with spaces are exported as multible tags.
017 ==> 146084 : Slide show interface suggestions.
018 ==> 145771 : Gnome Desktop crashes and restarts when select Cancel option for Mpeg Slideshow tool

v 0.1.3 
----------------------------------------------------------------------------

BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 137582 : Add preliminary support for Gallery 2.2 security features
002 ==> 132220 : Solved problems with filenames and commandline with thunderbird and mozilla

v 0.1.3 rc1
----------------------------------------------------------------------------

NEW FEATURES:

Slideshow      : Show image comments (configurable)


BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 138410 : kipi-tools-0.1.3-beta1 requires latest libkipi/libkexif.
002 ==> 106133 : Show image comments in slideshow mode.
003 ==> 124057 : Problems sending jpeg-pictures from digiKam using the "send picture"-feature.
004 ==> 108147 : Interval below 1 second.

v 0.1.3 beta1
----------------------------------------------------------------------------

NEW FEATURES:

New Plugin    : MetadataEdit : New tool to edit EXIF and IPTC pictures metadata (by Gilles Caulier).
New Plugin    : GPSSync      : New tool to sync photo metadata with a GPS device (by Gilles Caulier).
New Plugin    : IpodExport   : New tool to export pictures to an ipod device (by Seb Ruiz).

GalleryExport : Support for multiple galleries.

HTMLExport    : New "s0" theme from Petr Vanek

JPEGLossLess  : Removed libmagic++ depency.
JPEGLossLess  : Removed libkexif depency. Using Exiv2 instead.

RAWConverter  : New core to be compatible with recent dcraw release. A lot 
                of RAW decoding settings have been added.
RAWConverter  : Embedding ouput color space in target image (JPEG/PNG/TIFF).
RAWConverter  : Metadata preservation in target image during Raw conversion (JPEG/PNG).
RAWConverter  : Removing external dcraw depency. Now tool include a full supported version of 
                dcraw program in core.
RAWConverter  : updated dcraw.c implementation to version 8.41.

SendImages    : Added image size limit x mail (Michael H�hstetter)
 
TimeAdjust    : Removed libkexif depency. Using Exiv2 instead.
TimeAdjust    : New option to customize Date and Time to a specific timestamp.
TimeAdjust    : New options sync EXIF/IPTC Creation Date with timestamp.


BUG FIXING from B.K.O (http://bugs.kde.org):

001 ==> 127101 : expand sequence number start value in batch rename images.
002 ==>  94494 : support for multiple galleries.
003 ==> 128394 : convertion of RAW files fails with dcraw 8.21
004 ==> 132659 : "Missing signature" - Flickr API changed and upload of 
                 images is no longer possible.
005 ==> 107905 : copy exif data from raw to converted images.
006 ==> 119537 : Exif width and height are not corrected after lossless rotation.
007 ==> 91545  : tool does nothing if an album only contains subalbums, but no 
                 images directly or is empty.
008 ==> 134749 : altitude shown is wrong.
009 ==> 134298 : save settings / keep settings missing!
010 ==> 134747 : not optimal correlation.
011 ==> 135157 : warning about changes not applied always appear even when already applied.
012 ==> 135237 : filenames with multiple periods in them do not show up in the file 
                 listing (incorrect extension identification).
013 ==> 135484 : thumbnail generation for multible images can cause severe overload.
014 ==> 135353 : the name of the tool is missleading.
015 ==> 136257 : Editing the EXIF-data overwrites all the data for selected files.
016 ==> 128341 : html export should not resize images if "resize target images" 
                 is not checked.
017 ==> 127476 : Printing as very very slow (added a workaround running kjobviewer)
018 ==> 136941 : graphical picture ordering and renaming.
019 ==> 136855 : Editing metadata on a few selected imagefiles and clicking forward 
                 or apply crashes digiKam.
020 ==> 135408 : Window does not fit on screen.
021 ==> 117399 : Usability of Target folder.
022 ==> 137921 : wrong country code in IPTC.

v 0.1.2
----------------------------------------------------------------------------

Compilation fix release.

v 0.1.1
----------------------------------------------------------------------------

Compilation fix release.

v 0.1.0
----------------------------------------------------------------------------

NEW FEATURES:

GalleryExport : added Gallery 2 version support.
ImageGallery  : removed is removed and replaced by HTML export tool.

BUGFIX from B.K.O (http://bugs.kde.org):

001 ==> 117105 : Calendar tool should use internationalized country setting.
002 ==> 101656 : Use irretating filenames foo.jpeg.jpeg for images.
003 ==> 128125 : Album title from digikam are not converted into html entities.
004 ==> 123978 : "Invalid response" error when exporting images to Gallery 1.5.2.
005 ==> 96352  : Can not login into Gallery2.
006 ==> 123141 : Gallery Export - manage several cookies.
007 ==> 88887  : No Exif-rotation in HTML export.
008 ==> 115474 : Web export creats duplicate extensions.
009 ==> 120739 : Wrong thumbnail for album.
010 ==> 89068  : Improvement for the HTML export tool.
011 ==> 90943  : Add CSS functionality.
012 ==> 95116  : Incremental local export or other easy web publishing method.
013 ==> 96009  : Unnecessary deletion of directories in "export HTML" .
014 ==> 96363  : Option to save full/different sized images in gallery.
015 ==> 107380 : Split long html pages by number of images per page.
016 ==> 108696 : Themable html export.
017 ==> 109708 : Number of thumbnails per row should be in the same tab as size of thumbnail.
018 ==> 109709 : Create target dir when it does not exist.
019 ==> 109710 : Make clicking on image going to the next image.
020 ==> 111136 : export to non-local directory (fish://) does not work.
021 ==> 111509 : Subalbums not supported by HTML export.
022 ==> 111880 : New option to add original image (as link in thumbnail).
023 ==> 112107 : Avi files in html exports.
024 ==> 113355 : Add auto-forwarding (slide show) to HTML-Export.
025 ==> 127219 : Creation of mpeg slide show fails.
026 ==> 127532 : 'image2mpg' wrong directory error.
027 ==> 101455 : Make it possible to enter numbers with 2 digit precision in RAW converter dialog.

v0.1.0-rc2
----------------------------------------------------------------------------

NEW FEATURES:

New Plugin     : HTMLExport   : new images gallery export supporting XHTML and CSS (by Aurelien Gateau).
New Plugin     : SimpleViewer : new tool to export to flash web page (by Joern Ahrens)

BUGFIX from B.K.O (http://bugs.kde.org):

001 ==> 120242 : Bad sorting of images in html export.
002 ==> 112025 : digiKam overwrites albums previously exported to HTML.
003 ==> 106152 : Creates faulty links when choosing picture filenames derived from the EXIF info.
004 ==> 119933 : Image gallery has problems with german umlauts in file-/directorynames.
005 ==> 99418  : Help menu in progress dialog refers to batch process tool about instead of image gallery tools.
006 ==> 103449 : Title and name of album are together and album with accents do not function.
007 ==> 110596 : Apos entitity is not correct for HTML (it is XML entity).
008 ==> 116605 : Crash when exporting to an existing dir an choosing no to overwrite.
009 ==> 123499 : RAW images are rotated wrong.
010 ==> 99157  : Some kameraklient source files miss copyright and license info.
011 ==> 98286  : Print Wizard has wrong default paper size.
012 ==> 101495 : Raster effect on printout.
013 ==> 117670 : Printing is awfully slow.
014 ==> 108945 : Batch image filtering overwrite mode: always overwrite doesn't work.
015 ==> 117397 : batchtools 'start' not disabled when target folder is not writeable orwith no image in list.
016 ==> 114512 : The checkbox "Remove original" is left disabled after a preview.
017 ==> 120868 : Failed to create PDF callendar
018 ==> 118936 : calendar wizard should default to next year.
019 ==> 109739 : yuvscaler error in digikam.
020 ==> 114514 : Do not delete the temporary folder "~/tmp/kde-user/kipi-mpegencodertool-PID/" after 
                 each encoding process.
021 ==> 114515 : Verify the existence of the MPEG output file path and the existence of the audio input file 
                 before launching the encoding process.
022 ==> 114519 : Crash when stopping a Final Scan.
023 ==> 103763 : Rawconverter (single) should fill a default file name into the save as dialog.
024 ==> 118407 : dcrawprocess.cpp does not compileon Solaris
025 ==> 119562 : A patch that adds support for the Sylpheed-Claws mua.
026 ==> 119867 : Different icon types used by slideshow for for backwards/forwards.

v0.1.0-rc1
----------------------------------------------------------------------------

NEW FEATURES:

New Plugin     : FlickrExport : new tool to upload pictures on Flickr web service (by Vardhman Jain).

SendImages     : support for Thunderbird and GmailAgent.
Calendar       : The weekdays are now localized.
HTMLExport     : Export multiple tags to an html-page.
JPEGLossLess   : Rotate or flip your images lossless, while preserving  the timestamp.
RAWConverter   : Supports of dcraw>=6.x.

BUGFIX from B.K.O (http://bugs.kde.org):

001 ==> 108227 : Thunderbird will not open when sending emails in digiKam.
002 ==> 98269  : Status bar in archive to CD/DVD goes to 100% while creating thumbs.
003 ==> 89394  : Make CDArchiving tool work when ImageCollection!=Folder.
004 ==> 91651  : Running cdarchiving tool sends all images to the cd, not just the selected images.
005 ==> 100877 : kimdaba can not create temporary directories for CD-archive.
006 ==> 110391 : Batch rename removes tags and comments.
007 ==> 110659 : Batch rename function makes copies instead of renaming.
008 ==> 110698 : Adjust time and date does not work.
009 ==> 110575 : Crash when renaming images.
010 ==> 99895  : Rename ordered by modification date sorts by name.
011 ==> 104032 : Renaming images takes a lot memory and time.
012 ==> 105727 : digiKam adds to the first picture an additional "_1".
013 ==> 110508 : Umlauts-conversion error when renaming images.
014 ==> 104511 : Why a destination path for a rename action ? "No valid URL" when blank.
015 ==> 102219 : When you export html from a tags gallery links are incorrect.
016 ==> 98199  : Missing whitespace in german html-export (headline).
017 ==> 108537 : Plugin change file date/time. Could this be made optional since I want to keep 
                 the original file date/time.
018 ==> 101110 : Cannot create MPEG from photos using transitions.
019 ==> 103282 : No exif-rotation.
