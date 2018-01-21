/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-15
 * Description : Settings for the import tool
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importsettings.h"

// Qt includes

#include <QFontDatabase>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "camitemsortsettings.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ImportSettings::Private
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
        iconShowCoordinates(false),
        thumbnailSize(0),
        imageSortOrder(0),
        imageSortBy(0),
        imageSeparationMode(0),
        itemLeftClickAction(ImportSettings::ShowPreview),
        showToolTips(false),
        tooltipShowFileName(false),
        tooltipShowFileDate(false),
        tooltipShowFileSize(false),
        tooltipShowImageType(false),
        tooltipShowImageDim(true),
        tooltipShowPhotoMake(false),
        tooltipShowPhotoLens(false),
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
    static const QString                configImageSortByEntry;
    static const QString                configImageSeparationModeEntry;
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
    static const QString                configIconShowCoordinatesEntry;
    static const QString                configIconViewFontEntry;
    static const QString                configToolTipsFontEntry;
    static const QString                configShowToolTipsEntry;
    static const QString                configToolTipsShowFileNameEntry;
    static const QString                configToolTipsShowFileDateEntry;
    static const QString                configToolTipsShowFileSizeEntry;
    static const QString                configToolTipsShowImageTypeEntry;
    static const QString                configToolTipsShowImageDimEntry;
    static const QString                configToolTipsShowPhotoMakeEntry;
    static const QString                configToolTipsShowPhotoLensEntry;
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

    // Import icon-view settings
    bool                                iconShowName;
    bool                                iconShowSize;
    bool                                iconShowDate;
    bool                                iconShowTitle;
    bool                                iconShowResolution;
    bool                                iconShowTags;
    bool                                iconShowOverlays;
    bool                                iconShowRating;
    bool                                iconShowImageFormat;
    bool                                iconShowCoordinates;

    QFont                               iconviewFont;

    int                                 thumbnailSize;
    int                                 imageSortOrder;
    int                                 imageSortBy;
    int                                 imageSeparationMode;
    ImportSettings::ItemLeftClickAction itemLeftClickAction;

    // Import icon-view tooltip settings
    bool                                showToolTips;
    bool                                tooltipShowFileName;
    bool                                tooltipShowFileDate;
    bool                                tooltipShowFileSize;
    bool                                tooltipShowImageType;
    bool                                tooltipShowImageDim;
    bool                                tooltipShowPhotoMake;
    bool                                tooltipShowPhotoLens;
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

const QString ImportSettings::Private::configGroupDefault(QLatin1String("Import Settings"));
const QString ImportSettings::Private::configImageSortOrderEntry(QLatin1String("Image Sort Order"));
const QString ImportSettings::Private::configImageSortByEntry(QLatin1String("Image Sorting")); // TODO not changed due to backwards compatibility
const QString ImportSettings::Private::configImageSeparationModeEntry(QLatin1String("Image Separation Mode"));
const QString ImportSettings::Private::configItemLeftClickActionEntry(QLatin1String("Item Left Click Action"));
const QString ImportSettings::Private::configDefaultIconSizeEntry(QLatin1String("Default Icon Size"));
//const QString ImportSettings::Private::configRatingFilterConditionEntry(QLatin1String("Rating Filter Condition"));
const QString ImportSettings::Private::configIconShowNameEntry(QLatin1String("Icon Show Name"));
//const QString ImportSettings::Private::configIconShowResolutionEntry(QLatin1String("Icon Show Resolution"));
const QString ImportSettings::Private::configIconShowSizeEntry(QLatin1String("Icon Show Size"));
const QString ImportSettings::Private::configIconShowDateEntry(QLatin1String("Icon Show Date"));
const QString ImportSettings::Private::configIconShowTitleEntry(QLatin1String("Icon Show Title"));
const QString ImportSettings::Private::configIconShowTagsEntry(QLatin1String("Icon Show Tags"));
const QString ImportSettings::Private::configIconShowRatingEntry(QLatin1String("Icon Show Rating"));
const QString ImportSettings::Private::configIconShowImageFormatEntry(QLatin1String("Icon Show Image Format"));
const QString ImportSettings::Private::configIconShowCoordinatesEntry(QLatin1String("Icon Show Coordinates"));
const QString ImportSettings::Private::configIconShowOverlaysEntry(QLatin1String("Icon Show Overlays"));
const QString ImportSettings::Private::configIconViewFontEntry(QLatin1String("IconView Font"));
const QString ImportSettings::Private::configToolTipsFontEntry(QLatin1String("ToolTips Font"));
const QString ImportSettings::Private::configShowToolTipsEntry(QLatin1String("Show ToolTips"));
const QString ImportSettings::Private::configToolTipsShowFileNameEntry(QLatin1String("ToolTips Show File Name"));
const QString ImportSettings::Private::configToolTipsShowFileDateEntry(QLatin1String("ToolTips Show File Date"));
const QString ImportSettings::Private::configToolTipsShowFileSizeEntry(QLatin1String("ToolTips Show File Size"));
const QString ImportSettings::Private::configToolTipsShowImageTypeEntry(QLatin1String("ToolTips Show Image Type"));
const QString ImportSettings::Private::configToolTipsShowImageDimEntry(QLatin1String("ToolTips Show Image Dim"));
const QString ImportSettings::Private::configToolTipsShowPhotoMakeEntry(QLatin1String("ToolTips Show Photo Make"));
const QString ImportSettings::Private::configToolTipsShowPhotoLensEntry(QLatin1String("ToolTips Show Photo Lens"));
const QString ImportSettings::Private::configToolTipsShowPhotoDateEntry(QLatin1String("ToolTips Show Photo Date"));
const QString ImportSettings::Private::configToolTipsShowPhotoFocalEntry(QLatin1String("ToolTips Show Photo Focal"));
const QString ImportSettings::Private::configToolTipsShowPhotoExpoEntry(QLatin1String("ToolTips Show Photo Expo"));
const QString ImportSettings::Private::configToolTipsShowPhotoFlashEntry(QLatin1String("ToolTips Show Photo Flash"));
const QString ImportSettings::Private::configToolTipsShowPhotoWBEntry(QLatin1String("ToolTips Show Photo WB"));
const QString ImportSettings::Private::configToolTipsShowFolderNameEntry(QLatin1String("ToolTips Show Folder Name"));
const QString ImportSettings::Private::configToolTipsShowTagsEntry(QLatin1String("ToolTips Show Tags"));
const QString ImportSettings::Private::configToolTipsShowLabelRatingEntry(QLatin1String("ToolTips Show Label Rating"));
const QString ImportSettings::Private::configPreviewLoadFullImageSizeEntry(QLatin1String("Preview Load Full Image Size"));
const QString ImportSettings::Private::configPreviewItemsWhileDownloadEntry(QLatin1String("Preview Each Item While Downloading it"));
const QString ImportSettings::Private::configPreviewShowIconsEntry(QLatin1String("Preview Show Icons"));
const QString ImportSettings::Private::configShowThumbbarEntry(QLatin1String("Show Thumbbar"));

// -------------------------------------------------------------------------------------------------

class ImportSettingsCreator
{
public:

    ImportSettings object;
};

Q_GLOBAL_STATIC(ImportSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ImportSettings* ImportSettings::instance()
{
    return &creator->object;
}

ImportSettings::ImportSettings()
    : QObject(),
      d(new Private)
{
    d->config = KSharedConfig::openConfig();
    init();
    readSettings();
}

ImportSettings::~ImportSettings()
{
    delete d;
}

void ImportSettings::init()
{
    d->imageSortBy                  = CamItemSortSettings::SortByFileName;
    d->imageSortOrder               = CamItemSortSettings::AscendingOrder;
    d->imageSeparationMode          = CamItemSortSettings::CategoryByFolder;
    d->itemLeftClickAction          = ImportSettings::ShowPreview;

    d->thumbnailSize                = ThumbnailSize::Medium;

    d->iconShowName                 = true;
    d->iconShowSize                 = false;
    d->iconShowDate                 = true;
    d->iconShowTitle                = true;
    d->iconShowImageFormat          = false;
    d->iconShowOverlays             = true;
    d->iconShowRating               = true;
    d->iconShowTags                 = true;
    d->iconviewFont                 = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    d->toolTipsFont                 = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    d->showToolTips                 = false;
    d->tooltipShowFileName          = true;
    d->tooltipShowFileDate          = false;
    d->tooltipShowFileSize          = false;
    d->tooltipShowImageType         = false;
    d->tooltipShowImageDim          = true;
    d->tooltipShowPhotoMake         = true;
    d->tooltipShowPhotoLens         = true;
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

void ImportSettings::readSettings()
{
    KSharedConfigPtr config         = d->config;

    KConfigGroup group              = config->group(d->configGroupDefault);

    d->imageSortOrder               = group.readEntry(d->configImageSortOrderEntry, (int)CamItemSortSettings::AscendingOrder);
    d->imageSortBy                  = group.readEntry(d->configImageSortByEntry,    (int)CamItemSortSettings::SortByFileName);
    d->imageSeparationMode          = group.readEntry(d->configImageSeparationModeEntry, (int)CamItemSortSettings::CategoryByFolder);

    d->itemLeftClickAction          = ImportSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                                                         (int)ImportSettings::ShowPreview));

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
    d->iconShowCoordinates          = group.readEntry(d->configIconShowCoordinatesEntry,          false);
    d->iconviewFont                 = group.readEntry(d->configIconViewFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    d->toolTipsFont                 = group.readEntry(d->configToolTipsFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    d->showToolTips                 = group.readEntry(d->configShowToolTipsEntry,                 false);
    d->tooltipShowFileName          = group.readEntry(d->configToolTipsShowFileNameEntry,         true);
    d->tooltipShowFileDate          = group.readEntry(d->configToolTipsShowFileDateEntry,         false);
    d->tooltipShowFileSize          = group.readEntry(d->configToolTipsShowFileSizeEntry,         false);
    d->tooltipShowImageType         = group.readEntry(d->configToolTipsShowImageTypeEntry,        false);
    d->tooltipShowImageDim          = group.readEntry(d->configToolTipsShowImageDimEntry,         true);
    d->tooltipShowPhotoMake         = group.readEntry(d->configToolTipsShowPhotoMakeEntry,        true);
    d->tooltipShowPhotoLens         = group.readEntry(d->configToolTipsShowPhotoLensEntry,        true);
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

void ImportSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configImageSortOrderEntry,               (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortByEntry,                  (int)d->imageSortBy);
    group.writeEntry(d->configImageSeparationModeEntry,          (int)d->imageSeparationMode);
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
    group.writeEntry(d->configIconShowCoordinatesEntry,          d->iconShowCoordinates);
    group.writeEntry(d->configIconViewFontEntry,                 d->iconviewFont);

    group.writeEntry(d->configToolTipsFontEntry,                 d->toolTipsFont);
    group.writeEntry(d->configShowToolTipsEntry,                 d->showToolTips);
    group.writeEntry(d->configToolTipsShowFileNameEntry,         d->tooltipShowFileName);
    group.writeEntry(d->configToolTipsShowFileDateEntry,         d->tooltipShowFileDate);
    group.writeEntry(d->configToolTipsShowFileSizeEntry,         d->tooltipShowFileSize);
    group.writeEntry(d->configToolTipsShowImageTypeEntry,        d->tooltipShowImageType);
    group.writeEntry(d->configToolTipsShowImageDimEntry,         d->tooltipShowImageDim);
    group.writeEntry(d->configToolTipsShowPhotoMakeEntry,        d->tooltipShowPhotoMake);
    group.writeEntry(d->configToolTipsShowPhotoLensEntry,        d->tooltipShowPhotoLens);
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

void ImportSettings::setImageSortBy(int sortBy)
{
    d->imageSortBy = sortBy;
}

int ImportSettings::getImageSortBy() const
{
    return d->imageSortBy;
}

void ImportSettings::setImageSeparationMode(int mode)
{
    d->imageSeparationMode = mode;
}

int ImportSettings::getImageSeparationMode() const
{
    return d->imageSeparationMode;
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

void ImportSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;
}

bool ImportSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void ImportSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool ImportSettings::getIconShowDate() const
{
    return d->iconShowDate;
}

void ImportSettings::setIconShowRating(bool val)
{
    d->iconShowRating = val;
}

bool ImportSettings::getIconShowRating() const
{
    return d->iconShowRating;
}

void ImportSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool ImportSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

void ImportSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool ImportSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

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

void ImportSettings::setToolTipsShowPhotoLens(bool val)
{
    d->tooltipShowPhotoLens = val;
}

bool ImportSettings::getToolTipsShowPhotoLens() const
{
    return d->tooltipShowPhotoLens;
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

void ImportSettings::setToolTipsShowTags(bool val)
{
    d->tooltipShowTags = val;
}

bool ImportSettings::getToolTipsShowTags() const
{
    return d->tooltipShowTags;
}

void ImportSettings::setToolTipsShowLabelRating(bool val)
{
    d->tooltipShowLabelRating = val;
}

bool ImportSettings::getToolTipsShowLabelRating() const
{
    return d->tooltipShowLabelRating;
}

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
            d->tooltipShowPhotoLens  ||
            d->tooltipShowPhotoFocal ||
            d->tooltipShowPhotoExpo  ||
            d->tooltipShowPhotoFlash ||
            d->tooltipShowPhotoWb    ||
            d->tooltipShowFolderName ||
            d->tooltipShowLabelRating) /*||
            d->tooltipShowTags)*/
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

void ImportSettings::setPreviewItemsWhileDownload(bool val)
{
    d->previewItemsWhileDownload = val;
}

bool ImportSettings::getPreviewItemsWhileDownload() const
{
    return d->previewItemsWhileDownload;
}

void ImportSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ImportSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

void ImportSettings::setIconShowCoordinates(bool val)
{
    d->iconShowCoordinates = val;
}

bool ImportSettings::getIconShowCoordinates() const
{
    return d->iconShowCoordinates;
}

} // namespace Digikam
