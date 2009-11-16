/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : albums settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Arnd Baecker <arnd dot baecker at web dot de>
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

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes

#include "config-digikam.h"
#include "thumbnailsize.h"
#include "mimefilter.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "imagefiltersettings.h"
#include "imagesortsettings.h"

namespace Digikam
{

class AlbumSettingsPrivate
{

public:

    AlbumSettingsPrivate() :
        configGroupDefault("Album Settings"),
        configGroupExif("EXIF Settings"),
        configGroupMetadata("Metadata Settings"),
        configGroupNepomuk("Nepomuk Settings"),
        configGroupGeneral("General Settings"),

        configDatabaseFilePathEntry("Database File Path"),
        configAlbumCollectionsEntry("Album Collections"),
        configAlbumSortOrderEntry("Album Sort Order"),
        configImageSortOrderEntry("Image Sort Order"),
        configImageSortingEntry("Image Sorting"),
        configImageGroupModeEntry("Image Group Mode"),
        configItemLeftClickActionEntry("Item Left Click Action"),
        configDefaultIconSizeEntry("Default Icon Size"),
        configDefaultTreeIconSizeEntry("Default Tree Icon Size"),
        configTreeViewFontEntry("TreeView Font"),
        configThemeEntry("Theme"),
        configSidebarTitleStyleEntry("Sidebar Title Style"),
        configRatingFilterConditionEntry("Rating Filter Condition"),
        configRecursiveAlbumsEntry("Recursive Albums"),
        configRecursiveTagsEntry("Recursive Tags"),
        configIconShowNameEntry("Icon Show Name"),
        configIconShowResolutionEntry("Icon Show Resolution"),
        configIconShowSizeEntry("Icon Show Size"),
        configIconShowDateEntry("Icon Show Date"),
        configIconShowModificationDateEntry("Icon Show Modification Date"),
        configIconShowCommentsEntry("Icon Show Comments"),
        configIconShowTagsEntry("Icon Show Tags"),
        configIconShowRatingEntry("Icon Show Rating"),
        configIconShowOverlaysEntry("Icon Show Overlays"),
        configIconViewFontEntry("IconView Font"),
        configToolTipsFontEntry("ToolTips Font"),
        configShowToolTipsEntry("Show ToolTips"),
        configToolTipsShowFileNameEntry("ToolTips Show File Name"),
        configToolTipsShowFileDateEntry("ToolTips Show File Date"),
        configToolTipsShowFileSizeEntry("ToolTips Show File Size"),
        configToolTipsShowImageTypeEntry("ToolTips Show Image Type"),
        configToolTipsShowImageDimEntry("ToolTips Show Image Dim"),
        configToolTipsShowPhotoMakeEntry("ToolTips Show Photo Make"),
        configToolTipsShowPhotoDateEntry("ToolTips Show Photo Date"),
        configToolTipsShowPhotoFocalEntry("ToolTips Show Photo Focal"),
        configToolTipsShowPhotoExpoEntry("ToolTips Show Photo Expo"),
        configToolTipsShowPhotoModeEntry("ToolTips Show Photo Mode"),
        configToolTipsShowPhotoFlashEntry("ToolTips Show Photo Flash"),
        configToolTipsShowPhotoWBEntry("ToolTips Show Photo WB"),
        configToolTipsShowAlbumNameEntry("ToolTips Show Album Name"),
        configToolTipsShowCommentsEntry("ToolTips Show Comments"),
        configToolTipsShowTagsEntry("ToolTips Show Tags"),
        configToolTipsShowRatingEntry("ToolTips Show Rating"),
        configShowAlbumToolTipsEntry("Show Album ToolTips"),
        configToolTipsShowAlbumTitleEntry("ToolTips Show Album Title"),
        configToolTipsShowAlbumDateEntry("ToolTips Show Album Date"),
        configToolTipsShowAlbumCollectionEntry("ToolTips Show Album Collection"),
        configToolTipsShowAlbumCategoryEntry("ToolTips Show Album Category"),
        configToolTipsShowAlbumCaptionEntry("ToolTips Show Album Caption"),
        configPreviewLoadFullImageSizeEntry("Preview Load Full Image Size"),
        configShowThumbbarEntry("Show Thumbbar"),
        configShowFolderTreeViewItemsCountEntry("Show Folder Tree View Items Count"),
        configEXIFRotateEntry("EXIF Rotate"),
        configEXIFSetOrientationEntry("EXIF Set Orientation"),
        configSaveTagsEntry("Save Tags"),
        configSaveTemplateEntry("Save Template"),
        configSaveEXIFCommentsEntry("Save EXIF Comments"),
        configSaveDateTimeEntry("Save Date Time"),
        configSaveRatingEntry("Save Rating"),
        configWriteRAWFilesEntry("Write RAW Files"),
        configUpdateFileTimestampEntry("Update File Timestamp"),
        configShowSplashEntry("Show Splash"),
        configUseTrashEntry("Use Trash"),
        configShowTrashDeleteDialogEntry("Show Trash Delete Dialog"),
        configShowPermanentDeleteDialogEntry("Show Permanent Delete Dialog"),
        configApplySidebarChangesDirectlyEntry("Apply Sidebar Changes Directly"),
        configScanAtStartEntry("Scan At Start"),
        configSyncNepomuktoDigikamEntry("Sync Nepomuk to Digikam"),
        configSyncDigikamtoNepomukEntry("Sync Digikam to Nepomuk")
        {}

    const QString                       configGroupDefault;
    const QString                       configGroupExif;
    const QString                       configGroupMetadata;
    const QString                       configGroupNepomuk;
    const QString                       configGroupGeneral;

    const QString                       configDatabaseFilePathEntry;
    const QString                       configAlbumCollectionsEntry;
    const QString                       configAlbumSortOrderEntry;
    const QString                       configImageSortOrderEntry;
    const QString                       configImageSortingEntry;
    const QString                       configImageGroupModeEntry;
    const QString                       configItemLeftClickActionEntry;
    const QString                       configDefaultIconSizeEntry;
    const QString                       configDefaultTreeIconSizeEntry;
    const QString                       configTreeViewFontEntry;
    const QString                       configThemeEntry;
    const QString                       configSidebarTitleStyleEntry;
    const QString                       configRatingFilterConditionEntry;
    const QString                       configRecursiveAlbumsEntry;
    const QString                       configRecursiveTagsEntry;
    const QString                       configIconShowNameEntry;
    const QString                       configIconShowResolutionEntry;
    const QString                       configIconShowSizeEntry;
    const QString                       configIconShowDateEntry;
    const QString                       configIconShowModificationDateEntry;
    const QString                       configIconShowCommentsEntry;
    const QString                       configIconShowTagsEntry;
    const QString                       configIconShowRatingEntry;
    const QString                       configIconShowOverlaysEntry;
    const QString                       configIconViewFontEntry;
    const QString                       configToolTipsFontEntry;
    const QString                       configShowToolTipsEntry;
    const QString                       configToolTipsShowFileNameEntry;
    const QString                       configToolTipsShowFileDateEntry;
    const QString                       configToolTipsShowFileSizeEntry;
    const QString                       configToolTipsShowImageTypeEntry;
    const QString                       configToolTipsShowImageDimEntry;
    const QString                       configToolTipsShowPhotoMakeEntry;
    const QString                       configToolTipsShowPhotoDateEntry;
    const QString                       configToolTipsShowPhotoFocalEntry;
    const QString                       configToolTipsShowPhotoExpoEntry;
    const QString                       configToolTipsShowPhotoModeEntry;
    const QString                       configToolTipsShowPhotoFlashEntry;
    const QString                       configToolTipsShowPhotoWBEntry;
    const QString                       configToolTipsShowAlbumNameEntry;
    const QString                       configToolTipsShowCommentsEntry;
    const QString                       configToolTipsShowTagsEntry;
    const QString                       configToolTipsShowRatingEntry;
    const QString                       configShowAlbumToolTipsEntry;
    const QString                       configToolTipsShowAlbumTitleEntry;
    const QString                       configToolTipsShowAlbumDateEntry;
    const QString                       configToolTipsShowAlbumCollectionEntry;
    const QString                       configToolTipsShowAlbumCategoryEntry;
    const QString                       configToolTipsShowAlbumCaptionEntry;
    const QString                       configPreviewLoadFullImageSizeEntry;
    const QString                       configShowThumbbarEntry;
    const QString                       configShowFolderTreeViewItemsCountEntry;
    const QString                       configEXIFRotateEntry;
    const QString                       configEXIFSetOrientationEntry;
    const QString                       configSaveTagsEntry;
    const QString                       configSaveTemplateEntry;
    const QString                       configSaveEXIFCommentsEntry;
    const QString                       configSaveDateTimeEntry;
    const QString                       configSaveRatingEntry;
    const QString                       configWriteRAWFilesEntry;
    const QString                       configUpdateFileTimestampEntry;
    const QString                       configShowSplashEntry;
    const QString                       configUseTrashEntry;
    const QString                       configShowTrashDeleteDialogEntry;
    const QString                       configShowPermanentDeleteDialogEntry;
    const QString                       configApplySidebarChangesDirectlyEntry;
    const QString                       configScanAtStartEntry;
    const QString                       configSyncNepomuktoDigikamEntry;
    const QString                       configSyncDigikamtoNepomukEntry;

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
    bool                                iconShowComments;
    bool                                iconShowResolution;
    bool                                iconShowTags;
    bool                                iconShowRating;
    bool                                iconShowOverlays;

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
    bool                                tooltipShowRating;

    QFont                               toolTipsFont;

    // Folder-view tooltip settings
    bool                                showAlbumToolTips;
    bool                                tooltipShowAlbumTitle;
    bool                                tooltipShowAlbumDate;
    bool                                tooltipShowAlbumCollection;
    bool                                tooltipShowAlbumCategory;
    bool                                tooltipShowAlbumCaption;

    // metadata settings
    bool                                exifRotate;
    bool                                exifSetOrientation;

    bool                                saveTags;
    bool                                saveTemplate;

    bool                                saveComments;
    bool                                saveDateTime;
    bool                                saveRating;

    bool                                writeRawFiles;
    bool                                updateFileTimeStamp;

    // preview settings
    bool                                previewLoadFullImageSize;
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
    QString                             databaseFilePath;

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
};

class AlbumSettingsCreator { public: AlbumSettings object; };
K_GLOBAL_STATIC(AlbumSettingsCreator, creator)

AlbumSettings* AlbumSettings::instance()
{
    return &creator->object;
}

AlbumSettings::AlbumSettings()
             : QObject(), d(new AlbumSettingsPrivate)
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
    d->iconShowComments             = true;
    d->iconShowResolution           = false;
    d->iconShowTags                 = true;
    d->iconShowRating               = true;
    d->iconShowOverlays             = true;
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
    d->tooltipShowRating            = true;

    d->showAlbumToolTips            = false;
    d->tooltipShowAlbumTitle        = true;
    d->tooltipShowAlbumDate         = true;
    d->tooltipShowAlbumCollection   = true;
    d->tooltipShowAlbumCategory     = true;
    d->tooltipShowAlbumCaption      = true;

    d->exifRotate                   = true;
    d->exifSetOrientation           = true;

    d->saveTags                     = false;
    d->saveTemplate                 = false;

    d->saveComments                 = false;
    d->saveDateTime                 = false;
    d->saveRating                   = false;

    d->writeRawFiles                = false;
    d->updateFileTimeStamp          = false;

    d->previewLoadFullImageSize     = false;
    d->showThumbbar                 = true;

    d->recursiveAlbums              = false;
    d->recursiveTags                = true;

    d->showFolderTreeViewItemsCount = false;

    d->syncToDigikam                = false;
    d->syncToNepomuk                = false;
}

void AlbumSettings::readSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group  = config->group(d->configGroupDefault);

    d->databaseFilePath = group.readEntry(d->configDatabaseFilePathEntry, QString());

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

    d->thumbnailSize                = group.readEntry(d->configDefaultIconSizeEntry,        (int)ThumbnailSize::Medium);
    d->treeThumbnailSize            = group.readEntry(d->configDefaultTreeIconSizeEntry,    22);
    d->treeviewFont                 = group.readEntry(d->configTreeViewFontEntry,           KGlobalSettings::generalFont());
    d->currentTheme                 = group.readEntry(d->configThemeEntry,                  i18nc("default theme name", "Default"));

    d->sidebarTitleStyle            = (KMultiTabBar::KMultiTabBarStyle)group.readEntry(d->configSidebarTitleStyleEntry,
                                                                                       (int)KMultiTabBar::VSNET);


    d->ratingFilterCond             = group.readEntry(d->configRatingFilterConditionEntry, (int)ImageFilterSettings::GreaterEqualCondition);
    d->recursiveAlbums              = group.readEntry(d->configRecursiveAlbumsEntry,       false);
    d->recursiveTags                = group.readEntry(d->configRecursiveTagsEntry,         true);


    d->iconShowName                 = group.readEntry(d->configIconShowNameEntry,             false);
    d->iconShowResolution           = group.readEntry(d->configIconShowResolutionEntry,       false);
    d->iconShowSize                 = group.readEntry(d->configIconShowSizeEntry,             false);
    d->iconShowDate                 = group.readEntry(d->configIconShowDateEntry,             true);
    d->iconShowModDate              = group.readEntry(d->configIconShowModificationDateEntry, true);
    d->iconShowComments             = group.readEntry(d->configIconShowCommentsEntry,         true);
    d->iconShowTags                 = group.readEntry(d->configIconShowTagsEntry,             true);
    d->iconShowRating               = group.readEntry(d->configIconShowRatingEntry,           true);
    d->iconShowOverlays             = group.readEntry(d->configIconShowOverlaysEntry,         true);
    d->iconviewFont                 = group.readEntry(d->configIconViewFontEntry,             KGlobalSettings::generalFont());

    d->toolTipsFont                 = group.readEntry(d->configToolTipsFontEntry,           KGlobalSettings::generalFont());
    d->showToolTips                 = group.readEntry(d->configShowToolTipsEntry,           false);
    d->tooltipShowFileName          = group.readEntry(d->configToolTipsShowFileNameEntry,   true);
    d->tooltipShowFileDate          = group.readEntry(d->configToolTipsShowFileDateEntry,   false);
    d->tooltipShowFileSize          = group.readEntry(d->configToolTipsShowFileSizeEntry,   false);
    d->tooltipShowImageType         = group.readEntry(d->configToolTipsShowImageTypeEntry,  false);
    d->tooltipShowImageDim          = group.readEntry(d->configToolTipsShowImageDimEntry,   true);
    d->tooltipShowPhotoMake         = group.readEntry(d->configToolTipsShowPhotoMakeEntry,  true);
    d->tooltipShowPhotoDate         = group.readEntry(d->configToolTipsShowPhotoDateEntry,  true);
    d->tooltipShowPhotoFocal        = group.readEntry(d->configToolTipsShowPhotoFocalEntry, true);
    d->tooltipShowPhotoExpo         = group.readEntry(d->configToolTipsShowPhotoExpoEntry,  true);
    d->tooltipShowPhotoMode         = group.readEntry(d->configToolTipsShowPhotoModeEntry,  true);
    d->tooltipShowPhotoFlash        = group.readEntry(d->configToolTipsShowPhotoFlashEntry, false);
    d->tooltipShowPhotoWb           = group.readEntry(d->configToolTipsShowPhotoWBEntry,    false);
    d->tooltipShowAlbumName         = group.readEntry(d->configToolTipsShowAlbumNameEntry,  false);
    d->tooltipShowComments          = group.readEntry(d->configToolTipsShowCommentsEntry,   true);
    d->tooltipShowTags              = group.readEntry(d->configToolTipsShowTagsEntry,       true);
    d->tooltipShowRating            = group.readEntry(d->configToolTipsShowRatingEntry,     true);

    d->showAlbumToolTips            = group.readEntry(d->configShowAlbumToolTipsEntry,            false);
    d->tooltipShowAlbumTitle        = group.readEntry(d->configToolTipsShowAlbumTitleEntry,       true);
    d->tooltipShowAlbumDate         = group.readEntry(d->configToolTipsShowAlbumDateEntry,        true);
    d->tooltipShowAlbumCollection   = group.readEntry(d->configToolTipsShowAlbumCollectionEntry,  true);
    d->tooltipShowAlbumCategory     = group.readEntry(d->configToolTipsShowAlbumCategoryEntry,    true);
    d->tooltipShowAlbumCaption      = group.readEntry(d->configToolTipsShowAlbumCaptionEntry,     true);

    d->previewLoadFullImageSize     = group.readEntry(d->configPreviewLoadFullImageSizeEntry,     false);
    d->showThumbbar                 = group.readEntry(d->configShowThumbbarEntry,                 true);

    d->showFolderTreeViewItemsCount = group.readEntry(d->configShowFolderTreeViewItemsCountEntry, false);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupExif);

    d->exifRotate         = group.readEntry(d->configEXIFRotateEntry,         true);
    d->exifSetOrientation = group.readEntry(d->configEXIFSetOrientationEntry, true);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupMetadata);

    d->saveTags               = group.readEntry(d->configSaveTagsEntry,     false);
    d->saveTemplate           = group.readEntry(d->configSaveTemplateEntry, false);

    d->saveComments           = group.readEntry(d->configSaveEXIFCommentsEntry, false);
    d->saveDateTime           = group.readEntry(d->configSaveDateTimeEntry,     false);
    d->saveRating             = group.readEntry(d->configSaveRatingEntry,       false);

    d->writeRawFiles          = group.readEntry(d->configWriteRAWFilesEntry,       false);
    d->updateFileTimeStamp    = group.readEntry(d->configUpdateFileTimestampEntry, false);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupGeneral);

    d->showSplash                = group.readEntry(d->configShowSplashEntry,                  true);
    d->useTrash                  = group.readEntry(d->configUseTrashEntry,                    true);
    d->showTrashDeleteDialog     = group.readEntry(d->configShowTrashDeleteDialogEntry,       true);
    d->showPermanentDeleteDialog = group.readEntry(d->configShowPermanentDeleteDialogEntry,   true);
    d->sidebarApplyDirectly      = group.readEntry(d->configApplySidebarChangesDirectlyEntry, false);
    d->scanAtStart               = group.readEntry(d->configScanAtStartEntry,                 true);

    // ---------------------------------------------------------------------

#ifdef HAVE_NEPOMUK

    group = config->group(d->configGroupNepomuk);

    d->syncToDigikam         = group.readEntry(d->configSyncNepomuktoDigikamEntry, false);
    d->syncToNepomuk         = group.readEntry(d->configSyncDigikamtoNepomukEntry, false);

#endif // HAVE_NEPOMUK

    emit setupChanged();
    emit recurseSettingsChanged();
    emit nepomukSettingsChanged();
}

void AlbumSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configDatabaseFilePathEntry,             d->databaseFilePath);
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
    group.writeEntry(d->configIconShowCommentsEntry,             d->iconShowComments);
    group.writeEntry(d->configIconShowTagsEntry,                 d->iconShowTags);
    group.writeEntry(d->configIconShowRatingEntry,               d->iconShowRating);
    group.writeEntry(d->configIconShowOverlaysEntry,             d->iconShowOverlays);
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
    group.writeEntry(d->configToolTipsShowRatingEntry,           d->tooltipShowRating);

    group.writeEntry(d->configShowAlbumToolTipsEntry,            d->showAlbumToolTips);
    group.writeEntry(d->configToolTipsShowAlbumTitleEntry,       d->tooltipShowAlbumTitle);
    group.writeEntry(d->configToolTipsShowAlbumDateEntry,        d->tooltipShowAlbumDate);
    group.writeEntry(d->configToolTipsShowAlbumCollectionEntry,  d->tooltipShowAlbumCollection);
    group.writeEntry(d->configToolTipsShowAlbumCategoryEntry,    d->tooltipShowAlbumCategory);
    group.writeEntry(d->configToolTipsShowAlbumCaptionEntry,     d->tooltipShowAlbumCaption);

    group.writeEntry(d->configPreviewLoadFullImageSizeEntry,     d->previewLoadFullImageSize);
    group.writeEntry(d->configShowThumbbarEntry,                 d->showThumbbar);

    group.writeEntry(d->configShowFolderTreeViewItemsCountEntry, d->showFolderTreeViewItemsCount);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupExif);

    group.writeEntry(d->configEXIFRotateEntry, d->exifRotate);
    group.writeEntry(d->configEXIFSetOrientationEntry, d->exifSetOrientation);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupMetadata);

    group.writeEntry(d->configSaveTagsEntry,            d->saveTags);
    group.writeEntry(d->configSaveTemplateEntry,        d->saveTemplate);

    group.writeEntry(d->configSaveEXIFCommentsEntry,    d->saveComments);
    group.writeEntry(d->configSaveDateTimeEntry,        d->saveDateTime);
    group.writeEntry(d->configSaveRatingEntry,          d->saveRating);

    group.writeEntry(d->configWriteRAWFilesEntry,       d->writeRawFiles);
    group.writeEntry(d->configUpdateFileTimestampEntry, d->updateFileTimeStamp);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupGeneral);

    group.writeEntry(d->configShowSplashEntry,                  d->showSplash);
    group.writeEntry(d->configUseTrashEntry,                    d->useTrash);
    group.writeEntry(d->configShowTrashDeleteDialogEntry,       d->showTrashDeleteDialog);
    group.writeEntry(d->configShowPermanentDeleteDialogEntry,   d->showPermanentDeleteDialog);
    group.writeEntry(d->configApplySidebarChangesDirectlyEntry, d->sidebarApplyDirectly);
    group.writeEntry(d->configScanAtStartEntry,                 d->scanAtStart);

    // ---------------------------------------------------------------------

#ifdef HAVE_NEPOMUK

    group = config->group(d->configGroupNepomuk);

    group.writeEntry(d->configSyncNepomuktoDigikamEntry, d->syncToDigikam);
    group.writeEntry(d->configSyncDigikamtoNepomukEntry, d->syncToNepomuk);

#endif // HAVE_NEPOMUK

    config->sync();
}

void AlbumSettings::emitSetupChanged()
{
    emit setupChanged();
}

QString AlbumSettings::getDatabaseFilePath() const
{
    return d->databaseFilePath;
}

void AlbumSettings::setDatabaseFilePath(const QString& path)
{
    d->databaseFilePath = path;
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

QStringList AlbumSettings::getAlbumCategoryNames()
{
    return d->albumCategoryNames;
}

bool AlbumSettings::addAlbumCategoryName(const QString& name)
{
    if (d->albumCategoryNames.contains(name))
        return false;

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
    foreach (const QString& suffix, imageSettings)
        wildcards << "*." + suffix;
    return wildcards.join(" ");
}

QString AlbumSettings::getMovieFileFilter() const
{
    QStringList movieSettings;
    DatabaseAccess().db()->getFilterSettings(0, &movieSettings, 0);
    QStringList wildcards;
    foreach (const QString& suffix, movieSettings)
        wildcards << "*." + suffix;
    return wildcards.join(" ");
}

QString AlbumSettings::getAudioFileFilter() const
{
    QStringList audioSettings;
    DatabaseAccess().db()->getFilterSettings(0, 0, &audioSettings);
    QStringList wildcards;
    foreach (const QString& suffix, audioSettings)
        wildcards << "*." + suffix;
    return wildcards.join(" ");
}

QString AlbumSettings::getRawFileFilter() const
{
    QStringList supportedRaws;
#if KDCRAW_VERSION < 0x000400
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
        supportedRaws = KDcrawIface::DcrawBinary::rawFilesList();
#else
    supportedRaws = KDcrawIface::KDcraw::rawFilesList();
#endif

    QStringList imageSettings;
    DatabaseAccess().db()->getFilterSettings(&imageSettings, 0, 0);

    // form intersection: those extensions that are supported as RAW as well in the list of allowed extensions
    for (QStringList::iterator it = supportedRaws.begin(); it != supportedRaws.end(); )
    {
        if (imageSettings.contains(*it))
            ++it;
        else
            it = supportedRaws.erase(it);
    }

    QStringList wildcards;
    foreach (const QString& suffix, supportedRaws)
        wildcards << "*." + suffix;
    return wildcards.join(" ");
}

QString AlbumSettings::getAllFileFilter() const
{
    QStringList imageFilter, audioFilter, videoFilter;
    DatabaseAccess().db()->getFilterSettings(&imageFilter, &audioFilter, &videoFilter);
    QStringList wildcards;
    foreach (const QString& suffix, imageFilter)
        wildcards << "*." + suffix;
    foreach (const QString& suffix, audioFilter)
        wildcards << "*." + suffix;
    foreach (const QString& suffix, videoFilter)
        wildcards << "*." + suffix;
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

void AlbumSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool AlbumSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

void AlbumSettings::setExifRotate(bool val)
{
    d->exifRotate = val;
}

bool AlbumSettings::getExifRotate() const
{
    return d->exifRotate;
}

void AlbumSettings::setExifSetOrientation(bool val)
{
    d->exifSetOrientation = val;
}

bool AlbumSettings::getExifSetOrientation() const
{
    return d->exifSetOrientation;
}

void AlbumSettings::setSaveTags(bool val)
{
    d->saveTags = val;
}

bool AlbumSettings::getSaveTags() const
{
    return d->saveTags;
}

void AlbumSettings::setSaveTemplate(bool val)
{
    d->saveTemplate = val;
}

bool AlbumSettings::getSaveTemplate() const
{
    return d->saveTemplate;
}

void AlbumSettings::setWriteRawFiles(bool val)
{
    d->writeRawFiles = val;
}

bool AlbumSettings::getWriteRawFiles() const
{
    return d->writeRawFiles;
}

void AlbumSettings::setUpdateFileTimeStamp(bool val)
{
    d->updateFileTimeStamp = val;
}

bool AlbumSettings::getUpdateFileTimeStamp() const
{
    return d->updateFileTimeStamp;
}

void AlbumSettings::setSaveComments(bool val)
{
    d->saveComments = val;
}

bool AlbumSettings::getSaveComments() const
{
    return d->saveComments;
}

void AlbumSettings::setSaveDateTime(bool val)
{
    d->saveDateTime = val;
}

bool AlbumSettings::getSaveDateTime() const
{
    return d->saveDateTime;
}

bool AlbumSettings::getSaveRating() const
{
    return d->saveRating;
}

void AlbumSettings::setSaveRating(bool val)
{
    d->saveRating = val;
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

void AlbumSettings::setToolTipsShowRating(bool val)
{
    d->tooltipShowRating = val;
}

bool AlbumSettings::getToolTipsShowRating() const
{
    return d->tooltipShowRating;
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
            d->tooltipShowRating)
           return true;
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
           return true;
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


}  // namespace Digikam
