/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : albums settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
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

#include "albumsettings.moc"

// Qt includes

#include <QDBusInterface>
#include <QStyle>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "config-digikam.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "imagefiltersettings.h"
#include "imagesortsettings.h"
#include "mimefilter.h"
#include "thumbnailsize.h"
#include "versionmanager.h"
#include "thememanager.h"

using namespace KDcrawIface;

namespace Digikam
{

class AlbumSettings::Private
{

public:
    Private() :
        showSplash(false),
        useTrash(false),
        showTrashDeleteDialog(false),
        showPermanentDeleteDialog(false),
        sidebarApplyDirectly(false),
        scanAtStart(false),
        iconShowName(false),
        iconShowSize(false),
        iconShowDate(false),
        iconShowModDate(false),
        iconShowComments(false),
        iconShowResolution(false),
        iconShowTags(false),
        iconShowOverlays(false),
        iconShowRating(false),
        iconShowImageFormat(false),
        showToolTips(false),
        tooltipShowFileName(false),
        tooltipShowFileDate(false),
        tooltipShowFileSize(false),
        tooltipShowImageType(false),
        tooltipShowImageDim(false),
        tooltipShowPhotoMake(false),
        tooltipShowPhotoDate(false),
        tooltipShowPhotoFocal(false),
        tooltipShowPhotoExpo(false),
        tooltipShowPhotoMode(false),
        tooltipShowPhotoFlash(false),
        tooltipShowPhotoWb(false),
        tooltipShowAlbumName(false),
        tooltipShowComments(false),
        tooltipShowTags(false),
        tooltipShowLabelRating(false),
        showAlbumToolTips(false),
        tooltipShowAlbumTitle(false),
        tooltipShowAlbumDate(false),
        tooltipShowAlbumCollection(false),
        tooltipShowAlbumCategory(false),
        tooltipShowAlbumCaption(false),
        previewLoadFullImageSize(false),
        previewShowIcons(true),
        showThumbbar(false),
        showFolderTreeViewItemsCount(false),
        treeThumbnailSize(0),
        thumbnailSize(0),
        ratingFilterCond(0),
        recursiveAlbums(false),
        recursiveTags(false),
        imageSortOrder(0),
        imageSorting(0),
        imageGroupMode(0),
        syncToDigikam(false),
        syncToNepomuk(false)
    {
    }

    static const QString                configGroupDefault;
    static const QString                configGroupExif;
    static const QString                configGroupMetadata;
    static const QString                configGroupNepomuk;
    static const QString                configGroupGeneral;
    static const QString                configGroupVersioning;
    static const QString                configGroupFaceDetection;
    static const QString                configAlbumCollectionsEntry;
    static const QString                configAlbumSortOrderEntry;
    static const QString                configImageSortOrderEntry;
    static const QString                configImageSortingEntry;
    static const QString                configImageGroupModeEntry;
    static const QString                configItemLeftClickActionEntry;
    static const QString                configDefaultIconSizeEntry;
    static const QString                configDefaultTreeIconSizeEntry;
    static const QString                configTreeViewFontEntry;
    static const QString                configThemeEntry;
    static const QString                configSidebarTitleStyleEntry;
    static const QString                configRatingFilterConditionEntry;
    static const QString                configRecursiveAlbumsEntry;
    static const QString                configRecursiveTagsEntry;
    static const QString                configIconShowNameEntry;
    static const QString                configIconShowResolutionEntry;
    static const QString                configIconShowSizeEntry;
    static const QString                configIconShowDateEntry;
    static const QString                configIconShowModificationDateEntry;
    static const QString                configIconShowTitleEntry;
    static const QString                configIconShowCommentsEntry;
    static const QString                configIconShowTagsEntry;
    static const QString                configIconShowOverlaysEntry;
    static const QString                configIconShowRatingEntry;
    static const QString                configIconShowImageFormatEntry;
    static const QString                configIconViewFontEntry;
    static const QString                configToolTipsFontEntry;
    static const QString                configShowToolTipsEntry;
    static const QString                configToolTipsShowFileNameEntry;
    static const QString                configToolTipsShowFileDateEntry;
    static const QString                configToolTipsShowFileSizeEntry;
    static const QString                configToolTipsShowImageTypeEntry;
    static const QString                configToolTipsShowImageDimEntry;
    static const QString                configToolTipsShowPhotoMakeEntry;
    static const QString                configToolTipsShowPhotoDateEntry;
    static const QString                configToolTipsShowPhotoFocalEntry;
    static const QString                configToolTipsShowPhotoExpoEntry;
    static const QString                configToolTipsShowPhotoModeEntry;
    static const QString                configToolTipsShowPhotoFlashEntry;
    static const QString                configToolTipsShowPhotoWBEntry;
    static const QString                configToolTipsShowAlbumNameEntry;
    static const QString                configToolTipsShowCommentsEntry;
    static const QString                configToolTipsShowTagsEntry;
    static const QString                configToolTipsShowLabelRatingEntry;
    static const QString                configShowAlbumToolTipsEntry;
    static const QString                configToolTipsShowAlbumTitleEntry;
    static const QString                configToolTipsShowAlbumDateEntry;
    static const QString                configToolTipsShowAlbumCollectionEntry;
    static const QString                configToolTipsShowAlbumCategoryEntry;
    static const QString                configToolTipsShowAlbumCaptionEntry;
    static const QString                configPreviewLoadFullImageSizeEntry;
    static const QString                configPreviewShowIconsEntry;
    static const QString                configShowThumbbarEntry;
    static const QString                configShowFolderTreeViewItemsCountEntry;
    static const QString                configEXIFRotateEntry;
    static const QString                configEXIFSetOrientationEntry;
    static const QString                configSaveTagsEntry;
    static const QString                configSaveTemplateEntry;
    static const QString                configSaveEXIFCommentsEntry;
    static const QString                configSaveDateTimeEntry;
    static const QString                configSaveRatingEntry;
    static const QString                configWriteRAWFilesEntry;
    static const QString                configUpdateFileTimestampEntry;
    static const QString                configShowSplashEntry;
    static const QString                configUseTrashEntry;
    static const QString                configShowTrashDeleteDialogEntry;
    static const QString                configShowPermanentDeleteDialogEntry;
    static const QString                configApplySidebarChangesDirectlyEntry;
    static const QString                configScanAtStartEntry;
    static const QString                configSyncNepomuktoDigikamEntry;
    static const QString                configSyncDigikamtoNepomukEntry;
    static const QString                configStringComparisonTypeEntry;
    static const QString                configFaceDetectionAccuracyEntry;
    static const QString                configApplicationStyleEntry;

    // start up setting
    bool                                showSplash;
    // file ops settings
    bool                                useTrash;
    bool                                showTrashDeleteDialog;
    bool                                showPermanentDeleteDialog;
    // metadata setting
    bool                                sidebarApplyDirectly;
    // database setting
    bool                                scanAtStart;

    // icon view settings
    bool                                iconShowName;
    bool                                iconShowSize;
    bool                                iconShowDate;
    bool                                iconShowModDate;
    bool                                iconShowTitle;
    bool                                iconShowComments;
    bool                                iconShowResolution;
    bool                                iconShowTags;
    bool                                iconShowOverlays;
    bool                                iconShowRating;
    bool                                iconShowImageFormat;

    QFont                               iconviewFont;

    // Icon-view tooltip settings
    bool                                showToolTips;
    bool                                tooltipShowFileName;
    bool                                tooltipShowFileDate;
    bool                                tooltipShowFileSize;
    bool                                tooltipShowImageType;
    bool                                tooltipShowImageDim;
    bool                                tooltipShowPhotoMake;
    bool                                tooltipShowPhotoDate;
    bool                                tooltipShowPhotoFocal;
    bool                                tooltipShowPhotoExpo;
    bool                                tooltipShowPhotoMode;
    bool                                tooltipShowPhotoFlash;
    bool                                tooltipShowPhotoWb;
    bool                                tooltipShowAlbumName;
    bool                                tooltipShowComments;
    bool                                tooltipShowTags;
    bool                                tooltipShowLabelRating;

    QFont                               toolTipsFont;

    // Folder-view tooltip settings
    bool                                showAlbumToolTips;
    bool                                tooltipShowAlbumTitle;
    bool                                tooltipShowAlbumDate;
    bool                                tooltipShowAlbumCollection;
    bool                                tooltipShowAlbumCategory;
    bool                                tooltipShowAlbumCaption;

    // preview settings
    bool                                previewLoadFullImageSize;
    bool                                previewShowIcons;
    bool                                showThumbbar;

    bool                                showFolderTreeViewItemsCount;

    // tree-view settings
    int                                 treeThumbnailSize;
    QFont                               treeviewFont;

    // icon view settings
    int                                 thumbnailSize;
    int                                 ratingFilterCond;
    bool                                recursiveAlbums;
    bool                                recursiveTags;

    // theme settings
    QString                             currentTheme;

    // database settings
    DatabaseParameters                  databaseParams;

    // album settings
    QStringList                         albumCategoryNames;

    KSharedConfigPtr                    config;

    KMultiTabBar::KMultiTabBarStyle     sidebarTitleStyle;

    // album view settings
    AlbumSettings::AlbumSortOrder       albumSortOrder;

    // icon view settings
    int                                 imageSortOrder;
    int                                 imageSorting;
    int                                 imageGroupMode;
    AlbumSettings::ItemLeftClickAction  itemLeftClickAction;

    // nepomuk settings
    bool                                syncToDigikam;
    bool                                syncToNepomuk;

    // versioning settings

    VersionManagerSettings              versionSettings;

    // face detection settings
    double                              faceDetectionAccuracy;

    //misc
    AlbumSettings::StringComparisonType stringComparisonType;
    QString                             applicationStyle;
};

const QString AlbumSettings::Private::configGroupDefault("Album Settings");
const QString AlbumSettings::Private::configGroupExif("EXIF Settings");
const QString AlbumSettings::Private::configGroupMetadata("Metadata Settings");
const QString AlbumSettings::Private::configGroupNepomuk("Nepomuk Settings");
const QString AlbumSettings::Private::configGroupGeneral("General Settings");
const QString AlbumSettings::Private::configGroupVersioning("Versioning Settings");
const QString AlbumSettings::Private::configGroupFaceDetection("Face Detection Settings");
const QString AlbumSettings::Private::configAlbumCollectionsEntry("Album Collections");
const QString AlbumSettings::Private::configAlbumSortOrderEntry("Album Sort Order");
const QString AlbumSettings::Private::configImageSortOrderEntry("Image Sort Order");
const QString AlbumSettings::Private::configImageSortingEntry("Image Sorting");
const QString AlbumSettings::Private::configImageGroupModeEntry("Image Group Mode");
const QString AlbumSettings::Private::configItemLeftClickActionEntry("Item Left Click Action");
const QString AlbumSettings::Private::configDefaultIconSizeEntry("Default Icon Size");
const QString AlbumSettings::Private::configDefaultTreeIconSizeEntry("Default Tree Icon Size");
const QString AlbumSettings::Private::configTreeViewFontEntry("TreeView Font");
const QString AlbumSettings::Private::configThemeEntry("Theme");
const QString AlbumSettings::Private::configSidebarTitleStyleEntry("Sidebar Title Style");
const QString AlbumSettings::Private::configRatingFilterConditionEntry("Rating Filter Condition");
const QString AlbumSettings::Private::configRecursiveAlbumsEntry("Recursive Albums");
const QString AlbumSettings::Private::configRecursiveTagsEntry("Recursive Tags");
const QString AlbumSettings::Private::configIconShowNameEntry("Icon Show Name");
const QString AlbumSettings::Private::configIconShowResolutionEntry("Icon Show Resolution");
const QString AlbumSettings::Private::configIconShowSizeEntry("Icon Show Size");
const QString AlbumSettings::Private::configIconShowDateEntry("Icon Show Date");
const QString AlbumSettings::Private::configIconShowModificationDateEntry("Icon Show Modification Date");
const QString AlbumSettings::Private::configIconShowTitleEntry("Icon Show Title");
const QString AlbumSettings::Private::configIconShowCommentsEntry("Icon Show Comments");
const QString AlbumSettings::Private::configIconShowTagsEntry("Icon Show Tags");
const QString AlbumSettings::Private::configIconShowRatingEntry("Icon Show Rating");
const QString AlbumSettings::Private::configIconShowImageFormatEntry("Icon Show Image Format");
const QString AlbumSettings::Private::configIconShowOverlaysEntry("Icon Show Overlays");
const QString AlbumSettings::Private::configIconViewFontEntry("IconView Font");
const QString AlbumSettings::Private::configToolTipsFontEntry("ToolTips Font");
const QString AlbumSettings::Private::configShowToolTipsEntry("Show ToolTips");
const QString AlbumSettings::Private::configToolTipsShowFileNameEntry("ToolTips Show File Name");
const QString AlbumSettings::Private::configToolTipsShowFileDateEntry("ToolTips Show File Date");
const QString AlbumSettings::Private::configToolTipsShowFileSizeEntry("ToolTips Show File Size");
const QString AlbumSettings::Private::configToolTipsShowImageTypeEntry("ToolTips Show Image Type");
const QString AlbumSettings::Private::configToolTipsShowImageDimEntry("ToolTips Show Image Dim");
const QString AlbumSettings::Private::configToolTipsShowPhotoMakeEntry("ToolTips Show Photo Make");
const QString AlbumSettings::Private::configToolTipsShowPhotoDateEntry("ToolTips Show Photo Date");
const QString AlbumSettings::Private::configToolTipsShowPhotoFocalEntry("ToolTips Show Photo Focal");
const QString AlbumSettings::Private::configToolTipsShowPhotoExpoEntry("ToolTips Show Photo Expo");
const QString AlbumSettings::Private::configToolTipsShowPhotoModeEntry("ToolTips Show Photo Mode");
const QString AlbumSettings::Private::configToolTipsShowPhotoFlashEntry("ToolTips Show Photo Flash");
const QString AlbumSettings::Private::configToolTipsShowPhotoWBEntry("ToolTips Show Photo WB");
const QString AlbumSettings::Private::configToolTipsShowAlbumNameEntry("ToolTips Show Album Name");
const QString AlbumSettings::Private::configToolTipsShowCommentsEntry("ToolTips Show Comments");
const QString AlbumSettings::Private::configToolTipsShowTagsEntry("ToolTips Show Tags");
const QString AlbumSettings::Private::configToolTipsShowLabelRatingEntry("ToolTips Show Label Rating");
const QString AlbumSettings::Private::configShowAlbumToolTipsEntry("Show Album ToolTips");
const QString AlbumSettings::Private::configToolTipsShowAlbumTitleEntry("ToolTips Show Album Title");
const QString AlbumSettings::Private::configToolTipsShowAlbumDateEntry("ToolTips Show Album Date");
const QString AlbumSettings::Private::configToolTipsShowAlbumCollectionEntry("ToolTips Show Album Collection");
const QString AlbumSettings::Private::configToolTipsShowAlbumCategoryEntry("ToolTips Show Album Category");
const QString AlbumSettings::Private::configToolTipsShowAlbumCaptionEntry("ToolTips Show Album Caption");
const QString AlbumSettings::Private::configPreviewLoadFullImageSizeEntry("Preview Load Full Image Size");
const QString AlbumSettings::Private::configPreviewShowIconsEntry("Preview Show Icons");
const QString AlbumSettings::Private::configShowThumbbarEntry("Show Thumbbar");
const QString AlbumSettings::Private::configShowFolderTreeViewItemsCountEntry("Show Folder Tree View Items Count");
const QString AlbumSettings::Private::configEXIFRotateEntry("EXIF Rotate");
const QString AlbumSettings::Private::configEXIFSetOrientationEntry("EXIF Set Orientation");
const QString AlbumSettings::Private::configSaveTagsEntry("Save Tags");
const QString AlbumSettings::Private::configSaveTemplateEntry("Save Template");
const QString AlbumSettings::Private::configSaveEXIFCommentsEntry("Save EXIF Comments");
const QString AlbumSettings::Private::configSaveDateTimeEntry("Save Date Time");
const QString AlbumSettings::Private::configSaveRatingEntry("Save Rating");
const QString AlbumSettings::Private::configWriteRAWFilesEntry("Write RAW Files");
const QString AlbumSettings::Private::configUpdateFileTimestampEntry("Update File Timestamp");
const QString AlbumSettings::Private::configShowSplashEntry("Show Splash");
const QString AlbumSettings::Private::configUseTrashEntry("Use Trash");
const QString AlbumSettings::Private::configShowTrashDeleteDialogEntry("Show Trash Delete Dialog");
const QString AlbumSettings::Private::configShowPermanentDeleteDialogEntry("Show Permanent Delete Dialog");
const QString AlbumSettings::Private::configApplySidebarChangesDirectlyEntry("Apply Sidebar Changes Directly");
const QString AlbumSettings::Private::configScanAtStartEntry("Scan At Start");
const QString AlbumSettings::Private::configSyncNepomuktoDigikamEntry("Sync Nepomuk to Digikam");
const QString AlbumSettings::Private::configSyncDigikamtoNepomukEntry("Sync Digikam to Nepomuk");
const QString AlbumSettings::Private::configStringComparisonTypeEntry("String Comparison Type");
const QString AlbumSettings::Private::configFaceDetectionAccuracyEntry("Detection Accuracy");
const QString AlbumSettings::Private::configApplicationStyleEntry("Application Style");

// -------------------------------------------------------------------------------------------------

class AlbumSettingsCreator
{
public:

    AlbumSettings object;
};

K_GLOBAL_STATIC(AlbumSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

AlbumSettings* AlbumSettings::instance()
{
    return &creator->object;
}

AlbumSettings::AlbumSettings()
    : QObject(), d(new Private)
{
    d->config = KGlobal::config();
    init();
    readSettings();
}

AlbumSettings::~AlbumSettings()
{
    delete d;
}

void AlbumSettings::init()
{
    d->albumCategoryNames.clear();
    d->albumCategoryNames.append(i18n("Category"));
    d->albumCategoryNames.append(i18n("Travel"));
    d->albumCategoryNames.append(i18n("Holidays"));
    d->albumCategoryNames.append(i18n("Friends"));
    d->albumCategoryNames.append(i18n("Nature"));
    d->albumCategoryNames.append(i18n("Party"));
    d->albumCategoryNames.append(i18n("Todo"));
    d->albumCategoryNames.append(i18n("Miscellaneous"));
    d->albumCategoryNames.sort();

    d->albumSortOrder               = AlbumSettings::ByFolder;
    d->imageSortOrder               = ImageSortSettings::SortByFileName;
    d->imageSorting                 = ImageSortSettings::AscendingOrder;
    d->imageGroupMode               = ImageSortSettings::CategoryByAlbum;
    d->itemLeftClickAction          = AlbumSettings::ShowPreview;

    d->thumbnailSize                = ThumbnailSize::Medium;
    d->treeThumbnailSize            = 22;
    d->treeviewFont                 = KGlobalSettings::generalFont();
    d->sidebarTitleStyle            = KMultiTabBar::VSNET;

    d->ratingFilterCond             = ImageFilterSettings::GreaterEqualCondition;

    d->showSplash                   = true;
    d->useTrash                     = true;
    d->showTrashDeleteDialog        = true;
    d->showPermanentDeleteDialog    = true;
    d->sidebarApplyDirectly         = false;

    d->iconShowName                 = false;
    d->iconShowSize                 = false;
    d->iconShowDate                 = true;
    d->iconShowModDate              = true;
    d->iconShowTitle                = true;
    d->iconShowComments             = true;
    d->iconShowResolution           = false;
    d->iconShowTags                 = true;
    d->iconShowOverlays             = true;
    d->iconShowRating               = true;
    d->iconShowImageFormat          = false;
    d->iconviewFont                 = KGlobalSettings::generalFont();

    d->toolTipsFont                 = KGlobalSettings::generalFont();
    d->showToolTips                 = false;
    d->tooltipShowFileName          = true;
    d->tooltipShowFileDate          = false;
    d->tooltipShowFileSize          = false;
    d->tooltipShowImageType         = false;
    d->tooltipShowImageDim          = true;
    d->tooltipShowPhotoMake         = true;
    d->tooltipShowPhotoDate         = true;
    d->tooltipShowPhotoFocal        = true;
    d->tooltipShowPhotoExpo         = true;
    d->tooltipShowPhotoMode         = true;
    d->tooltipShowPhotoFlash        = false;
    d->tooltipShowPhotoWb           = false;
    d->tooltipShowAlbumName         = false;
    d->tooltipShowComments          = true;
    d->tooltipShowTags              = true;
    d->tooltipShowLabelRating       = true;

    d->showAlbumToolTips            = false;
    d->tooltipShowAlbumTitle        = true;
    d->tooltipShowAlbumDate         = true;
    d->tooltipShowAlbumCollection   = true;
    d->tooltipShowAlbumCategory     = true;
    d->tooltipShowAlbumCaption      = true;

    d->previewLoadFullImageSize     = false;
    d->previewShowIcons             = true;
    d->showThumbbar                 = true;

    d->recursiveAlbums              = false;
    d->recursiveTags                = true;

    d->showFolderTreeViewItemsCount = false;

    d->syncToDigikam                = false;
    d->syncToNepomuk                = false;

    d->faceDetectionAccuracy        = 0.8;

    d->stringComparisonType         = AlbumSettings::Natural;
    d->applicationStyle             = kapp->style()->objectName();

    connect(this, SIGNAL(nepomukSettingsChanged()),
            this, SLOT(applyNepomukSettings()));
}

void AlbumSettings::readSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group  = config->group(d->configGroupDefault);

    QStringList collectionList = group.readEntry(d->configAlbumCollectionsEntry, QStringList());

    if (!collectionList.isEmpty())
    {
        collectionList.sort();
        d->albumCategoryNames = collectionList;
    }

    d->albumSortOrder = AlbumSettings::AlbumSortOrder(group.readEntry(d->configAlbumSortOrderEntry,
                                                                      (int)AlbumSettings::ByFolder));

    d->imageSortOrder               = group.readEntry(d->configImageSortOrderEntry, (int)ImageSortSettings::SortByFileName);
    d->imageSorting                 = group.readEntry(d->configImageSortingEntry,   (int)ImageSortSettings::AscendingOrder);
    d->imageGroupMode               = group.readEntry(d->configImageGroupModeEntry, (int)ImageSortSettings::CategoryByAlbum);

    d->itemLeftClickAction          = AlbumSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                                                         (int)AlbumSettings::ShowPreview));

    d->thumbnailSize                = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);
    d->treeThumbnailSize            = group.readEntry(d->configDefaultTreeIconSizeEntry,          22);
    d->treeviewFont                 = group.readEntry(d->configTreeViewFontEntry,                 KGlobalSettings::generalFont());
    d->currentTheme                 = group.readEntry(d->configThemeEntry,                        ThemeManager::instance()->defaultThemeName());

    d->sidebarTitleStyle            = (KMultiTabBar::KMultiTabBarStyle)group.readEntry(d->configSidebarTitleStyleEntry,
                                      (int)KMultiTabBar::VSNET);

    d->ratingFilterCond             = group.readEntry(d->configRatingFilterConditionEntry,
                                      (int)ImageFilterSettings::GreaterEqualCondition);

    d->recursiveAlbums              = group.readEntry(d->configRecursiveAlbumsEntry,              false);
    d->recursiveTags                = group.readEntry(d->configRecursiveTagsEntry,                true);


    d->iconShowName                 = group.readEntry(d->configIconShowNameEntry,                 false);
    d->iconShowResolution           = group.readEntry(d->configIconShowResolutionEntry,           false);
    d->iconShowSize                 = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                 = group.readEntry(d->configIconShowDateEntry,                 true);
    d->iconShowModDate              = group.readEntry(d->configIconShowModificationDateEntry,     true);
    d->iconShowTitle                = group.readEntry(d->configIconShowTitleEntry,                true);
    d->iconShowComments             = group.readEntry(d->configIconShowCommentsEntry,             true);
    d->iconShowTags                 = group.readEntry(d->configIconShowTagsEntry,                 true);
    d->iconShowOverlays             = group.readEntry(d->configIconShowOverlaysEntry,             true);
    d->iconShowRating               = group.readEntry(d->configIconShowRatingEntry,               true);
    d->iconShowImageFormat          = group.readEntry(d->configIconShowImageFormatEntry,          false);
    d->iconviewFont                 = group.readEntry(d->configIconViewFontEntry,                 KGlobalSettings::generalFont());

    d->toolTipsFont                 = group.readEntry(d->configToolTipsFontEntry,                 KGlobalSettings::generalFont());
    d->showToolTips                 = group.readEntry(d->configShowToolTipsEntry,                 false);
    d->tooltipShowFileName          = group.readEntry(d->configToolTipsShowFileNameEntry,         true);
    d->tooltipShowFileDate          = group.readEntry(d->configToolTipsShowFileDateEntry,         false);
    d->tooltipShowFileSize          = group.readEntry(d->configToolTipsShowFileSizeEntry,         false);
    d->tooltipShowImageType         = group.readEntry(d->configToolTipsShowImageTypeEntry,        false);
    d->tooltipShowImageDim          = group.readEntry(d->configToolTipsShowImageDimEntry,         true);
    d->tooltipShowPhotoMake         = group.readEntry(d->configToolTipsShowPhotoMakeEntry,        true);
    d->tooltipShowPhotoDate         = group.readEntry(d->configToolTipsShowPhotoDateEntry,        true);
    d->tooltipShowPhotoFocal        = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo         = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
    d->tooltipShowPhotoMode         = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
    d->tooltipShowPhotoFlash        = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb           = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowAlbumName         = group.readEntry(d->configToolTipsShowAlbumNameEntry,        false);
    d->tooltipShowComments          = group.readEntry(d->configToolTipsShowCommentsEntry,         true);
    d->tooltipShowTags              = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    d->tooltipShowLabelRating       = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->showAlbumToolTips            = group.readEntry(d->configShowAlbumToolTipsEntry,            false);
    d->tooltipShowAlbumTitle        = group.readEntry(d->configToolTipsShowAlbumTitleEntry,       true);
    d->tooltipShowAlbumDate         = group.readEntry(d->configToolTipsShowAlbumDateEntry,        true);
    d->tooltipShowAlbumCollection   = group.readEntry(d->configToolTipsShowAlbumCollectionEntry,  true);
    d->tooltipShowAlbumCategory     = group.readEntry(d->configToolTipsShowAlbumCategoryEntry,    true);
    d->tooltipShowAlbumCaption      = group.readEntry(d->configToolTipsShowAlbumCaptionEntry,     true);

    d->previewLoadFullImageSize     = group.readEntry(d->configPreviewLoadFullImageSizeEntry,     false);
    d->previewShowIcons             = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->showThumbbar                 = group.readEntry(d->configShowThumbbarEntry,                 true);

    d->showFolderTreeViewItemsCount = group.readEntry(d->configShowFolderTreeViewItemsCountEntry, false);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupGeneral);

    d->showSplash                = group.readEntry(d->configShowSplashEntry,                      true);
    d->useTrash                  = group.readEntry(d->configUseTrashEntry,                        true);
    d->showTrashDeleteDialog     = group.readEntry(d->configShowTrashDeleteDialogEntry,           true);
    d->showPermanentDeleteDialog = group.readEntry(d->configShowPermanentDeleteDialogEntry,       true);
    d->sidebarApplyDirectly      = group.readEntry(d->configApplySidebarChangesDirectlyEntry,     false);
    d->scanAtStart               = group.readEntry(d->configScanAtStartEntry,                     true);
    d->stringComparisonType      = (StringComparisonType) group.readEntry(d->configStringComparisonTypeEntry, (int) Natural);

    // ---------------------------------------------------------------------

    d->databaseParams.readFromConfig();

#ifdef HAVE_NEPOMUK

    group = config->group(d->configGroupNepomuk);

    d->syncToDigikam         = group.readEntry(d->configSyncNepomuktoDigikamEntry, false);
    d->syncToNepomuk         = group.readEntry(d->configSyncDigikamtoNepomukEntry, false);

#endif // HAVE_NEPOMUK

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.readFromConfig(group);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupFaceDetection);

    d->faceDetectionAccuracy = group.readEntry(d->configFaceDetectionAccuracyEntry, double(0.8));

    setApplicationStyle(group.readEntry(d->configApplicationStyleEntry, kapp->style()->objectName()));

    emit setupChanged();
    emit recurseSettingsChanged();
    emit nepomukSettingsChanged();
}

void AlbumSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configAlbumCollectionsEntry,             d->albumCategoryNames);
    group.writeEntry(d->configAlbumSortOrderEntry,               (int)d->albumSortOrder);
    group.writeEntry(d->configImageSortOrderEntry,               (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortingEntry,                 (int)d->imageSorting);
    group.writeEntry(d->configImageGroupModeEntry,               (int)d->imageGroupMode);
    group.writeEntry(d->configItemLeftClickActionEntry,          (int)d->itemLeftClickAction);
    group.writeEntry(d->configDefaultIconSizeEntry,              QString::number(d->thumbnailSize));
    group.writeEntry(d->configDefaultTreeIconSizeEntry,          QString::number(d->treeThumbnailSize));
    group.writeEntry(d->configTreeViewFontEntry,                 d->treeviewFont);
    group.writeEntry(d->configRatingFilterConditionEntry,        d->ratingFilterCond);
    group.writeEntry(d->configRecursiveAlbumsEntry,              d->recursiveAlbums);
    group.writeEntry(d->configRecursiveTagsEntry,                d->recursiveTags);
    group.writeEntry(d->configThemeEntry,                        d->currentTheme);
    group.writeEntry(d->configSidebarTitleStyleEntry,            (int)d->sidebarTitleStyle);

    group.writeEntry(d->configIconShowNameEntry,                 d->iconShowName);
    group.writeEntry(d->configIconShowResolutionEntry,           d->iconShowResolution);
    group.writeEntry(d->configIconShowSizeEntry,                 d->iconShowSize);
    group.writeEntry(d->configIconShowDateEntry,                 d->iconShowDate);
    group.writeEntry(d->configIconShowModificationDateEntry,     d->iconShowModDate);
    group.writeEntry(d->configIconShowTitleEntry,                d->iconShowTitle);
    group.writeEntry(d->configIconShowCommentsEntry,             d->iconShowComments);
    group.writeEntry(d->configIconShowTagsEntry,                 d->iconShowTags);
    group.writeEntry(d->configIconShowOverlaysEntry,             d->iconShowOverlays);
    group.writeEntry(d->configIconShowRatingEntry,               d->iconShowRating);
    group.writeEntry(d->configIconShowImageFormatEntry,          d->iconShowImageFormat);
    group.writeEntry(d->configIconViewFontEntry,                 d->iconviewFont);

    group.writeEntry(d->configToolTipsFontEntry,                 d->toolTipsFont);
    group.writeEntry(d->configShowToolTipsEntry,                 d->showToolTips);
    group.writeEntry(d->configToolTipsShowFileNameEntry,         d->tooltipShowFileName);
    group.writeEntry(d->configToolTipsShowFileDateEntry,         d->tooltipShowFileDate);
    group.writeEntry(d->configToolTipsShowFileSizeEntry,         d->tooltipShowFileSize);
    group.writeEntry(d->configToolTipsShowImageTypeEntry,        d->tooltipShowImageType);
    group.writeEntry(d->configToolTipsShowImageDimEntry,         d->tooltipShowImageDim);
    group.writeEntry(d->configToolTipsShowPhotoMakeEntry,        d->tooltipShowPhotoMake);
    group.writeEntry(d->configToolTipsShowPhotoDateEntry,        d->tooltipShowPhotoDate);
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,       d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,        d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoModeEntry,        d->tooltipShowPhotoMode);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,       d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,          d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowAlbumNameEntry,        d->tooltipShowAlbumName);
    group.writeEntry(d->configToolTipsShowCommentsEntry,         d->tooltipShowComments);
    group.writeEntry(d->configToolTipsShowTagsEntry,             d->tooltipShowTags);
    group.writeEntry(d->configToolTipsShowLabelRatingEntry,      d->tooltipShowLabelRating);

    group.writeEntry(d->configShowAlbumToolTipsEntry,            d->showAlbumToolTips);
    group.writeEntry(d->configToolTipsShowAlbumTitleEntry,       d->tooltipShowAlbumTitle);
    group.writeEntry(d->configToolTipsShowAlbumDateEntry,        d->tooltipShowAlbumDate);
    group.writeEntry(d->configToolTipsShowAlbumCollectionEntry,  d->tooltipShowAlbumCollection);
    group.writeEntry(d->configToolTipsShowAlbumCategoryEntry,    d->tooltipShowAlbumCategory);
    group.writeEntry(d->configToolTipsShowAlbumCaptionEntry,     d->tooltipShowAlbumCaption);

    group.writeEntry(d->configPreviewLoadFullImageSizeEntry,     d->previewLoadFullImageSize);
    group.writeEntry(d->configPreviewShowIconsEntry,             d->previewShowIcons);
    group.writeEntry(d->configShowThumbbarEntry,                 d->showThumbbar);

    group.writeEntry(d->configShowFolderTreeViewItemsCountEntry, d->showFolderTreeViewItemsCount);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupGeneral);

    group.writeEntry(d->configShowSplashEntry,                   d->showSplash);
    group.writeEntry(d->configUseTrashEntry,                     d->useTrash);
    group.writeEntry(d->configShowTrashDeleteDialogEntry,        d->showTrashDeleteDialog);
    group.writeEntry(d->configShowPermanentDeleteDialogEntry,    d->showPermanentDeleteDialog);
    group.writeEntry(d->configApplySidebarChangesDirectlyEntry,  d->sidebarApplyDirectly);
    group.writeEntry(d->configScanAtStartEntry,                  d->scanAtStart);
    group.writeEntry(d->configStringComparisonTypeEntry,         (int) d->stringComparisonType);

    // ---------------------------------------------------------------------

    d->databaseParams.writeToConfig();

#ifdef HAVE_NEPOMUK

    group = config->group(d->configGroupNepomuk);

    group.writeEntry(d->configSyncNepomuktoDigikamEntry, d->syncToDigikam);
    group.writeEntry(d->configSyncDigikamtoNepomukEntry, d->syncToNepomuk);

#endif // HAVE_NEPOMUK

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.writeToConfig(group);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupFaceDetection);

    group.writeEntry(d->configFaceDetectionAccuracyEntry, d->faceDetectionAccuracy);
    group.writeEntry(d->configApplicationStyleEntry,      d->applicationStyle);

    config->sync();
}

void AlbumSettings::emitSetupChanged()
{
    emit setupChanged();
}

QString AlbumSettings::getDatabaseFilePath() const
{
    return d->databaseParams.getDatabaseNameOrDir();
}

void AlbumSettings::setDatabaseFilePath(const QString& path)
{
    d->databaseParams.setDatabasePath(path);
    d->databaseParams.setThumbsDatabasePath(path);
}

void AlbumSettings::setShowSplashScreen(bool val)
{
    d->showSplash = val;
}

bool AlbumSettings::getShowSplashScreen() const
{
    return d->showSplash;
}

void AlbumSettings::setScanAtStart(bool val)
{
    d->scanAtStart = val;
}

bool AlbumSettings::getScanAtStart() const
{
    return d->scanAtStart;
}

void AlbumSettings::setAlbumCategoryNames(const QStringList& list)
{
    d->albumCategoryNames = list;
}

QStringList AlbumSettings::getAlbumCategoryNames() const
{
    return d->albumCategoryNames;
}

bool AlbumSettings::addAlbumCategoryName(const QString& name)
{
    if (d->albumCategoryNames.contains(name))
    {
        return false;
    }

    d->albumCategoryNames.append(name);
    return true;
}

bool AlbumSettings::delAlbumCategoryName(const QString& name)
{
    uint count = d->albumCategoryNames.removeAll(name);
    return (count > 0) ? true : false;
}

void AlbumSettings::setAlbumSortOrder(const AlbumSettings::AlbumSortOrder order)
{
    d->albumSortOrder = order;
}

AlbumSettings::AlbumSortOrder AlbumSettings::getAlbumSortOrder() const
{
    return d->albumSortOrder;
}

void AlbumSettings::setImageSortOrder(int order)
{
    d->imageSortOrder = order;
}

int AlbumSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
}

void AlbumSettings::setImageSorting(int sorting)
{
    d->imageSorting = sorting;
}

int AlbumSettings::getImageSorting() const
{
    return d->imageSorting;
}

void AlbumSettings::setImageGroupMode(int mode)
{
    d->imageGroupMode = mode;
}

int AlbumSettings::getImageGroupMode() const
{
    return d->imageGroupMode;
}

void AlbumSettings::setItemLeftClickAction(const ItemLeftClickAction action)
{
    d->itemLeftClickAction = action;
}

AlbumSettings::ItemLeftClickAction AlbumSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

QString AlbumSettings::getImageFileFilter() const
{
    QStringList imageSettings;
    DatabaseAccess().db()->getFilterSettings(&imageSettings, 0, 0);
    QStringList wildcards;

    foreach(const QString& suffix, imageSettings)
    {
        wildcards << "*." + suffix;
    }

    return wildcards.join(" ");
}

QString AlbumSettings::getMovieFileFilter() const
{
    QStringList movieSettings;
    DatabaseAccess().db()->getFilterSettings(0, &movieSettings, 0);
    QStringList wildcards;

    foreach(const QString& suffix, movieSettings)
    {
        wildcards << "*." + suffix;
    }

    return wildcards.join(" ");
}

QString AlbumSettings::getAudioFileFilter() const
{
    QStringList audioSettings;
    DatabaseAccess().db()->getFilterSettings(0, 0, &audioSettings);
    QStringList wildcards;

    foreach(const QString& suffix, audioSettings)
    {
        wildcards << "*." + suffix;
    }

    return wildcards.join(" ");
}

QString AlbumSettings::getRawFileFilter() const
{
    QStringList supportedRaws = KDcraw::rawFilesList();
    QStringList imageSettings;
    DatabaseAccess().db()->getFilterSettings(&imageSettings, 0, 0);

    // form intersection: those extensions that are supported as RAW as well in the list of allowed extensions
    for (QStringList::iterator it = supportedRaws.begin(); it != supportedRaws.end(); )
    {
        if (imageSettings.contains(*it))
        {
            ++it;
        }
        else
        {
            it = supportedRaws.erase(it);
        }
    }

    QStringList wildcards;

    foreach(const QString& suffix, supportedRaws)
    {
        wildcards << "*." + suffix;
    }

    return wildcards.join(" ");
}

QString AlbumSettings::getAllFileFilter() const
{
    QStringList imageFilter, audioFilter, videoFilter;
    DatabaseAccess().db()->getFilterSettings(&imageFilter, &audioFilter, &videoFilter);
    QStringList wildcards;
    foreach(const QString& suffix, imageFilter)
    {
        wildcards << "*." + suffix;
    }
    foreach(const QString& suffix, audioFilter)
    {
        wildcards << "*." + suffix;
    }
    foreach(const QString& suffix, videoFilter)
    {
        wildcards << "*." + suffix;
    }
    return wildcards.join(" ");
}

void AlbumSettings::addToImageFileFilter(const QString& extensions)
{
    DatabaseAccess().db()->addToUserImageFilterSettings(extensions);
}

void AlbumSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int AlbumSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void AlbumSettings::setTreeViewIconSize(int val)
{
    d->treeThumbnailSize = val;
}

int AlbumSettings::getTreeViewIconSize() const
{
    return ((d->treeThumbnailSize < 8) || (d->treeThumbnailSize > 48)) ? 48 : d->treeThumbnailSize;
}

void AlbumSettings::setTreeViewFont(const QFont& font)
{
    d->treeviewFont = font;
}

QFont AlbumSettings::getTreeViewFont() const
{
    return d->treeviewFont;
}

void AlbumSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont AlbumSettings::getIconViewFont() const
{
    return d->iconviewFont;
}

void AlbumSettings::setRatingFilterCond(int val)
{
    d->ratingFilterCond = val;
}

int AlbumSettings::getRatingFilterCond() const
{
    return d->ratingFilterCond;
}

void AlbumSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool AlbumSettings::getIconShowName() const
{
    return d->iconShowName;
}

void AlbumSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool AlbumSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void AlbumSettings::setIconShowTitle(bool val)
{
    d->iconShowTitle = val;
}

bool AlbumSettings::getIconShowTitle() const
{
    return d->iconShowTitle;
}

void AlbumSettings::setIconShowComments(bool val)
{
    d->iconShowComments = val;
}

bool AlbumSettings::getIconShowComments() const
{
    return d->iconShowComments;
}

void AlbumSettings::setIconShowResolution(bool val)
{
    d->iconShowResolution = val;
}

bool AlbumSettings::getIconShowResolution() const
{
    return d->iconShowResolution;
}

void AlbumSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;
}

bool AlbumSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void AlbumSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool AlbumSettings::getIconShowDate() const
{
    return d->iconShowDate;
}

void AlbumSettings::setIconShowModDate(bool val)
{
    d->iconShowModDate = val;
}

bool AlbumSettings::getIconShowModDate() const
{
    return d->iconShowModDate;
}

void AlbumSettings::setIconShowRating(bool val)
{
    d->iconShowRating = val;
}

bool AlbumSettings::getIconShowRating() const
{
    return d->iconShowRating;
}

void AlbumSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool AlbumSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

void AlbumSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool AlbumSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

void AlbumSettings::setToolTipsFont(const QFont& font)
{
    d->toolTipsFont = font;
}

QFont AlbumSettings::getToolTipsFont() const
{
    return d->toolTipsFont;
}

void AlbumSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool AlbumSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void AlbumSettings::setToolTipsShowFileName(bool val)
{
    d->tooltipShowFileName = val;
}

bool AlbumSettings::getToolTipsShowFileName() const
{
    return d->tooltipShowFileName;
}

void AlbumSettings::setToolTipsShowFileDate(bool val)
{
    d->tooltipShowFileDate = val;
}

bool AlbumSettings::getToolTipsShowFileDate() const
{
    return d->tooltipShowFileDate;
}

void AlbumSettings::setToolTipsShowFileSize(bool val)
{
    d->tooltipShowFileSize = val;
}

bool AlbumSettings::getToolTipsShowFileSize() const
{
    return d->tooltipShowFileSize;
}

void AlbumSettings::setToolTipsShowImageType(bool val)
{
    d->tooltipShowImageType = val;
}

bool AlbumSettings::getToolTipsShowImageType() const
{
    return d->tooltipShowImageType;
}

void AlbumSettings::setToolTipsShowImageDim(bool val)
{
    d->tooltipShowImageDim = val;
}

bool AlbumSettings::getToolTipsShowImageDim() const
{
    return d->tooltipShowImageDim;
}

void AlbumSettings::setToolTipsShowPhotoMake(bool val)
{
    d->tooltipShowPhotoMake = val;
}

bool AlbumSettings::getToolTipsShowPhotoMake() const
{
    return d->tooltipShowPhotoMake;
}

void AlbumSettings::setToolTipsShowPhotoDate(bool val)
{
    d->tooltipShowPhotoDate = val;
}

bool AlbumSettings::getToolTipsShowPhotoDate() const
{
    return d->tooltipShowPhotoDate;
}

void AlbumSettings::setToolTipsShowPhotoFocal(bool val)
{
    d->tooltipShowPhotoFocal = val;
}

bool AlbumSettings::getToolTipsShowPhotoFocal() const
{
    return d->tooltipShowPhotoFocal;
}

void AlbumSettings::setToolTipsShowPhotoExpo(bool val)
{
    d->tooltipShowPhotoExpo = val;
}

bool AlbumSettings::getToolTipsShowPhotoExpo() const
{
    return d->tooltipShowPhotoExpo;
}

void AlbumSettings::setToolTipsShowPhotoMode(bool val)
{
    d->tooltipShowPhotoMode = val;
}

bool AlbumSettings::getToolTipsShowPhotoMode() const
{
    return d->tooltipShowPhotoMode;
}

void AlbumSettings::setToolTipsShowPhotoFlash(bool val)
{
    d->tooltipShowPhotoFlash = val;
}

bool AlbumSettings::getToolTipsShowPhotoFlash() const
{
    return d->tooltipShowPhotoFlash;
}

void AlbumSettings::setToolTipsShowPhotoWB(bool val)
{
    d->tooltipShowPhotoWb = val;
}

bool AlbumSettings::getToolTipsShowPhotoWB() const
{
    return d->tooltipShowPhotoWb;
}

void AlbumSettings::setToolTipsShowAlbumName(bool val)
{
    d->tooltipShowAlbumName = val;
}

bool AlbumSettings::getToolTipsShowAlbumName() const
{
    return d->tooltipShowAlbumName;
}

void AlbumSettings::setToolTipsShowComments(bool val)
{
    d->tooltipShowComments = val;
}

bool AlbumSettings::getToolTipsShowComments() const
{
    return d->tooltipShowComments;
}

void AlbumSettings::setToolTipsShowTags(bool val)
{
    d->tooltipShowTags = val;
}

bool AlbumSettings::getToolTipsShowTags() const
{
    return d->tooltipShowTags;
}

void AlbumSettings::setToolTipsShowLabelRating(bool val)
{
    d->tooltipShowLabelRating = val;
}

bool AlbumSettings::getToolTipsShowLabelRating() const
{
    return d->tooltipShowLabelRating;
}

void AlbumSettings::setShowAlbumToolTips(bool val)
{
    d->showAlbumToolTips = val;
}

bool AlbumSettings::getShowAlbumToolTips() const
{
    return d->showAlbumToolTips;
}

void AlbumSettings::setToolTipsShowAlbumTitle(bool val)
{
    d->tooltipShowAlbumTitle = val;
}

bool AlbumSettings::getToolTipsShowAlbumTitle() const
{
    return d->tooltipShowAlbumTitle;
}

void AlbumSettings::setToolTipsShowAlbumDate(bool val)
{
    d->tooltipShowAlbumDate = val;
}

bool AlbumSettings::getToolTipsShowAlbumDate() const
{
    return d->tooltipShowAlbumDate;
}

void AlbumSettings::setToolTipsShowAlbumCollection(bool val)
{
    d->tooltipShowAlbumCollection = val;
}

bool AlbumSettings::getToolTipsShowAlbumCollection() const
{
    return d->tooltipShowAlbumCollection;
}

void AlbumSettings::setToolTipsShowAlbumCategory(bool val)
{
    d->tooltipShowAlbumCategory = val;
}

bool AlbumSettings::getToolTipsShowAlbumCategory() const
{
    return d->tooltipShowAlbumCategory;
}

void AlbumSettings::setToolTipsShowAlbumCaption(bool val)
{
    d->tooltipShowAlbumCaption = val;
}

bool AlbumSettings::getToolTipsShowAlbumCaption() const
{
    return d->tooltipShowAlbumCaption;
}

void AlbumSettings::setCurrentTheme(const QString& theme)
{
    d->currentTheme = theme;
}

QString AlbumSettings::getCurrentTheme() const
{
    return d->currentTheme;
}

void AlbumSettings::setSidebarTitleStyle(KMultiTabBar::KMultiTabBarStyle style)
{
    d->sidebarTitleStyle = style;
}

KMultiTabBar::KMultiTabBarStyle AlbumSettings::getSidebarTitleStyle() const
{
    return d->sidebarTitleStyle;
}

void AlbumSettings::setUseTrash(bool val)
{
    d->useTrash = val;
}

bool AlbumSettings::getUseTrash() const
{
    return d->useTrash;
}

void AlbumSettings::setShowTrashDeleteDialog(bool val)
{
    d->showTrashDeleteDialog = val;
}

bool AlbumSettings::getShowTrashDeleteDialog() const
{
    return d->showTrashDeleteDialog;
}

void AlbumSettings::setShowPermanentDeleteDialog(bool val)
{
    d->showPermanentDeleteDialog = val;
}

bool AlbumSettings::getShowPermanentDeleteDialog() const
{
    return d->showPermanentDeleteDialog;
}

void AlbumSettings::setApplySidebarChangesDirectly(bool val)
{
    d->sidebarApplyDirectly= val;
}

bool AlbumSettings::getApplySidebarChangesDirectly() const
{
    return d->sidebarApplyDirectly;
}

bool AlbumSettings::showToolTipsIsValid() const
{
    if (d->showToolTips)
    {
        if (d->tooltipShowFileName   ||
            d->tooltipShowFileDate   ||
            d->tooltipShowFileSize   ||
            d->tooltipShowImageType  ||
            d->tooltipShowImageDim   ||
            d->tooltipShowPhotoMake  ||
            d->tooltipShowPhotoDate  ||
            d->tooltipShowPhotoFocal ||
            d->tooltipShowPhotoExpo  ||
            d->tooltipShowPhotoMode  ||
            d->tooltipShowPhotoFlash ||
            d->tooltipShowPhotoWb    ||
            d->tooltipShowAlbumName  ||
            d->tooltipShowComments   ||
            d->tooltipShowTags       ||
            d->tooltipShowLabelRating)
        {
            return true;
        }
    }

    return false;
}

bool AlbumSettings::showAlbumToolTipsIsValid() const
{
    if (d->showAlbumToolTips)
    {
        if (
            d->tooltipShowAlbumTitle      ||
            d->tooltipShowAlbumDate       ||
            d->tooltipShowAlbumCollection ||
            d->tooltipShowAlbumCaption    ||
            d->tooltipShowAlbumCategory
        )
        {
            return true;
        }
    }

    return false;
}

void AlbumSettings::setPreviewLoadFullImageSize(bool val)
{
    d->previewLoadFullImageSize = val;
}

bool AlbumSettings::getPreviewLoadFullImageSize() const
{
    return d->previewLoadFullImageSize;
}

void AlbumSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool AlbumSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

void AlbumSettings::setRecurseAlbums(bool val)
{
    d->recursiveAlbums = val;
    emit recurseSettingsChanged();
}

bool AlbumSettings::getRecurseAlbums() const
{
    return d->recursiveAlbums;
}

void AlbumSettings::setRecurseTags(bool val)
{
    d->recursiveTags = val;
    emit recurseSettingsChanged();
}

bool AlbumSettings::getRecurseTags() const
{
    return d->recursiveTags;
}

void AlbumSettings::setShowFolderTreeViewItemsCount(bool val)
{
    d->showFolderTreeViewItemsCount = val;
}

bool AlbumSettings::getShowFolderTreeViewItemsCount() const
{
    return d->showFolderTreeViewItemsCount;
}

void AlbumSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool AlbumSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void AlbumSettings::setSyncNepomukToDigikam(bool val)
{
    d->syncToDigikam = val;
    emit nepomukSettingsChanged();
}

bool AlbumSettings::getSyncNepomukToDigikam() const
{
    return d->syncToDigikam;
}

void AlbumSettings::setSyncDigikamToNepomuk(bool val)
{
    d->syncToNepomuk = val;
    emit nepomukSettingsChanged();
}

bool AlbumSettings::getSyncDigikamToNepomuk() const
{
    return d->syncToNepomuk;
}

DatabaseParameters AlbumSettings::getDatabaseParameters() const
{
    return d->databaseParams;
}

void AlbumSettings::setDatabaseParameters(const DatabaseParameters& params)
{
    d->databaseParams = params;
}

QString AlbumSettings::getDatabaseType() const
{
    return d->databaseParams.databaseType;
}

void AlbumSettings::setDatabaseType(const QString& databaseType)
{
    d->databaseParams.databaseType = databaseType;
}

QString AlbumSettings::getDatabaseConnectoptions() const
{
    return d->databaseParams.connectOptions;
}

QString AlbumSettings::getDatabaseName() const
{
    return d->databaseParams.databaseName;
}

QString AlbumSettings::getDatabaseNameThumbnails() const
{
    return d->databaseParams.databaseNameThumbnails;
}

QString AlbumSettings::getDatabaseHostName() const
{
    return d->databaseParams.hostName;
}

QString AlbumSettings::getDatabasePassword() const
{
    return d->databaseParams.password;
}

int AlbumSettings::getDatabasePort() const
{
    return d->databaseParams.port;
}

QString AlbumSettings::getDatabaseUserName() const
{
    return d->databaseParams.userName;
}

bool AlbumSettings::getInternalDatabaseServer() const
{
    return d->databaseParams.internalServer;
}

void AlbumSettings::setDatabaseConnectoptions(const QString& connectoptions)
{
    d->databaseParams.connectOptions = connectoptions;
}

void AlbumSettings::setDatabaseName(const QString& databaseName)
{
    d->databaseParams.databaseName = databaseName;
}

void AlbumSettings::setDatabaseNameThumbnails(const QString& databaseNameThumbnails)
{
    d->databaseParams.databaseNameThumbnails = databaseNameThumbnails;
}

void AlbumSettings::setDatabaseHostName(const QString& hostName)
{
    d->databaseParams.hostName = hostName;
}

void AlbumSettings::setDatabasePassword(const QString& password)
{
    d->databaseParams.password = password;
}

void AlbumSettings::setDatabasePort(int port)
{
    d->databaseParams.port = port;
}

void AlbumSettings::setDatabaseUserName(const QString& userName)
{
    d->databaseParams.userName = userName;
}

void AlbumSettings::setInternalDatabaseServer(const bool useInternalDBServer)
{
    d->databaseParams.internalServer = useInternalDBServer;
}

void AlbumSettings::setStringComparisonType(AlbumSettings::StringComparisonType val)
{
    d->stringComparisonType = val;
}

AlbumSettings::StringComparisonType AlbumSettings::getStringComparisonType() const
{
    return d->stringComparisonType;
}

void AlbumSettings::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    d->versionSettings = settings;
}

VersionManagerSettings AlbumSettings::getVersionManagerSettings() const
{
    return d->versionSettings;
}

double AlbumSettings::getFaceDetectionAccuracy() const
{
    return d->faceDetectionAccuracy;
}

void AlbumSettings::setFaceDetectionAccuracy(double value)
{
    d->faceDetectionAccuracy = value;
}

void AlbumSettings::applyNepomukSettings() const
{
#ifdef HAVE_NEPOMUK
    QDBusInterface interface("org.kde.nepomuk.services.digikamnepomukservice",
                             "/digikamnepomukservice", "org.kde.digikam.DigikamNepomukService");

    if (interface.isValid())
    {
        interface.call(QDBus::NoBlock, "enableSyncToDigikam", d->syncToDigikam);
        interface.call(QDBus::NoBlock, "enableSyncToNepomuk", d->syncToNepomuk);
    }

#endif // HAVE_NEPOMUK
}

void AlbumSettings::triggerResyncWithNepomuk() const
{
#ifdef HAVE_NEPOMUK
    QDBusInterface interface("org.kde.nepomuk.services.digikamnepomukservice",
                             "/digikamnepomukservice", "org.kde.digikam.DigikamNepomukService");

    if (interface.isValid())
    {
        interface.call(QDBus::NoBlock, "triggerResync");
    }

#endif // HAVE_NEPOMUK
}

void AlbumSettings::setApplicationStyle(const QString& style)
{
    if (d->applicationStyle != style)
    {
        d->applicationStyle = style;
        kapp->setStyle(d->applicationStyle);
    }
}

QString AlbumSettings::getApplicationStyle() const
{
    return d->applicationStyle;
}

}  // namespace Digikam
