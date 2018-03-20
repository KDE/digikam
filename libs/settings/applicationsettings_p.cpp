/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QString>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imagefiltersettings.h"
#include "imagesortsettings.h"
#include "thumbnailsize.h"
#include "dbengineparameters.h"
#include "versionmanager.h"
#include "applicationsettings.h"
#include "applicationsettings_p.h"

namespace Digikam
{

const QString ApplicationSettings::Private::configGroupDefault(QLatin1String("Album Settings"));
const QString ApplicationSettings::Private::configGroupExif(QLatin1String("EXIF Settings"));
const QString ApplicationSettings::Private::configGroupMetadata(QLatin1String("Metadata Settings"));
const QString ApplicationSettings::Private::configGroupBaloo(QLatin1String("Baloo Settings"));
const QString ApplicationSettings::Private::configGroupGeneral(QLatin1String("General Settings"));
const QString ApplicationSettings::Private::configGroupVersioning(QLatin1String("Versioning Settings"));
const QString ApplicationSettings::Private::configGroupFaceDetection(QLatin1String("Face Detection Settings"));
const QString ApplicationSettings::Private::configGroupDuplicatesSearch(QLatin1String("Find Duplicates View"));
const QString ApplicationSettings::Private::configGroupGrouping(QLatin1String("Grouping Behaviour"));
const QString ApplicationSettings::Private::configAlbumCollectionsEntry(QLatin1String("Album Collections"));
const QString ApplicationSettings::Private::configAlbumSortRoleEntry(QLatin1String("Album Sort Role"));
const QString ApplicationSettings::Private::configImageSortOrderEntry(QLatin1String("Image Sort Order"));
const QString ApplicationSettings::Private::configImageSortingEntry(QLatin1String("Image Sorting"));
const QString ApplicationSettings::Private::configImageSeparationModeEntry(QLatin1String("Image Group Mode"));
const QString ApplicationSettings::Private::configImageSeparationSortOrderEntry(QLatin1String("Image Group Sort Order"));
const QString ApplicationSettings::Private::configItemLeftClickActionEntry(QLatin1String("Item Left Click Action"));
const QString ApplicationSettings::Private::configDefaultIconSizeEntry(QLatin1String("Default Icon Size"));
const QString ApplicationSettings::Private::configDefaultTreeIconSizeEntry(QLatin1String("Default Tree Icon Size"));
const QString ApplicationSettings::Private::configTreeViewFontEntry(QLatin1String("TreeView Font"));
const QString ApplicationSettings::Private::configThemeEntry(QLatin1String("Theme"));
const QString ApplicationSettings::Private::configSidebarTitleStyleEntry(QLatin1String("Sidebar Title Style"));
const QString ApplicationSettings::Private::configRatingFilterConditionEntry(QLatin1String("Rating Filter Condition"));
const QString ApplicationSettings::Private::configRecursiveAlbumsEntry(QLatin1String("Recursive Albums"));
const QString ApplicationSettings::Private::configRecursiveTagsEntry(QLatin1String("Recursive Tags"));
const QString ApplicationSettings::Private::configIconShowNameEntry(QLatin1String("Icon Show Name"));
const QString ApplicationSettings::Private::configIconShowResolutionEntry(QLatin1String("Icon Show Resolution"));
const QString ApplicationSettings::Private::configIconShowSizeEntry(QLatin1String("Icon Show Size"));
const QString ApplicationSettings::Private::configIconShowDateEntry(QLatin1String("Icon Show Date"));
const QString ApplicationSettings::Private::configIconShowModificationDateEntry(QLatin1String("Icon Show Modification Date"));
const QString ApplicationSettings::Private::configIconShowTitleEntry(QLatin1String("Icon Show Title"));
const QString ApplicationSettings::Private::configIconShowCommentsEntry(QLatin1String("Icon Show Comments"));
const QString ApplicationSettings::Private::configIconShowTagsEntry(QLatin1String("Icon Show Tags"));
const QString ApplicationSettings::Private::configIconShowRatingEntry(QLatin1String("Icon Show Rating"));
const QString ApplicationSettings::Private::configIconShowImageFormatEntry(QLatin1String("Icon Show Image Format"));
const QString ApplicationSettings::Private::configIconShowCoordinatesEntry(QLatin1String("Icon Show Coordinates"));
const QString ApplicationSettings::Private::configIconShowAspectRatioEntry(QLatin1String("Icon Show Aspect Ratio"));
const QString ApplicationSettings::Private::configIconShowOverlaysEntry(QLatin1String("Icon Show Overlays"));
const QString ApplicationSettings::Private::configIconShowFullscreenEntry(QLatin1String("Icon Show Fullscreen"));
const QString ApplicationSettings::Private::configIconViewFontEntry(QLatin1String("IconView Font"));
const QString ApplicationSettings::Private::configToolTipsFontEntry(QLatin1String("ToolTips Font"));
const QString ApplicationSettings::Private::configShowToolTipsEntry(QLatin1String("Show ToolTips"));
const QString ApplicationSettings::Private::configToolTipsShowFileNameEntry(QLatin1String("ToolTips Show File Name"));
const QString ApplicationSettings::Private::configToolTipsShowFileDateEntry(QLatin1String("ToolTips Show File Date"));
const QString ApplicationSettings::Private::configToolTipsShowFileSizeEntry(QLatin1String("ToolTips Show File Size"));
const QString ApplicationSettings::Private::configToolTipsShowImageTypeEntry(QLatin1String("ToolTips Show Image Type"));
const QString ApplicationSettings::Private::configToolTipsShowImageDimEntry(QLatin1String("ToolTips Show Image Dim"));
const QString ApplicationSettings::Private::configToolTipsShowImageAREntry(QLatin1String("ToolTips Show Image AR"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoMakeEntry(QLatin1String("ToolTips Show Photo Make"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoLensEntry(QLatin1String("ToolTips Show Photo Lens"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoDateEntry(QLatin1String("ToolTips Show Photo Date"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoFocalEntry(QLatin1String("ToolTips Show Photo Focal"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoExpoEntry(QLatin1String("ToolTips Show Photo Expo"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoModeEntry(QLatin1String("ToolTips Show Photo Mode"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoFlashEntry(QLatin1String("ToolTips Show Photo Flash"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoWBEntry(QLatin1String("ToolTips Show Photo WB"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumNameEntry(QLatin1String("ToolTips Show Album Name"));
const QString ApplicationSettings::Private::configToolTipsShowTitlesEntry(QLatin1String("ToolTips Show Titles"));
const QString ApplicationSettings::Private::configToolTipsShowCommentsEntry(QLatin1String("ToolTips Show Comments"));
const QString ApplicationSettings::Private::configToolTipsShowTagsEntry(QLatin1String("ToolTips Show Tags"));
const QString ApplicationSettings::Private::configToolTipsShowLabelRatingEntry(QLatin1String("ToolTips Show Label Rating"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAspectRatioEntry(QLatin1String("ToolTips Show Video Aspect Ratio"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioBitRateEntry(QLatin1String("ToolTips Show Audio Bit Rate"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioChannelTypeEntry(QLatin1String("ToolTips Show Audio Channel Type"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioCodecEntry(QLatin1String("ToolTips Show Audio Codec"));
const QString ApplicationSettings::Private::configToolTipsShowVideoDurationEntry(QLatin1String("ToolTips Show Video Duration"));
const QString ApplicationSettings::Private::configToolTipsShowVideoFrameRateEntry(QLatin1String("ToolTips Show Video Frame Rate"));
const QString ApplicationSettings::Private::configToolTipsShowVideoVideoCodecEntry(QLatin1String("ToolTips Show Video Codec"));
const QString ApplicationSettings::Private::configShowAlbumToolTipsEntry(QLatin1String("Show Album ToolTips"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumTitleEntry(QLatin1String("ToolTips Show Album Title"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumDateEntry(QLatin1String("ToolTips Show Album Date"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumCollectionEntry(QLatin1String("ToolTips Show Album Collection"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumCategoryEntry(QLatin1String("ToolTips Show Album Category"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumCaptionEntry(QLatin1String("ToolTips Show Album Caption"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumPreviewEntry(QLatin1String("ToolTips Show Album Preview"));
const QString ApplicationSettings::Private::configPreviewLoadFullItemSizeEntry(QLatin1String("Preview Load Full Image Size"));
const QString ApplicationSettings::Private::configPreviewRawUseEmbeddedPreview(QLatin1String("Preview Raw Use Embedded Preview"));
const QString ApplicationSettings::Private::configPreviewRawUseHalfSizeData(QLatin1String("Preview Raw Use Half Size Data"));
const QString ApplicationSettings::Private::configPreviewConvertToEightBitEntry(QLatin1String("Preview Convert To Eight Bit"));
const QString ApplicationSettings::Private::configPreviewZoomOrgSizeEntry(QLatin1String("Preview Zoom Use Original Size"));
const QString ApplicationSettings::Private::configPreviewShowIconsEntry(QLatin1String("Preview Show Icons"));
const QString ApplicationSettings::Private::configShowThumbbarEntry(QLatin1String("Show Thumbbar"));
const QString ApplicationSettings::Private::configShowFolderTreeViewItemsCountEntry(QLatin1String("Show Folder Tree View Items Count"));
const QString ApplicationSettings::Private::configShowSplashEntry(QLatin1String("Show Splash"));
const QString ApplicationSettings::Private::configUseTrashEntry(QLatin1String("Use Trash"));
const QString ApplicationSettings::Private::configShowTrashDeleteDialogEntry(QLatin1String("Show Trash Delete Dialog"));
const QString ApplicationSettings::Private::configShowPermanentDeleteDialogEntry(QLatin1String("Show Permanent Delete Dialog"));
const QString ApplicationSettings::Private::configApplySidebarChangesDirectlyEntry(QLatin1String("Apply Sidebar Changes Directly"));
const QString ApplicationSettings::Private::configUseNativeFileDialogEntry(QLatin1String("Use Native File Dialog"));
const QString ApplicationSettings::Private::configDrawFramesToGroupedEntry(QLatin1String("Draw Frames To Grouped Items"));
const QString ApplicationSettings::Private::configScrollItemToCenterEntry(QLatin1String("Scroll Current Item To Center"));
const QString ApplicationSettings::Private::configShowOnlyPersonTagsInPeopleSidebarEntry(QLatin1String("Show Only Face Tags For Assigning Name"));
const QString ApplicationSettings::Private::configSyncBalootoDigikamEntry(QLatin1String("Sync Baloo to Digikam"));
const QString ApplicationSettings::Private::configSyncDigikamtoBalooEntry(QLatin1String("Sync Digikam to Baloo"));
const QString ApplicationSettings::Private::configStringComparisonTypeEntry(QLatin1String("String Comparison Type"));
const QString ApplicationSettings::Private::configFaceDetectionAccuracyEntry(QLatin1String("Detection Accuracy"));
const QString ApplicationSettings::Private::configApplicationStyleEntry(QLatin1String("Application Style"));
const QString ApplicationSettings::Private::configIconThemeEntry(QLatin1String("Icon Theme"));
const QString ApplicationSettings::Private::configScanAtStartEntry(QLatin1String("Scan At Start"));
const QString ApplicationSettings::Private::configCleanAtStartEntry(QLatin1String("Clean core DB At Start"));
const QString ApplicationSettings::Private::configMinimumSimilarityBound(QLatin1String("Lower bound for minimum similarity"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastMinSimilarity(QLatin1String("Last minimum similarity"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastMaxSimilarity(QLatin1String("Last maximum similarity"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastAlbumTagRelation(QLatin1String("Last search album tag relation"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastRestrictions(QLatin1String("Last search results restriction"));
const ApplicationSettings::OperationStrings ApplicationSettings::Private::configGroupingOperateOnAll =
        ApplicationSettings::Private::createConfigGroupingOperateOnAll();

ApplicationSettings::OperationStrings ApplicationSettings::Private::createConfigGroupingOperateOnAll()
{
    ApplicationSettings::OperationStrings out;
    out.insert(ApplicationSettings::Metadata,     QLatin1String("Do metadata operations on all"));
    out.insert(ApplicationSettings::ImportExport, QLatin1String("Do Import Export operations on all"));
    out.insert(ApplicationSettings::BQM,          QLatin1String("Do BQM operations on all"));
    out.insert(ApplicationSettings::LightTable,   QLatin1String("Do light table operations on all"));
    out.insert(ApplicationSettings::Slideshow,    QLatin1String("Do slideshow operations on all"));
    out.insert(ApplicationSettings::Rename,       QLatin1String("Rename all"));
    out.insert(ApplicationSettings::Tools,        QLatin1String("Operate tools on all"));
    return out;
}

ApplicationSettings::Private::Private(ApplicationSettings* const qq)
    : showSplash(false),
      useTrash(false),
      showTrashDeleteDialog(false),
      showPermanentDeleteDialog(false),
      sidebarApplyDirectly(false),
      useNativeFileDialog(false),
      drawFramesToGrouped(true),
      scrollItemToCenter(false),
      showOnlyPersonTagsInPeopleSidebar(false),
      iconShowName(false),
      iconShowSize(false),
      iconShowDate(false),
      iconShowModDate(false),
      iconShowTitle(false),
      iconShowComments(false),
      iconShowResolution(false),
      iconShowTags(false),
      iconShowOverlays(false),
      iconShowFullscreen(false),
      iconShowRating(false),
      iconShowImageFormat(false),
      iconShowCoordinates(false),
      iconShowAspectRatio(false),
      showToolTips(false),
      tooltipShowFileName(false),
      tooltipShowFileDate(false),
      tooltipShowFileSize(false),
      tooltipShowImageType(false),
      tooltipShowImageDim(false),
      tooltipShowImageAR(false),
      tooltipShowPhotoMake(false),
      tooltipShowPhotoLens(false),
      tooltipShowPhotoDate(false),
      tooltipShowPhotoFocal(false),
      tooltipShowPhotoExpo(false),
      tooltipShowPhotoMode(false),
      tooltipShowPhotoFlash(false),
      tooltipShowPhotoWb(false),
      tooltipShowAlbumName(false),
      tooltipShowTitles(false),
      tooltipShowComments(false),
      tooltipShowTags(false),
      tooltipShowLabelRating(false),
      tooltipShowVideoAspectRatio(false),
      tooltipShowVideoAudioBitRate(false),
      tooltipShowVideoAudioChannelType(false),
      tooltipShowVideoAudioCodec(false),
      tooltipShowVideoDuration(false),
      tooltipShowVideoFrameRate(false),
      tooltipShowVideoVideoCodec(false),
      showAlbumToolTips(false),
      tooltipShowAlbumTitle(false),
      tooltipShowAlbumDate(false),
      tooltipShowAlbumCollection(false),
      tooltipShowAlbumCategory(false),
      tooltipShowAlbumCaption(false),
      tooltipShowAlbumPreview(false),
      previewShowIcons(true),
      showThumbbar(false),
      showFolderTreeViewItemsCount(false),
      treeThumbnailSize(0),
      thumbnailSize(0),
      ratingFilterCond(0),
      recursiveAlbums(false),
      recursiveTags(false),
      scanAtStart(true),
      cleanAtStart(true),
      databaseDirSetAtCmd(false),
      sidebarTitleStyle(DMultiTabBar::AllIconsText),
      albumSortRole(ApplicationSettings::ByFolder),
      albumSortChanged(false),
      imageSortOrder(0),
      imageSorting(0),
      imageSeparationMode(0),
      imageSeparationSortOrder(0),
      itemLeftClickAction(ApplicationSettings::ShowPreview),
      syncToDigikam(false),
      syncToBaloo(false),
      faceDetectionAccuracy(0.8),
      stringComparisonType(ApplicationSettings::Natural),
      minimumSimilarityBound(40),
      duplicatesSearchLastMinSimilarity(90),
      duplicatesSearchLastMaxSimilarity(100),
      duplicatesSearchLastAlbumTagRelation(0),
      duplicatesSearchLastRestrictions(0),
      groupingOperateOnAll(ApplicationSettings::OperationModes()),
      q(qq)
{
    for (int i = 0; i != ApplicationSettings::Unspecified; ++i)
    {
        groupingOperateOnAll.insert((ApplicationSettings::OperationType)i,
                ApplicationSettings::Ask);
    }
}

ApplicationSettings::Private::~Private()
{
}

void ApplicationSettings::Private::init()
{
    albumCategoryNames.clear();
    albumCategoryNames.append(i18n("Category"));
    albumCategoryNames.append(i18n("Travel"));
    albumCategoryNames.append(i18n("Holidays"));
    albumCategoryNames.append(i18n("Friends"));
    albumCategoryNames.append(i18n("Nature"));
    albumCategoryNames.append(i18n("Party"));
    albumCategoryNames.append(i18n("Todo"));
    albumCategoryNames.append(i18n("Miscellaneous"));
    albumCategoryNames.sort();

    albumSortRole                        = ApplicationSettings::ByFolder;
    imageSortOrder                       = ImageSortSettings::SortByFileName;
    imageSorting                         = ImageSortSettings::AscendingOrder;
    imageSeparationMode                       = ImageSortSettings::CategoryByAlbum;
    imageSeparationSortOrder                  = ImageSortSettings::AscendingOrder;

    itemLeftClickAction                  = ApplicationSettings::ShowPreview;

    thumbnailSize                        = ThumbnailSize::Medium;
    treeThumbnailSize                    = 22;
    treeviewFont                         = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    sidebarTitleStyle                    = DMultiTabBar::AllIconsText;

    ratingFilterCond                     = ImageFilterSettings::GreaterEqualCondition;

    showSplash                           = true;
    useTrash                             = true;
    showTrashDeleteDialog                = true;
    showPermanentDeleteDialog            = true;
    sidebarApplyDirectly                 = false;
    useNativeFileDialog                  = false;
    drawFramesToGrouped                  = true;
    scrollItemToCenter                   = false;
    showOnlyPersonTagsInPeopleSidebar    = false;

    iconShowName                         = false;
    iconShowSize                         = false;
    iconShowDate                         = false;
    iconShowModDate                      = false;
    iconShowTitle                        = false;
    iconShowComments                     = false;
    iconShowResolution                   = false;
    iconShowAspectRatio                  = false;
    iconShowTags                         = true;
    iconShowOverlays                     = true;
    iconShowFullscreen                   = true;
    iconShowRating                       = true;
    iconShowImageFormat                  = true;
    iconShowCoordinates                  = true;
    iconviewFont                         = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    toolTipsFont                         = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    showToolTips                         = false;
    tooltipShowFileName                  = true;
    tooltipShowFileDate                  = false;
    tooltipShowFileSize                  = false;
    tooltipShowImageType                 = false;
    tooltipShowImageDim                  = true;
    tooltipShowImageAR                   = true;
    tooltipShowPhotoMake                 = true;
    tooltipShowPhotoLens                 = true;
    tooltipShowPhotoDate                 = true;
    tooltipShowPhotoFocal                = true;
    tooltipShowPhotoExpo                 = true;
    tooltipShowPhotoMode                 = true;
    tooltipShowPhotoFlash                = false;
    tooltipShowPhotoWb                   = false;
    tooltipShowAlbumName                 = false;
    tooltipShowTitles                    = false;
    tooltipShowComments                  = true;
    tooltipShowTags                      = true;
    tooltipShowLabelRating               = true;

    tooltipShowVideoAspectRatio          = true;
    tooltipShowVideoAudioBitRate         = true;
    tooltipShowVideoAudioChannelType     = true;
    tooltipShowVideoAudioCodec      = true;
    tooltipShowVideoDuration             = true;
    tooltipShowVideoFrameRate            = true;
    tooltipShowVideoVideoCodec           = true;

    showAlbumToolTips                    = false;
    tooltipShowAlbumTitle                = true;
    tooltipShowAlbumDate                 = true;
    tooltipShowAlbumCollection           = true;
    tooltipShowAlbumCategory             = true;
    tooltipShowAlbumCaption              = true;
    tooltipShowAlbumPreview              = false;

    previewShowIcons                     = true;
    showThumbbar                         = true;

    recursiveAlbums                      = false;
    recursiveTags                        = true;

    showFolderTreeViewItemsCount         = false;

    syncToDigikam                        = false;
    syncToBaloo                          = false;
    albumSortChanged                     = false;

    faceDetectionAccuracy                = 0.8;

    minimumSimilarityBound               = 40;
    duplicatesSearchLastMinSimilarity    = 90;
    duplicatesSearchLastMaxSimilarity    = 100;
    duplicatesSearchLastAlbumTagRelation = 0;
    duplicatesSearchLastRestrictions     = 0;

    scanAtStart                          = true;
    cleanAtStart                         = true;
    databaseDirSetAtCmd                  = false;
    stringComparisonType                 = ApplicationSettings::Natural;

    applicationStyle                     = qApp->style()->objectName();
    iconTheme                            = QString();

    for (int i = 0; i != ApplicationSettings::Unspecified; ++i)
    {
        groupingOperateOnAll.insert((ApplicationSettings::OperationType)i,
                                    ApplicationSettings::Ask);
    }

    q->connect(q, SIGNAL(balooSettingsChanged()),
               q, SLOT(applyBalooSettings()));
}

}  // namespace Digikam
