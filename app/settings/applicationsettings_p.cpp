/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QFont>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kapplication.h>

// Local includes

#include "imagefiltersettings.h"
#include "imagesortsettings.h"
#include "thumbnailsize.h"
#include "databaseparameters.h"
#include "versionmanager.h"
#include "applicationsettings.h"
#include "applicationsettings_p.h"

namespace Digikam
{

const QString ApplicationSettings::Private::configGroupDefault("Album Settings");
const QString ApplicationSettings::Private::configGroupExif("EXIF Settings");
const QString ApplicationSettings::Private::configGroupMetadata("Metadata Settings");
const QString ApplicationSettings::Private::configGroupBaloo("Baloo Settings");
const QString ApplicationSettings::Private::configGroupGeneral("General Settings");
const QString ApplicationSettings::Private::configGroupVersioning("Versioning Settings");
const QString ApplicationSettings::Private::configGroupFaceDetection("Face Detection Settings");
const QString ApplicationSettings::Private::configAlbumCollectionsEntry("Album Collections");
const QString ApplicationSettings::Private::configAlbumSortOrderEntry("Album Sort Order");
const QString ApplicationSettings::Private::configImageSortOrderEntry("Image Sort Order");
const QString ApplicationSettings::Private::configImageSortingEntry("Image Sorting");
const QString ApplicationSettings::Private::configImageGroupModeEntry("Image Group Mode");
const QString ApplicationSettings::Private::configImageGroupSortOrderEntry("Image Group Sort Order");
const QString ApplicationSettings::Private::configItemLeftClickActionEntry("Item Left Click Action");
const QString ApplicationSettings::Private::configDefaultIconSizeEntry("Default Icon Size");
const QString ApplicationSettings::Private::configDefaultTreeIconSizeEntry("Default Tree Icon Size");
const QString ApplicationSettings::Private::configTreeViewFontEntry("TreeView Font");
const QString ApplicationSettings::Private::configThemeEntry("Theme");
const QString ApplicationSettings::Private::configSidebarTitleStyleEntry("Sidebar Title Style");
const QString ApplicationSettings::Private::configRatingFilterConditionEntry("Rating Filter Condition");
const QString ApplicationSettings::Private::configRecursiveAlbumsEntry("Recursive Albums");
const QString ApplicationSettings::Private::configRecursiveTagsEntry("Recursive Tags");
const QString ApplicationSettings::Private::configIconShowNameEntry("Icon Show Name");
const QString ApplicationSettings::Private::configIconShowResolutionEntry("Icon Show Resolution");
const QString ApplicationSettings::Private::configIconShowSizeEntry("Icon Show Size");
const QString ApplicationSettings::Private::configIconShowDateEntry("Icon Show Date");
const QString ApplicationSettings::Private::configIconShowModificationDateEntry("Icon Show Modification Date");
const QString ApplicationSettings::Private::configIconShowTitleEntry("Icon Show Title");
const QString ApplicationSettings::Private::configIconShowCommentsEntry("Icon Show Comments");
const QString ApplicationSettings::Private::configIconShowTagsEntry("Icon Show Tags");
const QString ApplicationSettings::Private::configIconShowRatingEntry("Icon Show Rating");
const QString ApplicationSettings::Private::configIconShowImageFormatEntry("Icon Show Image Format");
const QString ApplicationSettings::Private::configIconShowCoordinatesEntry("Icon Show Coordinates");
const QString ApplicationSettings::Private::configIconShowAspectRatioEntry("Icon Show Aspect Ratio");
const QString ApplicationSettings::Private::configIconShowOverlaysEntry("Icon Show Overlays");
const QString ApplicationSettings::Private::configIconViewFontEntry("IconView Font");
const QString ApplicationSettings::Private::configToolTipsFontEntry("ToolTips Font");
const QString ApplicationSettings::Private::configShowToolTipsEntry("Show ToolTips");
const QString ApplicationSettings::Private::configToolTipsShowFileNameEntry("ToolTips Show File Name");
const QString ApplicationSettings::Private::configToolTipsShowFileDateEntry("ToolTips Show File Date");
const QString ApplicationSettings::Private::configToolTipsShowFileSizeEntry("ToolTips Show File Size");
const QString ApplicationSettings::Private::configToolTipsShowImageTypeEntry("ToolTips Show Image Type");
const QString ApplicationSettings::Private::configToolTipsShowImageDimEntry("ToolTips Show Image Dim");
const QString ApplicationSettings::Private::configToolTipsShowImageAREntry("ToolTips Show Image AR");
const QString ApplicationSettings::Private::configToolTipsShowPhotoMakeEntry("ToolTips Show Photo Make");
const QString ApplicationSettings::Private::configToolTipsShowPhotoDateEntry("ToolTips Show Photo Date");
const QString ApplicationSettings::Private::configToolTipsShowPhotoFocalEntry("ToolTips Show Photo Focal");
const QString ApplicationSettings::Private::configToolTipsShowPhotoExpoEntry("ToolTips Show Photo Expo");
const QString ApplicationSettings::Private::configToolTipsShowPhotoModeEntry("ToolTips Show Photo Mode");
const QString ApplicationSettings::Private::configToolTipsShowPhotoFlashEntry("ToolTips Show Photo Flash");
const QString ApplicationSettings::Private::configToolTipsShowPhotoWBEntry("ToolTips Show Photo WB");
const QString ApplicationSettings::Private::configToolTipsShowAlbumNameEntry("ToolTips Show Album Name");
const QString ApplicationSettings::Private::configToolTipsShowTitlesEntry("ToolTips Show Titles");
const QString ApplicationSettings::Private::configToolTipsShowCommentsEntry("ToolTips Show Comments");
const QString ApplicationSettings::Private::configToolTipsShowTagsEntry("ToolTips Show Tags");
const QString ApplicationSettings::Private::configToolTipsShowLabelRatingEntry("ToolTips Show Label Rating");
const QString ApplicationSettings::Private::configToolTipsShowVideoAspectRatioEntry("ToolTips Show Video Aspect Ratio");
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioBitRateEntry("ToolTips Show Audio Bit Rate");
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioChannelTypeEntry("ToolTips Show Audio Channel Type");
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioCompressorEntry("ToolTips Show Audio Compressor");
const QString ApplicationSettings::Private::configToolTipsShowVideoDurationEntry("ToolTips Show Video Duration");
const QString ApplicationSettings::Private::configToolTipsShowVideoFrameRateEntry("ToolTips Show Video Frame Rate");
const QString ApplicationSettings::Private::configToolTipsShowVideoVideoCodecEntry("ToolTips Show Video Codec");
const QString ApplicationSettings::Private::configShowAlbumToolTipsEntry("Show Album ToolTips");
const QString ApplicationSettings::Private::configToolTipsShowAlbumTitleEntry("ToolTips Show Album Title");
const QString ApplicationSettings::Private::configToolTipsShowAlbumDateEntry("ToolTips Show Album Date");
const QString ApplicationSettings::Private::configToolTipsShowAlbumCollectionEntry("ToolTips Show Album Collection");
const QString ApplicationSettings::Private::configToolTipsShowAlbumCategoryEntry("ToolTips Show Album Category");
const QString ApplicationSettings::Private::configToolTipsShowAlbumCaptionEntry("ToolTips Show Album Caption");
const QString ApplicationSettings::Private::configToolTipsShowAlbumPreviewEntry("ToolTips Show Album Preview");
const QString ApplicationSettings::Private::configPreviewLoadFullImageSizeEntry("Preview Load Full Image Size");
const QString ApplicationSettings::Private::configPreviewRawUseEmbeddedPreview("Preview Raw Use Embedded Preview");
const QString ApplicationSettings::Private::configPreviewRawUseHalfSizeData("Preview Raw Use Half Size Data");
const QString ApplicationSettings::Private::configPreviewShowIconsEntry("Preview Show Icons");
const QString ApplicationSettings::Private::configShowThumbbarEntry("Show Thumbbar");
const QString ApplicationSettings::Private::configShowFolderTreeViewItemsCountEntry("Show Folder Tree View Items Count");
const QString ApplicationSettings::Private::configShowSplashEntry("Show Splash");
const QString ApplicationSettings::Private::configUseTrashEntry("Use Trash");
const QString ApplicationSettings::Private::configShowTrashDeleteDialogEntry("Show Trash Delete Dialog");
const QString ApplicationSettings::Private::configShowPermanentDeleteDialogEntry("Show Permanent Delete Dialog");
const QString ApplicationSettings::Private::configApplySidebarChangesDirectlyEntry("Apply Sidebar Changes Directly");
const QString ApplicationSettings::Private::configSyncBalootoDigikamEntry("Sync Baloo to Digikam");
const QString ApplicationSettings::Private::configSyncDigikamtoBalooEntry("Sync Digikam to Baloo");
const QString ApplicationSettings::Private::configStringComparisonTypeEntry("String Comparison Type");
const QString ApplicationSettings::Private::configFaceDetectionAccuracyEntry("Detection Accuracy");
const QString ApplicationSettings::Private::configApplicationStyleEntry("Application Style");

ApplicationSettings::Private::Private(ApplicationSettings* const q)
    : showSplash(false),
      useTrash(false),
      showTrashDeleteDialog(false),
      showPermanentDeleteDialog(false),
      sidebarApplyDirectly(false),
      iconShowName(false),
      iconShowSize(false),
      iconShowDate(false),
      iconShowModDate(false),
      iconShowTitle(false),
      iconShowComments(false),
      iconShowResolution(false),
      iconShowTags(false),
      iconShowOverlays(false),
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
      tooltipShowVideoAudioCompressor(false),
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
      sidebarTitleStyle(KMultiTabBar::VSNET),
      albumSortOrder(ApplicationSettings::ByFolder),
      albumSortChanged(false),
      imageSortOrder(0),
      imageSorting(0),
      imageGroupMode(0),
      imageGroupSortOrder(0),
      itemLeftClickAction(ApplicationSettings::ShowPreview),
      syncToDigikam(false),
      syncToBaloo(false),
      faceDetectionAccuracy(0.8),
      stringComparisonType(ApplicationSettings::Natural),
      q(q)
{
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

    albumSortOrder                      = ApplicationSettings::ByFolder;
    imageSortOrder                      = ImageSortSettings::SortByFileName;
    imageSorting                        = ImageSortSettings::AscendingOrder;
    imageGroupMode                      = ImageSortSettings::CategoryByAlbum;
    imageGroupSortOrder                 = ImageSortSettings::AscendingOrder;

    itemLeftClickAction                 = ApplicationSettings::ShowPreview;

    thumbnailSize                       = ThumbnailSize::Medium;
    treeThumbnailSize                   = 22;
    treeviewFont                        = KGlobalSettings::generalFont();
    sidebarTitleStyle                   = KMultiTabBar::VSNET;

    ratingFilterCond                    = ImageFilterSettings::GreaterEqualCondition;

    showSplash                          = true;
    useTrash                            = true;
    showTrashDeleteDialog               = true;
    showPermanentDeleteDialog           = true;
    sidebarApplyDirectly                = false;

    iconShowName                        = false;
    iconShowSize                        = false;
    iconShowDate                        = true;
    iconShowModDate                     = true;
    iconShowTitle                       = true;
    iconShowComments                    = true;
    iconShowResolution                  = false;
    iconShowAspectRatio                 = false;
    iconShowTags                        = true;
    iconShowOverlays                    = true;
    iconShowRating                      = true;
    iconShowImageFormat                 = false;
    iconShowCoordinates                 = false;
    iconviewFont                        = KGlobalSettings::generalFont();
    toolTipsFont                        = KGlobalSettings::generalFont();
    showToolTips                        = false;
    tooltipShowFileName                 = true;
    tooltipShowFileDate                 = false;
    tooltipShowFileSize                 = false;
    tooltipShowImageType                = false;
    tooltipShowImageDim                 = true;
    tooltipShowImageAR                  = true;
    tooltipShowPhotoMake                = true;
    tooltipShowPhotoDate                = true;
    tooltipShowPhotoFocal               = true;
    tooltipShowPhotoExpo                = true;
    tooltipShowPhotoMode                = true;
    tooltipShowPhotoFlash               = false;
    tooltipShowPhotoWb                  = false;
    tooltipShowAlbumName                = false;
    tooltipShowTitles                   = false;
    tooltipShowComments                 = true;
    tooltipShowTags                     = true;
    tooltipShowLabelRating              = true;

    tooltipShowVideoAspectRatio         = true;
    tooltipShowVideoAudioBitRate        = true;
    tooltipShowVideoAudioChannelType    = true;
    tooltipShowVideoAudioCompressor     = true;
    tooltipShowVideoDuration            = true;
    tooltipShowVideoFrameRate           = true;
    tooltipShowVideoVideoCodec          = true;

    showAlbumToolTips                   = false;
    tooltipShowAlbumTitle               = true;
    tooltipShowAlbumDate                = true;
    tooltipShowAlbumCollection          = true;
    tooltipShowAlbumCategory            = true;
    tooltipShowAlbumCaption             = true;
    tooltipShowAlbumPreview             = false;

    previewShowIcons                    = true;
    showThumbbar                        = true;

    recursiveAlbums                     = false;
    recursiveTags                       = true;

    showFolderTreeViewItemsCount        = false;

    syncToDigikam                       = false;
    syncToBaloo                         = false;
    albumSortChanged                    = false;

    faceDetectionAccuracy               = 0.8;

    stringComparisonType                = ApplicationSettings::Natural;
    applicationStyle                    = kapp->style()->objectName();

    q->connect(q, SIGNAL(balooSettingsChanged()),
               q, SLOT(applyBalooSettings()));
}

}  // namespace Digikam
