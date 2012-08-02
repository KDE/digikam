/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-15
 * Description : Settings for the import tool
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importsettings.moc"

// KDE includes

#include <KGlobal>
#include <KGlobalSettings>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "camitemsortsettings.h"
#include "thumbnailsize.h"

namespace Digikam
{
class ImportSettings::ImportSettingsPrivate
{

public:
    ImportSettingsPrivate() :
        iconShowName(false),
        iconShowSize(false),
        iconShowDate(false),
        iconShowModDate(false),
        //iconShowResolution(false),
        //iconShowTags(false),
        //iconShowOverlays(false),
        //iconShowRating(false),
        iconShowImageFormat(false),
        thumbnailSize(0),
        imageSortOrder(0),
        imageSorting(0),
        imageGroupMode(0),
        showToolTips(false),
        tooltipShowFileName(false),
        tooltipShowFileDate(false),
        tooltipShowFileSize(false),
        tooltipShowImageType(false),
        tooltipShowPhotoMake(false),
        tooltipShowPhotoFocal(false),
        tooltipShowPhotoExpo(false),
        tooltipShowPhotoFlash(false),
        tooltipShowPhotoWb(false),
        tooltipShowFolderName(false),
        //tooltipShowTags(false),
        //tooltipShowLabelRating(false),
        previewLoadFullImageSize(false),
        previewShowIcons(true),
        //ratingFilterCond(0),
        showThumbbar(false)
    {
    }

    static const QString                configGroupDefault;
    static const QString                configImageSortOrderEntry;
    static const QString                configImageSortingEntry;
    static const QString                configImageGroupModeEntry;
    static const QString                configItemLeftClickActionEntry;
    static const QString                configDefaultIconSizeEntry;
    static const QString                configIconShowNameEntry;
    static const QString                configIconShowResolutionEntry;
    static const QString                configIconShowSizeEntry;
    static const QString                configIconShowDateEntry;
    static const QString                configIconShowModificationDateEntry;
    static const QString                configIconShowTitleEntry;
    //static const QString                configIconShowTagsEntry;
    //static const QString                configIconShowOverlaysEntry;
    //static const QString                configIconShowRatingEntry;
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
    static const QString                configToolTipsShowPhotoFlashEntry;
    static const QString                configToolTipsShowPhotoWBEntry;
    static const QString                configToolTipsShowFolderNameEntry;
    //static const QString                configToolTipsShowTagsEntry;
    //static const QString                configToolTipsShowLabelRatingEntry;
    static const QString                configPreviewLoadFullImageSizeEntry;
    static const QString                configPreviewShowIconsEntry;
    static const QString                configShowThumbbarEntry;

    // Import icon-view settings
    bool                                iconShowName;
    bool                                iconShowSize;
    bool                                iconShowDate;
    bool                                iconShowModDate;
    bool                                iconShowTitle;
    //bool                                iconShowResolution;
    //bool                                iconShowTags;
    //bool                                iconShowOverlays;
    //bool                                iconShowRating;
    bool                                iconShowImageFormat;

    QFont                               iconviewFont;

    int                                 thumbnailSize;
    int                                 imageSortOrder;
    int                                 imageSorting;
    int                                 imageGroupMode;
    ImportSettings::ItemLeftClickAction itemLeftClickAction;

    // Import icon-view tooltip settings
    bool                                showToolTips;
    bool                                tooltipShowFileName;
    bool                                tooltipShowFileDate;
    bool                                tooltipShowFileSize;
    bool                                tooltipShowImageType;
    bool                                tooltipShowImageDim;
    bool                                tooltipShowPhotoMake;
    bool                                tooltipShowPhotoFocal;
    bool                                tooltipShowPhotoExpo;
    bool                                tooltipShowPhotoFlash;
    bool                                tooltipShowPhotoWb;
    bool                                tooltipShowFolderName;
    //bool                                tooltipShowTags;
    //bool                                tooltipShowLabelRating;

    QFont                               toolTipsFont;

    // preview settings
    bool                                previewLoadFullImageSize;
    bool                                previewShowIcons;
    bool                                showThumbbar;

    KSharedConfigPtr                    config;
 };

    const QString ImportSettings::ImportSettingsPrivate::configGroupDefault("Import Settings");
    const QString ImportSettings::ImportSettingsPrivate::configImageSortOrderEntry("Image Sort Order");
    const QString ImportSettings::ImportSettingsPrivate::configImageSortingEntry("Image Sorting");
    const QString ImportSettings::ImportSettingsPrivate::configImageGroupModeEntry("Image Group Mode");
    const QString ImportSettings::ImportSettingsPrivate::configItemLeftClickActionEntry("Item Left Click Action");
    const QString ImportSettings::ImportSettingsPrivate::configDefaultIconSizeEntry("Default Icon Size");
    //const QString ImportSettings::ImportSettingsPrivate::configRatingFilterConditionEntry("Rating Filter Condition");
    const QString ImportSettings::ImportSettingsPrivate::configIconShowNameEntry("Icon Show Name");
    //const QString ImportSettings::ImportSettingsPrivate::configIconShowResolutionEntry("Icon Show Resolution");
    const QString ImportSettings::ImportSettingsPrivate::configIconShowSizeEntry("Icon Show Size");
    const QString ImportSettings::ImportSettingsPrivate::configIconShowDateEntry("Icon Show Date");
    const QString ImportSettings::ImportSettingsPrivate::configIconShowModificationDateEntry("Icon Show Modification Date");
    const QString ImportSettings::ImportSettingsPrivate::configIconShowTitleEntry("Icon Show Title");
    //const QString ImportSettings::ImportSettingsPrivate::configIconShowTagsEntry("Icon Show Tags");
    //const QString ImportSettings::ImportSettingsPrivate::configIconShowRatingEntry("Icon Show Rating");
    const QString ImportSettings::ImportSettingsPrivate::configIconShowImageFormatEntry("Icon Show Image Format");
    //const QString ImportSettings::ImportSettingsPrivate::configIconShowOverlaysEntry("Icon Show Overlays");
    const QString ImportSettings::ImportSettingsPrivate::configIconViewFontEntry("IconView Font");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsFontEntry("ToolTips Font");
    const QString ImportSettings::ImportSettingsPrivate::configShowToolTipsEntry("Show ToolTips");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowFileNameEntry("ToolTips Show File Name");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowFileDateEntry("ToolTips Show File Date");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowFileSizeEntry("ToolTips Show File Size");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowImageTypeEntry("ToolTips Show Image Type");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowImageDimEntry("ToolTips Show Image Dim");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowPhotoMakeEntry("ToolTips Show Photo Make");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowPhotoDateEntry("ToolTips Show Photo Date");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowPhotoFocalEntry("ToolTips Show Photo Focal");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowPhotoExpoEntry("ToolTips Show Photo Expo");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowPhotoFlashEntry("ToolTips Show Photo Flash");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowPhotoWBEntry("ToolTips Show Photo WB");
    const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowFolderNameEntry("ToolTips Show Folder Name");
    //const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowTagsEntry("ToolTips Show Tags");
    //const QString ImportSettings::ImportSettingsPrivate::configToolTipsShowLabelRatingEntry("ToolTips Show Label Rating");
    const QString ImportSettings::ImportSettingsPrivate::configPreviewLoadFullImageSizeEntry("Preview Load Full Image Size");
    const QString ImportSettings::ImportSettingsPrivate::configPreviewShowIconsEntry("Preview Show Icons");
    const QString ImportSettings::ImportSettingsPrivate::configShowThumbbarEntry("Show Thumbbar");

// -------------------------------------------------------------------------------------------------

class ImportSettingsCreator
{
public:

    ImportSettings object;
};

K_GLOBAL_STATIC(ImportSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ImportSettings* ImportSettings::instance()
{
    return &creator->object;
}

ImportSettings::ImportSettings()
    : QObject(), d(new ImportSettingsPrivate)
{
    d->config = KGlobal::config();
    init();
    readSettings();
}

ImportSettings::~ImportSettings()
{
    delete d;
}

void ImportSettings::init()
{
    d->imageSortOrder               = CamItemSortSettings::SortByFileName;
    d->imageSorting                 = CamItemSortSettings::AscendingOrder;
    d->imageGroupMode               = CamItemSortSettings::CategoryByFolder;
    d->itemLeftClickAction          = ImportSettings::ShowPreview;

    d->thumbnailSize                = ThumbnailSize::Medium;

    d->iconShowName                 = true;
    d->iconShowSize                 = false;
    d->iconShowDate                 = false;
    d->iconShowModDate              = true;
    d->iconShowTitle                = true;
    d->iconShowImageFormat          = false;
    //d->iconShowOverlays             = true;
    //d->iconShowRating               = true;
    //d->iconShowTags                 = true;
    d->iconviewFont                 = KGlobalSettings::generalFont();

    d->toolTipsFont                 = KGlobalSettings::generalFont();
    d->showToolTips                 = false;
    d->tooltipShowFileName          = true;
    d->tooltipShowFileDate          = false;
    d->tooltipShowFileSize          = false;
    d->tooltipShowImageType         = false;
    d->tooltipShowImageDim          = true;
    d->tooltipShowPhotoMake         = true;
    d->tooltipShowPhotoFocal        = true;
    d->tooltipShowPhotoExpo         = true;
    d->tooltipShowPhotoFlash        = false;
    d->tooltipShowPhotoWb           = false;
    //d->tooltipShowTags             = true;
    //d->tooltipShowLabelRating       = true;

    d->previewLoadFullImageSize     = false;
    d->previewShowIcons             = true;
    d->showThumbbar                 = true;
}

void ImportSettings::readSettings()
{
    KSharedConfigPtr config = d->config;

    KConfigGroup group  = config->group(d->configGroupDefault);

    d->imageSortOrder               = group.readEntry(d->configImageSortOrderEntry, (int)CamItemSortSettings::SortByFileName);
    d->imageSorting                 = group.readEntry(d->configImageSortingEntry,   (int)CamItemSortSettings::AscendingOrder);
    d->imageGroupMode               = group.readEntry(d->configImageGroupModeEntry, (int)CamItemSortSettings::CategoryByFolder);

    d->itemLeftClickAction          = ImportSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                                                         (int)ImportSettings::ShowPreview));

    d->thumbnailSize                = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);

    d->iconShowName                 = group.readEntry(d->configIconShowNameEntry,                 false);
    //d->iconShowResolution           = group.readEntry(d->configIconShowResolutionEntry,           false);
    d->iconShowSize                 = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                 = group.readEntry(d->configIconShowDateEntry,                 true);
    d->iconShowModDate              = group.readEntry(d->configIconShowModificationDateEntry,     true);
    d->iconShowTitle                = group.readEntry(d->configIconShowTitleEntry,                true);
    //d->iconShowTags                 = group.readEntry(d->configIconShowTagsEntry,                 true);
    //d->iconShowOverlays             = group.readEntry(d->configIconShowOverlaysEntry,             true);
    //d->iconShowRating               = group.readEntry(d->configIconShowRatingEntry,               true);
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
    d->tooltipShowPhotoFocal        = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo         = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
    //d->tooltipShowPhotoMode         = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
    d->tooltipShowPhotoFlash        = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb           = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowFolderName         = group.readEntry(d->configToolTipsShowFolderNameEntry,        false);
    //d->tooltipShowTags              = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    //d->tooltipShowLabelRating       = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->previewLoadFullImageSize     = group.readEntry(d->configPreviewLoadFullImageSizeEntry,     false);
    d->previewShowIcons             = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->showThumbbar                 = group.readEntry(d->configShowThumbbarEntry,                 true);

    // ---------------------------------------------------------------------

    emit setupChanged();
}

void ImportSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configImageSortOrderEntry,               (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortingEntry,                 (int)d->imageSorting);
    group.writeEntry(d->configImageGroupModeEntry,               (int)d->imageGroupMode);
    group.writeEntry(d->configItemLeftClickActionEntry,          (int)d->itemLeftClickAction);
    group.writeEntry(d->configDefaultIconSizeEntry,              QString::number(d->thumbnailSize));

    group.writeEntry(d->configIconShowNameEntry,                 d->iconShowName);
    group.writeEntry(d->configIconShowSizeEntry,                 d->iconShowSize);
    group.writeEntry(d->configIconShowDateEntry,                 d->iconShowDate);
    group.writeEntry(d->configIconShowModificationDateEntry,     d->iconShowModDate);
    group.writeEntry(d->configIconShowTitleEntry,                d->iconShowTitle);
    //group.writeEntry(d->configIconShowTagsEntry,                 d->iconShowTags);
    //group.writeEntry(d->configIconShowOverlaysEntry,             d->iconShowOverlays);
    //group.writeEntry(d->configIconShowRatingEntry,               d->iconShowRating);
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
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,       d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,        d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,       d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,          d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowFolderNameEntry,        d->tooltipShowFolderName);
    //group.writeEntry(d->configToolTipsShowTagsEntry,             d->tooltipShowTags);
    //group.writeEntry(d->configToolTipsShowLabelRatingEntry,      d->tooltipShowLabelRating);

    group.writeEntry(d->configPreviewLoadFullImageSizeEntry,     d->previewLoadFullImageSize);
    group.writeEntry(d->configPreviewShowIconsEntry,             d->previewShowIcons);
    group.writeEntry(d->configShowThumbbarEntry,                 d->showThumbbar);

    config->sync();
}

void ImportSettings::emitSetupChanged()
{
    emit setupChanged();
}

void ImportSettings::setImageSortOrder(int order)
{
    d->imageSortOrder = order;
}

int ImportSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
}

void ImportSettings::setImageSorting(int sorting)
{
    d->imageSorting = sorting;
}

int ImportSettings::getImageSorting() const
{
    return d->imageSorting;
}

void ImportSettings::setImageGroupMode(int mode)
{
    d->imageGroupMode = mode;
}

int ImportSettings::getImageGroupMode() const
{
    return d->imageGroupMode;
}

void ImportSettings::setItemLeftClickAction(const ItemLeftClickAction action)
{
    d->itemLeftClickAction = action;
}

ImportSettings::ItemLeftClickAction ImportSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

void ImportSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int ImportSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void ImportSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont ImportSettings::getIconViewFont() const
{
    return d->iconviewFont;
}

void ImportSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool ImportSettings::getIconShowName() const
{
    return d->iconShowName;
}

void ImportSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool ImportSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void ImportSettings::setIconShowTitle(bool val)
{
    d->iconShowTitle = val;
}

bool ImportSettings::getIconShowTitle() const
{
    return d->iconShowTitle;
}

//void ImportSettings::setIconShowTags(bool val)
//{
//    d->iconShowTags = val;
//}

//bool ImportSettings::getIconShowTags() const
//{
//    return d->iconShowTags;
//}

void ImportSettings::setIconShowModDate(bool val)
{
    d->iconShowModDate = val;
}

bool ImportSettings::getIconShowModDate() const
{
    return d->iconShowModDate;
}

//void ImportSettings::setIconShowRating(bool val)
//{
//    d->iconShowRating = val;
//}

//bool ImportSettings::getIconShowRating() const
//{
//    return d->iconShowRating;
//}

void ImportSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool ImportSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

//void ImportSettings::setIconShowOverlays(bool val)
//{
//    d->iconShowOverlays = val;
//}

//bool ImportSettings::getIconShowOverlays() const
//{
//    return d->iconShowOverlays;
//}

void ImportSettings::setToolTipsFont(const QFont& font)
{
    d->toolTipsFont = font;
}

QFont ImportSettings::getToolTipsFont() const
{
    return d->toolTipsFont;
}

void ImportSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool ImportSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void ImportSettings::setToolTipsShowFileName(bool val)
{
    d->tooltipShowFileName = val;
}

bool ImportSettings::getToolTipsShowFileName() const
{
    return d->tooltipShowFileName;
}

void ImportSettings::setToolTipsShowFileDate(bool val)
{
    d->tooltipShowFileDate = val;
}

bool ImportSettings::getToolTipsShowFileDate() const
{
    return d->tooltipShowFileDate;
}

void ImportSettings::setToolTipsShowFileSize(bool val)
{
    d->tooltipShowFileSize = val;
}

bool ImportSettings::getToolTipsShowFileSize() const
{
    return d->tooltipShowFileSize;
}

void ImportSettings::setToolTipsShowImageType(bool val)
{
    d->tooltipShowImageType = val;
}

bool ImportSettings::getToolTipsShowImageType() const
{
    return d->tooltipShowImageType;
}

void ImportSettings::setToolTipsShowImageDim(bool val)
{
    d->tooltipShowImageDim = val;
}

bool ImportSettings::getToolTipsShowImageDim() const
{
    return d->tooltipShowImageDim;
}

void ImportSettings::setToolTipsShowPhotoMake(bool val)
{
    d->tooltipShowPhotoMake = val;
}

bool ImportSettings::getToolTipsShowPhotoMake() const
{
    return d->tooltipShowPhotoMake;
}

void ImportSettings::setToolTipsShowPhotoFocal(bool val)
{
    d->tooltipShowPhotoFocal = val;
}

bool ImportSettings::getToolTipsShowPhotoFocal() const
{
    return d->tooltipShowPhotoFocal;
}

void ImportSettings::setToolTipsShowPhotoExpo(bool val)
{
    d->tooltipShowPhotoExpo = val;
}

bool ImportSettings::getToolTipsShowPhotoExpo() const
{
    return d->tooltipShowPhotoExpo;
}

void ImportSettings::setToolTipsShowPhotoFlash(bool val)
{
    d->tooltipShowPhotoFlash = val;
}

bool ImportSettings::getToolTipsShowPhotoFlash() const
{
    return d->tooltipShowPhotoFlash;
}

void ImportSettings::setToolTipsShowPhotoWB(bool val)
{
    d->tooltipShowPhotoWb = val;
}

bool ImportSettings::getToolTipsShowPhotoWB() const
{
    return d->tooltipShowPhotoWb;
}

//void ImportSettings::setToolTipsShowTags(bool val)
//{
//    d->tooltipShowTags = val;
//}

//bool ImportSettings::getToolTipsShowTags() const
//{
//    return d->tooltipShowTags;
//}

//void ImportSettings::setToolTipsShowLabelRating(bool val)
//{
//    d->tooltipShowLabelRating = val;
//}

//bool ImportSettings::getToolTipsShowLabelRating() const
//{
//    return d->tooltipShowLabelRating;
//}

bool ImportSettings::showToolTipsIsValid() const
{
    if (d->showToolTips)
    {
        if (d->tooltipShowFileName   ||
            d->tooltipShowFileDate   ||
            d->tooltipShowFileSize   ||
            d->tooltipShowImageType  ||
            d->tooltipShowImageDim   ||
            d->tooltipShowPhotoMake  ||
            d->tooltipShowPhotoFocal ||
            d->tooltipShowPhotoExpo  ||
            d->tooltipShowPhotoFlash ||
            d->tooltipShowPhotoWb    ||
            d->tooltipShowFolderName)
            //d->tooltipShowTags       ||
            //d->tooltipShowLabelRating)
        {
            return true;
        }
    }

    return false;
}

void ImportSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool ImportSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void ImportSettings::setPreviewLoadFullImageSize(bool val)
{
    d->previewLoadFullImageSize = val;
}

bool ImportSettings::getPreviewLoadFullImageSize() const
{
    return d->previewLoadFullImageSize;
}

void ImportSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ImportSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

} // namespace Digikam
