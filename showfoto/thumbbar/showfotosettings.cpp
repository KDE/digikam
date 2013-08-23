/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-29
 * Description : Settings for the Showfoto
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotosettings.h"

// KDE includes

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "showfotoitemsortsettings.h"
#include "thumbnailsize.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoSettings::Private
{

public:
    Private() :
        iconShowName(true),
        iconShowSize(false),
        iconShowDate(true),
        iconShowTitle(false),
        iconShowResolution(false),
        iconShowTags(false),
        iconShowOverlays(false),
        iconShowRating(false),
        iconShowImageFormat(false),
        thumbnailSize(0),
        imageSortOrder(0),
        imageSorting(0),
        imageGroupMode(0),
        itemLeftClickAction(ShowfotoSettings::ShowPreview),
        showToolTips(false),
        tooltipShowFileName(false),
        tooltipShowFileDate(false),
        tooltipShowFileSize(false),
        tooltipShowImageType(false),
        tooltipShowImageDim(true),
        tooltipShowPhotoMake(false),
        tooltipShowPhotoFocal(false),
        tooltipShowPhotoExpo(false),
        tooltipShowPhotoFlash(false),
        tooltipShowPhotoWb(false),
        tooltipShowFolderName(false),
        tooltipShowTags(false),
        tooltipShowLabelRating(false),
        previewLoadFullImageSize(false),
        previewItemsWhileDownload(false),
        previewShowIcons(true),
//      ratingFilterCond(0),
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
    static const QString                configIconShowTitleEntry;
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
    static const QString                configToolTipsShowPhotoFlashEntry;
    static const QString                configToolTipsShowPhotoWBEntry;
    static const QString                configToolTipsShowFolderNameEntry;
    static const QString                configToolTipsShowTagsEntry;
    static const QString                configToolTipsShowLabelRatingEntry;
    static const QString                configPreviewLoadFullImageSizeEntry;
    static const QString                configPreviewItemsWhileDownloadEntry;
    static const QString                configPreviewShowIconsEntry;
    static const QString                configShowThumbbarEntry;

    // Showfoto icon-view settings
    bool                                iconShowName;
    bool                                iconShowSize;
    bool                                iconShowDate;
    bool                                iconShowTitle;
    bool                                iconShowResolution;
    bool                                iconShowTags;
    bool                                iconShowOverlays;
    bool                                iconShowRating;
    bool                                iconShowImageFormat;

    QFont                               iconviewFont;

    int                                 thumbnailSize;
    int                                 imageSortOrder;
    int                                 imageSorting;
    int                                 imageGroupMode;
    ShowfotoSettings::ItemLeftClickAction itemLeftClickAction;

    // Showfoto icon-view tooltip settings
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
    bool                                tooltipShowTags;
    bool                                tooltipShowLabelRating;

    QFont                               toolTipsFont;

    // preview settings
    bool                                previewLoadFullImageSize;
    bool                                previewItemsWhileDownload;
    bool                                previewShowIcons;
    bool                                showThumbbar;

    KSharedConfigPtr                    config;
 };

const QString ShowfotoSettings::Private::configGroupDefault("Showfoto Settings");
const QString ShowfotoSettings::Private::configImageSortOrderEntry("Image Sort Order");
const QString ShowfotoSettings::Private::configImageSortingEntry("Image Sorting");
const QString ShowfotoSettings::Private::configImageGroupModeEntry("Image Group Mode");
const QString ShowfotoSettings::Private::configItemLeftClickActionEntry("Item Left Click Action");
const QString ShowfotoSettings::Private::configDefaultIconSizeEntry("Default Icon Size");
//const QString ShowfotoSettings::Private::configRatingFilterConditionEntry("Rating Filter Condition");
const QString ShowfotoSettings::Private::configIconShowNameEntry("Icon Show Name");
//const QString ShowfotoSettings::Private::configIconShowResolutionEntry("Icon Show Resolution");
const QString ShowfotoSettings::Private::configIconShowSizeEntry("Icon Show Size");
const QString ShowfotoSettings::Private::configIconShowDateEntry("Icon Show Date");
const QString ShowfotoSettings::Private::configIconShowTitleEntry("Icon Show Title");
const QString ShowfotoSettings::Private::configIconShowTagsEntry("Icon Show Tags");
const QString ShowfotoSettings::Private::configIconShowRatingEntry("Icon Show Rating");
const QString ShowfotoSettings::Private::configIconShowImageFormatEntry("Icon Show Image Format");
const QString ShowfotoSettings::Private::configIconShowOverlaysEntry("Icon Show Overlays");
const QString ShowfotoSettings::Private::configIconViewFontEntry("IconView Font");
const QString ShowfotoSettings::Private::configToolTipsFontEntry("ToolTips Font");
const QString ShowfotoSettings::Private::configShowToolTipsEntry("Show ToolTips");
const QString ShowfotoSettings::Private::configToolTipsShowFileNameEntry("ToolTips Show File Name");
const QString ShowfotoSettings::Private::configToolTipsShowFileDateEntry("ToolTips Show File Date");
const QString ShowfotoSettings::Private::configToolTipsShowFileSizeEntry("ToolTips Show File Size");
const QString ShowfotoSettings::Private::configToolTipsShowImageTypeEntry("ToolTips Show Image Type");
const QString ShowfotoSettings::Private::configToolTipsShowImageDimEntry("ToolTips Show Image Dim");
const QString ShowfotoSettings::Private::configToolTipsShowPhotoMakeEntry("ToolTips Show Photo Make");
const QString ShowfotoSettings::Private::configToolTipsShowPhotoDateEntry("ToolTips Show Photo Date");
const QString ShowfotoSettings::Private::configToolTipsShowPhotoFocalEntry("ToolTips Show Photo Focal");
const QString ShowfotoSettings::Private::configToolTipsShowPhotoExpoEntry("ToolTips Show Photo Expo");
const QString ShowfotoSettings::Private::configToolTipsShowPhotoFlashEntry("ToolTips Show Photo Flash");
const QString ShowfotoSettings::Private::configToolTipsShowPhotoWBEntry("ToolTips Show Photo WB");
const QString ShowfotoSettings::Private::configToolTipsShowFolderNameEntry("ToolTips Show Folder Name");
const QString ShowfotoSettings::Private::configToolTipsShowTagsEntry("ToolTips Show Tags");
const QString ShowfotoSettings::Private::configToolTipsShowLabelRatingEntry("ToolTips Show Label Rating");
const QString ShowfotoSettings::Private::configPreviewLoadFullImageSizeEntry("Preview Load Full Image Size");
const QString ShowfotoSettings::Private::configPreviewItemsWhileDownloadEntry("Preview Each Item While Downloading it");
const QString ShowfotoSettings::Private::configPreviewShowIconsEntry("Preview Show Icons");
const QString ShowfotoSettings::Private::configShowThumbbarEntry("Show Thumbbar");

// -------------------------------------------------------------------------------------------------

class ShowfotoSettingsCreator
{
public:

    ShowfotoSettings object;
};

K_GLOBAL_STATIC(ShowfotoSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ShowfotoSettings* ShowfotoSettings::instance()
{
    return &creator->object;
}

ShowfotoSettings::ShowfotoSettings()
    : QObject(), d(new Private)
{
    d->config = KGlobal::config();
    init();
    readSettings();
}

ShowfotoSettings::~ShowfotoSettings()
{
    delete d;
}

void ShowfotoSettings::init()
{
    d->imageSortOrder               = ShowfotoItemSortSettings::SortByFileName;
    d->imageSorting                 = ShowfotoItemSortSettings::AscendingOrder;
    d->imageGroupMode               = ShowfotoItemSortSettings::CategoryByFolder;
    d->itemLeftClickAction          = ShowfotoSettings::ShowPreview;

    d->thumbnailSize                = ThumbnailSize::Medium;

    d->iconShowName                 = true;
    d->iconShowSize                 = false;
    d->iconShowDate                 = true;
    d->iconShowTitle                = true;
    d->iconShowImageFormat          = false;
    d->iconShowOverlays             = true;
    d->iconShowRating               = true;
    d->iconShowTags                 = true;
    d->iconviewFont                 = KGlobalSettings::generalFont();

    d->toolTipsFont                 = KGlobalSettings::generalFont();
    d->showToolTips                 = false;
    d->tooltipShowFileName          = true;
    d->tooltipShowFileDate          = true;
    d->tooltipShowFileSize          = true;
    d->tooltipShowImageType         = true;
    d->tooltipShowImageDim          = true;
    d->tooltipShowPhotoMake         = true;
    d->tooltipShowPhotoFocal        = true;
    d->tooltipShowPhotoExpo         = true;
    d->tooltipShowPhotoFlash        = false;
    d->tooltipShowPhotoWb           = false;
    d->tooltipShowTags              = true;
    d->tooltipShowLabelRating       = true;

    d->previewLoadFullImageSize     = false;
    d->previewItemsWhileDownload    = false;
    d->previewShowIcons             = true;
    d->showThumbbar                 = true;
}

void ShowfotoSettings::readSettings()
{
    KSharedConfigPtr config         = d->config;

    KConfigGroup group              = config->group(d->configGroupDefault);

    d->imageSortOrder               = group.readEntry(d->configImageSortOrderEntry, (int)ShowfotoItemSortSettings::SortByFileName);
    d->imageSorting                 = group.readEntry(d->configImageSortingEntry,   (int)ShowfotoItemSortSettings::AscendingOrder);
    d->imageGroupMode               = group.readEntry(d->configImageGroupModeEntry, (int)ShowfotoItemSortSettings::CategoryByFolder);

    d->itemLeftClickAction          = ShowfotoSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                                                         (int)ShowfotoSettings::ShowPreview));

    d->thumbnailSize                = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);

    d->iconShowName                 = group.readEntry(d->configIconShowNameEntry,                 true);
//  d->iconShowResolution           = group.readEntry(d->configIconShowResolutionEntry,           false);
    d->iconShowSize                 = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                 = group.readEntry(d->configIconShowDateEntry,                 true);
    d->iconShowTitle                = group.readEntry(d->configIconShowTitleEntry,                true);
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
    d->tooltipShowPhotoFocal        = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo         = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
//  d->tooltipShowPhotoMode         = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
    d->tooltipShowPhotoFlash        = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb           = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowFolderName         = group.readEntry(d->configToolTipsShowFolderNameEntry,      false);
    d->tooltipShowTags              = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    d->tooltipShowLabelRating       = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->previewLoadFullImageSize     = group.readEntry(d->configPreviewLoadFullImageSizeEntry,     false);
    d->previewItemsWhileDownload    = group.readEntry(d->configPreviewItemsWhileDownloadEntry,    false);
    d->previewShowIcons             = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->showThumbbar                 = group.readEntry(d->configShowThumbbarEntry,                 true);

    // ---------------------------------------------------------------------

    emit setupChanged();
}

void ShowfotoSettings::saveSettings()
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
    group.writeEntry(d->configIconShowTitleEntry,                d->iconShowTitle);
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
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,       d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,        d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,       d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,          d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowFolderNameEntry,       d->tooltipShowFolderName);
    group.writeEntry(d->configToolTipsShowTagsEntry,             d->tooltipShowTags);
    group.writeEntry(d->configToolTipsShowLabelRatingEntry,      d->tooltipShowLabelRating);

    group.writeEntry(d->configPreviewLoadFullImageSizeEntry,     d->previewLoadFullImageSize);
    group.writeEntry(d->configPreviewItemsWhileDownloadEntry,    d->previewItemsWhileDownload);
    group.writeEntry(d->configPreviewShowIconsEntry,             d->previewShowIcons);
    group.writeEntry(d->configShowThumbbarEntry,                 d->showThumbbar);

    config->sync();
}

void ShowfotoSettings::emitSetupChanged()
{
    emit setupChanged();
}

void ShowfotoSettings::setImageSortOrder(int order)
{
    d->imageSortOrder = order;
}

int ShowfotoSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
}

void ShowfotoSettings::setImageSorting(int sorting)
{
    d->imageSorting = sorting;
}

int ShowfotoSettings::getImageSorting() const
{
    return d->imageSorting;
}

void ShowfotoSettings::setImageGroupMode(int mode)
{
    d->imageGroupMode = mode;
}

int ShowfotoSettings::getImageGroupMode() const
{
    return d->imageGroupMode;
}

void ShowfotoSettings::setItemLeftClickAction(const ItemLeftClickAction action)
{
    d->itemLeftClickAction = action;
}

ShowfotoSettings::ItemLeftClickAction ShowfotoSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

void ShowfotoSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int ShowfotoSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void ShowfotoSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont ShowfotoSettings::getIconViewFont() const
{
    return d->iconviewFont;
}

void ShowfotoSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool ShowfotoSettings::getIconShowName() const
{
    return d->iconShowName;
}

void ShowfotoSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool ShowfotoSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void ShowfotoSettings::setIconShowTitle(bool val)
{
    d->iconShowTitle = val;
}

bool ShowfotoSettings::getIconShowTitle() const
{
    return d->iconShowTitle;
}

void ShowfotoSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;
}

bool ShowfotoSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void ShowfotoSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool ShowfotoSettings::getIconShowDate() const
{
    return d->iconShowDate;
}

void ShowfotoSettings::setIconShowRating(bool val)
{
    d->iconShowRating = val;
}

bool ShowfotoSettings::getIconShowRating() const
{
    return d->iconShowRating;
}

void ShowfotoSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool ShowfotoSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

void ShowfotoSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool ShowfotoSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

void ShowfotoSettings::setToolTipsFont(const QFont& font)
{
    d->toolTipsFont = font;
}

QFont ShowfotoSettings::getToolTipsFont() const
{
    return d->toolTipsFont;
}

void ShowfotoSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool ShowfotoSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void ShowfotoSettings::setToolTipsShowFileName(bool val)
{
    d->tooltipShowFileName = val;
}

bool ShowfotoSettings::getToolTipsShowFileName() const
{
    return d->tooltipShowFileName;
}

void ShowfotoSettings::setToolTipsShowFileDate(bool val)
{
    d->tooltipShowFileDate = val;
}

bool ShowfotoSettings::getToolTipsShowFileDate() const
{
    return d->tooltipShowFileDate;
}

void ShowfotoSettings::setToolTipsShowFileSize(bool val)
{
    d->tooltipShowFileSize = val;
}

bool ShowfotoSettings::getToolTipsShowFileSize() const
{
    return d->tooltipShowFileSize;
}

void ShowfotoSettings::setToolTipsShowImageType(bool val)
{
    d->tooltipShowImageType = val;
}

bool ShowfotoSettings::getToolTipsShowImageType() const
{
    return d->tooltipShowImageType;
}

void ShowfotoSettings::setToolTipsShowImageDim(bool val)
{
    d->tooltipShowImageDim = val;
}

bool ShowfotoSettings::getToolTipsShowImageDim() const
{
    return d->tooltipShowImageDim;
}

void ShowfotoSettings::setToolTipsShowPhotoMake(bool val)
{
    d->tooltipShowPhotoMake = val;
}

bool ShowfotoSettings::getToolTipsShowPhotoMake() const
{
    return d->tooltipShowPhotoMake;
}

void ShowfotoSettings::setToolTipsShowPhotoFocal(bool val)
{
    d->tooltipShowPhotoFocal = val;
}

bool ShowfotoSettings::getToolTipsShowPhotoFocal() const
{
    return d->tooltipShowPhotoFocal;
}

void ShowfotoSettings::setToolTipsShowPhotoExpo(bool val)
{
    d->tooltipShowPhotoExpo = val;
}

bool ShowfotoSettings::getToolTipsShowPhotoExpo() const
{
    return d->tooltipShowPhotoExpo;
}

void ShowfotoSettings::setToolTipsShowPhotoFlash(bool val)
{
    d->tooltipShowPhotoFlash = val;
}

bool ShowfotoSettings::getToolTipsShowPhotoFlash() const
{
    return d->tooltipShowPhotoFlash;
}

void ShowfotoSettings::setToolTipsShowPhotoWB(bool val)
{
    d->tooltipShowPhotoWb = val;
}

bool ShowfotoSettings::getToolTipsShowPhotoWB() const
{
    return d->tooltipShowPhotoWb;
}

void ShowfotoSettings::setToolTipsShowTags(bool val)
{
    d->tooltipShowTags = val;
}

bool ShowfotoSettings::getToolTipsShowTags() const
{
    return d->tooltipShowTags;
}

void ShowfotoSettings::setToolTipsShowLabelRating(bool val)
{
    d->tooltipShowLabelRating = val;
}

bool ShowfotoSettings::getToolTipsShowLabelRating() const
{
    return d->tooltipShowLabelRating;
}

bool ShowfotoSettings::showToolTipsIsValid() const
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
            d->tooltipShowFolderName ||
            d->tooltipShowLabelRating ||
            d->tooltipShowTags)
        {
            return true;
        }
    }
    return true;
}

void ShowfotoSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool ShowfotoSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void ShowfotoSettings::setPreviewLoadFullImageSize(bool val)
{
    d->previewLoadFullImageSize = val;
}

bool ShowfotoSettings::getPreviewLoadFullImageSize() const
{
    return d->previewLoadFullImageSize;
}

void ShowfotoSettings::setPreviewItemsWhileDownload(bool val)
{
    d->previewItemsWhileDownload = val;
}

bool ShowfotoSettings::getPreviewItemsWhileDownload() const
{
    return d->previewItemsWhileDownload;
}

void ShowfotoSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ShowfotoSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

} // namespace ShowFoto
