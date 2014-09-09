/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2014      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
 * Copyright (C) 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "applicationsettings.moc"

// Qt includes

#include <QDBusInterface>
#include <QStyle>

// KDE includes

#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
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
#include "baloowrap.h"
#include "applicationsettings_p.h"

using namespace KDcrawIface;

namespace Digikam
{

class ApplicationSettingsCreator
{
public:

    ApplicationSettings object;
};

K_GLOBAL_STATIC(ApplicationSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ApplicationSettings* ApplicationSettings::instance()
{
    return &creator->object;
}

ApplicationSettings::ApplicationSettings()
    : QObject(), d(new Private(this))
{
    d->config = KGlobal::config();
    d->init();
    readSettings();

    // Init Max Thumbnail Size at startup.
    ThumbnailSize::readSettings(generalConfigGroup());
}

ApplicationSettings::~ApplicationSettings()
{
    delete d;
}

KConfigGroup ApplicationSettings::generalConfigGroup() const
{
    return d->config->group(d->configGroupGeneral);
}

void ApplicationSettings::applyBalooSettings()
{
#ifdef HAVE_BALOO
    BalooWrap::instance()->setSyncToBaloo(d->syncToBaloo);
    BalooWrap::instance()->setSyncToDigikam(d->syncToDigikam);
#endif
}

void ApplicationSettings::readSettings()
{
    KSharedConfigPtr config    = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group         = config->group(d->configGroupDefault);
    QStringList collectionList = group.readEntry(d->configAlbumCollectionsEntry, QStringList());

    if (!collectionList.isEmpty())
    {
        collectionList.sort();
        d->albumCategoryNames = collectionList;
    }

    d->albumSortOrder                   = ApplicationSettings::AlbumSortOrder(group.readEntry(d->configAlbumSortOrderEntry,
                                          (int)ApplicationSettings::ByFolder));

    d->imageSortOrder                   = group.readEntry(d->configImageSortOrderEntry,      (int)ImageSortSettings::SortByFileName);
    d->imageSorting                     = group.readEntry(d->configImageSortingEntry,        (int)ImageSortSettings::AscendingOrder);
    d->imageGroupMode                   = group.readEntry(d->configImageGroupModeEntry,      (int)ImageSortSettings::CategoryByAlbum);
    d->imageGroupSortOrder              = group.readEntry(d->configImageGroupSortOrderEntry, (int)ImageSortSettings::AscendingOrder);

    d->itemLeftClickAction              = ApplicationSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                          (int)ApplicationSettings::ShowPreview));

    d->thumbnailSize                    = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);
    d->treeThumbnailSize                = group.readEntry(d->configDefaultTreeIconSizeEntry,          22);
    d->treeviewFont                     = group.readEntry(d->configTreeViewFontEntry,                 KGlobalSettings::generalFont());
    d->currentTheme                     = group.readEntry(d->configThemeEntry,                        ThemeManager::instance()->defaultThemeName());

    d->sidebarTitleStyle                = (KMultiTabBar::KMultiTabBarStyle)group.readEntry(d->configSidebarTitleStyleEntry,
                                          (int)KMultiTabBar::VSNET);

    d->ratingFilterCond                 = group.readEntry(d->configRatingFilterConditionEntry,
                                          (int)ImageFilterSettings::GreaterEqualCondition);

    d->recursiveAlbums                  = group.readEntry(d->configRecursiveAlbumsEntry,              false);
    d->recursiveTags                    = group.readEntry(d->configRecursiveTagsEntry,                true);


    d->iconShowName                     = group.readEntry(d->configIconShowNameEntry,                 false);
    d->iconShowResolution               = group.readEntry(d->configIconShowResolutionEntry,           false);
    d->iconShowAspectRatio              = group.readEntry(d->configIconShowAspectRatioEntry,          false);
    d->iconShowSize                     = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                     = group.readEntry(d->configIconShowDateEntry,                 true);
    d->iconShowModDate                  = group.readEntry(d->configIconShowModificationDateEntry,     true);
    d->iconShowTitle                    = group.readEntry(d->configIconShowTitleEntry,                true);
    d->iconShowComments                 = group.readEntry(d->configIconShowCommentsEntry,             true);
    d->iconShowTags                     = group.readEntry(d->configIconShowTagsEntry,                 true);
    d->iconShowOverlays                 = group.readEntry(d->configIconShowOverlaysEntry,             true);
    d->iconShowRating                   = group.readEntry(d->configIconShowRatingEntry,               true);
    d->iconShowImageFormat              = group.readEntry(d->configIconShowImageFormatEntry,          false);
    d->iconShowCoordinates              = group.readEntry(d->configIconShowCoordinatesEntry,          false);
    d->iconviewFont                     = group.readEntry(d->configIconViewFontEntry,                 KGlobalSettings::generalFont());

    d->toolTipsFont                     = group.readEntry(d->configToolTipsFontEntry,                 KGlobalSettings::generalFont());
    d->showToolTips                     = group.readEntry(d->configShowToolTipsEntry,                 false);
    d->tooltipShowFileName              = group.readEntry(d->configToolTipsShowFileNameEntry,         true);
    d->tooltipShowFileDate              = group.readEntry(d->configToolTipsShowFileDateEntry,         false);
    d->tooltipShowFileSize              = group.readEntry(d->configToolTipsShowFileSizeEntry,         false);
    d->tooltipShowImageType             = group.readEntry(d->configToolTipsShowImageTypeEntry,        false);
    d->tooltipShowImageDim              = group.readEntry(d->configToolTipsShowImageDimEntry,         true);
    d->tooltipShowImageAR               = group.readEntry(d->configToolTipsShowImageAREntry,          true);
    d->tooltipShowPhotoMake             = group.readEntry(d->configToolTipsShowPhotoMakeEntry,        true);
    d->tooltipShowPhotoDate             = group.readEntry(d->configToolTipsShowPhotoDateEntry,        true);
    d->tooltipShowPhotoFocal            = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo             = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
    d->tooltipShowPhotoMode             = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
    d->tooltipShowPhotoFlash            = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb               = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowAlbumName             = group.readEntry(d->configToolTipsShowAlbumNameEntry,        false);
    d->tooltipShowComments              = group.readEntry(d->configToolTipsShowCommentsEntry,         true);
    d->tooltipShowTags                  = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    d->tooltipShowLabelRating           = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->tooltipShowVideoAspectRatio      = group.readEntry(d->configToolTipsShowVideoAspectRatioEntry,      true);
    d->tooltipShowVideoAudioBitRate     = group.readEntry(d->configToolTipsShowVideoAudioBitRateEntry,     true);
    d->tooltipShowVideoAudioChannelType = group.readEntry(d->configToolTipsShowVideoAudioChannelTypeEntry, true);
    d->tooltipShowVideoAudioCompressor  = group.readEntry(d->configToolTipsShowVideoAudioCompressorEntry,  true);
    d->tooltipShowVideoDuration         = group.readEntry(d->configToolTipsShowVideoDurationEntry,         true);
    d->tooltipShowVideoFrameRate        = group.readEntry(d->configToolTipsShowVideoFrameRateEntry,        true);
    d->tooltipShowVideoVideoCodec       = group.readEntry(d->configToolTipsShowVideoVideoCodecEntry,       true);

    d->showAlbumToolTips                = group.readEntry(d->configShowAlbumToolTipsEntry,            false);
    d->tooltipShowAlbumTitle            = group.readEntry(d->configToolTipsShowAlbumTitleEntry,       true);
    d->tooltipShowAlbumDate             = group.readEntry(d->configToolTipsShowAlbumDateEntry,        true);
    d->tooltipShowAlbumCollection       = group.readEntry(d->configToolTipsShowAlbumCollectionEntry,  true);
    d->tooltipShowAlbumCategory         = group.readEntry(d->configToolTipsShowAlbumCategoryEntry,    true);
    d->tooltipShowAlbumCaption          = group.readEntry(d->configToolTipsShowAlbumCaptionEntry,     true);

    d->previewLoadFullImageSize         = group.readEntry(d->configPreviewLoadFullImageSizeEntry,     false);
    d->previewShowIcons                 = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->showThumbbar                     = group.readEntry(d->configShowThumbbarEntry,                 true);

    d->showFolderTreeViewItemsCount     = group.readEntry(d->configShowFolderTreeViewItemsCountEntry, false);

    // ---------------------------------------------------------------------

    group                               = generalConfigGroup();

    d->showSplash                       = group.readEntry(d->configShowSplashEntry,                                  true);
    d->useTrash                         = group.readEntry(d->configUseTrashEntry,                                    true);
    d->showTrashDeleteDialog            = group.readEntry(d->configShowTrashDeleteDialogEntry,                       true);
    d->showPermanentDeleteDialog        = group.readEntry(d->configShowPermanentDeleteDialogEntry,                   true);
    d->sidebarApplyDirectly             = group.readEntry(d->configApplySidebarChangesDirectlyEntry,                 false);
    d->stringComparisonType             = (StringComparisonType) group.readEntry(d->configStringComparisonTypeEntry, (int) Natural);
    setApplicationStyle(group.readEntry(d->configApplicationStyleEntry, kapp->style()->objectName()));

    // ---------------------------------------------------------------------

    d->databaseParams.readFromConfig();

#ifdef HAVE_BALOO

    group                               = config->group(d->configGroupBaloo);

    d->syncToDigikam                    = group.readEntry(d->configSyncBalootoDigikamEntry, false);
    d->syncToBaloo                      = group.readEntry(d->configSyncDigikamtoBalooEntry, false);

    emit balooSettingsChanged();

#endif // HAVE_BALOO

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.readFromConfig(group);

    // ---------------------------------------------------------------------

    group                    = config->group(d->configGroupFaceDetection);
    d->faceDetectionAccuracy = group.readEntry(d->configFaceDetectionAccuracyEntry, double(0.8));

    emit setupChanged();
    emit recurseSettingsChanged();
    emit balooSettingsChanged();
}

void ApplicationSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configAlbumCollectionsEntry,                   d->albumCategoryNames);
    group.writeEntry(d->configAlbumSortOrderEntry,                     (int)d->albumSortOrder);
    group.writeEntry(d->configImageSortOrderEntry,                     (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortingEntry,                       (int)d->imageSorting);
    group.writeEntry(d->configImageGroupModeEntry,                     (int)d->imageGroupMode);
    group.writeEntry(d->configImageGroupSortOrderEntry,                (int)d->imageGroupSortOrder);

    group.writeEntry(d->configItemLeftClickActionEntry,                (int)d->itemLeftClickAction);
    group.writeEntry(d->configDefaultIconSizeEntry,                    QString::number(d->thumbnailSize));
    group.writeEntry(d->configDefaultTreeIconSizeEntry,                QString::number(d->treeThumbnailSize));
    group.writeEntry(d->configTreeViewFontEntry,                       d->treeviewFont);
    group.writeEntry(d->configRatingFilterConditionEntry,              d->ratingFilterCond);
    group.writeEntry(d->configRecursiveAlbumsEntry,                    d->recursiveAlbums);
    group.writeEntry(d->configRecursiveTagsEntry,                      d->recursiveTags);
    group.writeEntry(d->configThemeEntry,                              d->currentTheme);
    group.writeEntry(d->configSidebarTitleStyleEntry,                  (int)d->sidebarTitleStyle);

    group.writeEntry(d->configIconShowNameEntry,                       d->iconShowName);
    group.writeEntry(d->configIconShowResolutionEntry,                 d->iconShowResolution);
    group.writeEntry(d->configIconShowAspectRatioEntry,                d->iconShowAspectRatio);
    group.writeEntry(d->configIconShowSizeEntry,                       d->iconShowSize);
    group.writeEntry(d->configIconShowDateEntry,                       d->iconShowDate);
    group.writeEntry(d->configIconShowModificationDateEntry,           d->iconShowModDate);
    group.writeEntry(d->configIconShowTitleEntry,                      d->iconShowTitle);
    group.writeEntry(d->configIconShowCommentsEntry,                   d->iconShowComments);
    group.writeEntry(d->configIconShowTagsEntry,                       d->iconShowTags);
    group.writeEntry(d->configIconShowOverlaysEntry,                   d->iconShowOverlays);
    group.writeEntry(d->configIconShowRatingEntry,                     d->iconShowRating);
    group.writeEntry(d->configIconShowImageFormatEntry,                d->iconShowImageFormat);
    group.writeEntry(d->configIconShowCoordinatesEntry,                d->iconShowCoordinates);
    group.writeEntry(d->configIconViewFontEntry,                       d->iconviewFont);

    group.writeEntry(d->configToolTipsFontEntry,                       d->toolTipsFont);
    group.writeEntry(d->configShowToolTipsEntry,                       d->showToolTips);
    group.writeEntry(d->configToolTipsShowFileNameEntry,               d->tooltipShowFileName);
    group.writeEntry(d->configToolTipsShowFileDateEntry,               d->tooltipShowFileDate);
    group.writeEntry(d->configToolTipsShowFileSizeEntry,               d->tooltipShowFileSize);
    group.writeEntry(d->configToolTipsShowImageTypeEntry,              d->tooltipShowImageType);
    group.writeEntry(d->configToolTipsShowImageDimEntry,               d->tooltipShowImageDim);
    group.writeEntry(d->configToolTipsShowImageAREntry,                d->tooltipShowImageAR);
    group.writeEntry(d->configToolTipsShowPhotoMakeEntry,              d->tooltipShowPhotoMake);
    group.writeEntry(d->configToolTipsShowPhotoDateEntry,              d->tooltipShowPhotoDate);
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,             d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,              d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoModeEntry,              d->tooltipShowPhotoMode);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,             d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,                d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowAlbumNameEntry,              d->tooltipShowAlbumName);
    group.writeEntry(d->configToolTipsShowCommentsEntry,               d->tooltipShowComments);
    group.writeEntry(d->configToolTipsShowTagsEntry,                   d->tooltipShowTags);
    group.writeEntry(d->configToolTipsShowLabelRatingEntry,            d->tooltipShowLabelRating);

    group.writeEntry(d->configToolTipsShowVideoAspectRatioEntry,       d->tooltipShowVideoAspectRatio);
    group.writeEntry(d->configToolTipsShowVideoAudioBitRateEntry,      d->tooltipShowVideoAudioBitRate);
    group.writeEntry(d->configToolTipsShowVideoAudioChannelTypeEntry,  d->tooltipShowVideoAudioChannelType);
    group.writeEntry(d->configToolTipsShowVideoAudioCompressorEntry,   d->tooltipShowVideoAudioCompressor);
    group.writeEntry(d->configToolTipsShowVideoDurationEntry,          d->tooltipShowVideoDuration);
    group.writeEntry(d->configToolTipsShowVideoFrameRateEntry,         d->tooltipShowVideoFrameRate);
    group.writeEntry(d->configToolTipsShowVideoVideoCodecEntry,        d->tooltipShowVideoVideoCodec);

    group.writeEntry(d->configShowAlbumToolTipsEntry,                  d->showAlbumToolTips);
    group.writeEntry(d->configToolTipsShowAlbumTitleEntry,             d->tooltipShowAlbumTitle);
    group.writeEntry(d->configToolTipsShowAlbumDateEntry,              d->tooltipShowAlbumDate);
    group.writeEntry(d->configToolTipsShowAlbumCollectionEntry,        d->tooltipShowAlbumCollection);
    group.writeEntry(d->configToolTipsShowAlbumCategoryEntry,          d->tooltipShowAlbumCategory);
    group.writeEntry(d->configToolTipsShowAlbumCaptionEntry,           d->tooltipShowAlbumCaption);

    group.writeEntry(d->configPreviewLoadFullImageSizeEntry,           d->previewLoadFullImageSize);
    group.writeEntry(d->configPreviewShowIconsEntry,                   d->previewShowIcons);
    group.writeEntry(d->configShowThumbbarEntry,                       d->showThumbbar);

    group.writeEntry(d->configShowFolderTreeViewItemsCountEntry,       d->showFolderTreeViewItemsCount);

    // ---------------------------------------------------------------------

    group = generalConfigGroup();

    group.writeEntry(d->configShowSplashEntry,                         d->showSplash);
    group.writeEntry(d->configUseTrashEntry,                           d->useTrash);
    group.writeEntry(d->configShowTrashDeleteDialogEntry,              d->showTrashDeleteDialog);
    group.writeEntry(d->configShowPermanentDeleteDialogEntry,          d->showPermanentDeleteDialog);
    group.writeEntry(d->configApplySidebarChangesDirectlyEntry,        d->sidebarApplyDirectly);
    group.writeEntry(d->configStringComparisonTypeEntry,               (int) d->stringComparisonType);
    group.writeEntry(d->configApplicationStyleEntry,                   d->applicationStyle);

    // ---------------------------------------------------------------------

    d->databaseParams.writeToConfig();

#ifdef HAVE_BALOO

    group = config->group(d->configGroupBaloo);

    group.writeEntry(d->configSyncBalootoDigikamEntry,                 d->syncToDigikam);
    group.writeEntry(d->configSyncDigikamtoBalooEntry,                 d->syncToBaloo);

#endif // HAVE_BALOO

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.writeToConfig(group);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupFaceDetection);

    group.writeEntry(d->configFaceDetectionAccuracyEntry,             d->faceDetectionAccuracy);

    config->sync();
}

void ApplicationSettings::emitSetupChanged()
{
    emit setupChanged();
}

QString ApplicationSettings::getDatabaseFilePath() const
{
    return d->databaseParams.getDatabaseNameOrDir();
}

void ApplicationSettings::setDatabaseFilePath(const QString& path)
{
    d->databaseParams.setDatabasePath(path);
    d->databaseParams.setThumbsDatabasePath(path);
}

void ApplicationSettings::setShowSplashScreen(bool val)
{
    d->showSplash = val;
}

bool ApplicationSettings::getShowSplashScreen() const
{
    return d->showSplash;
}

void ApplicationSettings::setAlbumCategoryNames(const QStringList& list)
{
    d->albumCategoryNames = list;
}

QStringList ApplicationSettings::getAlbumCategoryNames() const
{
    return d->albumCategoryNames;
}

bool ApplicationSettings::addAlbumCategoryName(const QString& name)
{
    if (d->albumCategoryNames.contains(name))
    {
        return false;
    }

    d->albumCategoryNames.append(name);
    return true;
}

bool ApplicationSettings::delAlbumCategoryName(const QString& name)
{
    uint count = d->albumCategoryNames.removeAll(name);
    return (count > 0) ? true : false;
}

void ApplicationSettings::setAlbumSortOrder(const ApplicationSettings::AlbumSortOrder order)
{
    d->albumSortOrder = order;
}

ApplicationSettings::AlbumSortOrder ApplicationSettings::getAlbumSortOrder() const
{
    return d->albumSortOrder;
}

void ApplicationSettings::setImageSortOrder(int order)
{
    d->imageSortOrder = order;
}

int ApplicationSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
}

void ApplicationSettings::setImageSorting(int sorting)
{
    d->imageSorting = sorting;
}

int ApplicationSettings::getImageSorting() const
{
    return d->imageSorting;
}

void ApplicationSettings::setImageGroupMode(int mode)
{
    d->imageGroupMode = mode;
}

int ApplicationSettings::getImageGroupMode() const
{
    return d->imageGroupMode;
}

void ApplicationSettings::setImageGroupSortOrder(int order)
{
    d->imageGroupSortOrder = order;
}

int ApplicationSettings::getImageGroupSortOrder() const
{
    return d->imageGroupSortOrder;
}

void ApplicationSettings::setItemLeftClickAction(const ItemLeftClickAction action)
{
    d->itemLeftClickAction = action;
}

ApplicationSettings::ItemLeftClickAction ApplicationSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

QString ApplicationSettings::getImageFileFilter() const
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

QString ApplicationSettings::getMovieFileFilter() const
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

QString ApplicationSettings::getAudioFileFilter() const
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

QString ApplicationSettings::getRawFileFilter() const
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

QString ApplicationSettings::getAllFileFilter() const
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

void ApplicationSettings::addToImageFileFilter(const QString& extensions)
{
    DatabaseAccess().db()->addToUserImageFilterSettings(extensions);
}

void ApplicationSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int ApplicationSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void ApplicationSettings::setTreeViewIconSize(int val)
{
    d->treeThumbnailSize = val;
}

int ApplicationSettings::getTreeViewIconSize() const
{
    return ((d->treeThumbnailSize < 8) || (d->treeThumbnailSize > 48)) ? 48 : d->treeThumbnailSize;
}

void ApplicationSettings::setTreeViewFont(const QFont& font)
{
    d->treeviewFont = font;
}

QFont ApplicationSettings::getTreeViewFont() const
{
    return d->treeviewFont;
}

void ApplicationSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont ApplicationSettings::getIconViewFont() const
{
    return d->iconviewFont;
}

void ApplicationSettings::setRatingFilterCond(int val)
{
    d->ratingFilterCond = val;
}

int ApplicationSettings::getRatingFilterCond() const
{
    return d->ratingFilterCond;
}

void ApplicationSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool ApplicationSettings::getIconShowName() const
{
    return d->iconShowName;
}

void ApplicationSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool ApplicationSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void ApplicationSettings::setIconShowTitle(bool val)
{
    d->iconShowTitle = val;
}

bool ApplicationSettings::getIconShowTitle() const
{
    return d->iconShowTitle;
}

void ApplicationSettings::setIconShowComments(bool val)
{
    d->iconShowComments = val;
}

bool ApplicationSettings::getIconShowComments() const
{
    return d->iconShowComments;
}

void ApplicationSettings::setIconShowResolution(bool val)
{
    d->iconShowResolution = val;
}

bool ApplicationSettings::getIconShowResolution() const
{
    return d->iconShowResolution;
}

void ApplicationSettings::setIconShowAspectRatio(bool val)
{
    d->iconShowAspectRatio = val;
}

bool ApplicationSettings::getIconShowAspectRatio() const
{
    return d->iconShowAspectRatio;
}

void ApplicationSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;
}

bool ApplicationSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void ApplicationSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool ApplicationSettings::getIconShowDate() const
{
    return d->iconShowDate;
}

void ApplicationSettings::setIconShowModDate(bool val)
{
    d->iconShowModDate = val;
}

bool ApplicationSettings::getIconShowModDate() const
{
    return d->iconShowModDate;
}

void ApplicationSettings::setIconShowRating(bool val)
{
    d->iconShowRating = val;
}

bool ApplicationSettings::getIconShowRating() const
{
    return d->iconShowRating;
}

void ApplicationSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool ApplicationSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

void ApplicationSettings::setIconShowCoordinates(bool val)
{
    d->iconShowCoordinates = val;
}

bool ApplicationSettings::getIconShowCoordinates() const
{
    return d->iconShowCoordinates;
}

void ApplicationSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool ApplicationSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

void ApplicationSettings::setToolTipsFont(const QFont& font)
{
    d->toolTipsFont = font;
}

QFont ApplicationSettings::getToolTipsFont() const
{
    return d->toolTipsFont;
}

void ApplicationSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool ApplicationSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void ApplicationSettings::setToolTipsShowFileName(bool val)
{
    d->tooltipShowFileName = val;
}

bool ApplicationSettings::getToolTipsShowFileName() const
{
    return d->tooltipShowFileName;
}

void ApplicationSettings::setToolTipsShowFileDate(bool val)
{
    d->tooltipShowFileDate = val;
}

bool ApplicationSettings::getToolTipsShowFileDate() const
{
    return d->tooltipShowFileDate;
}

void ApplicationSettings::setToolTipsShowFileSize(bool val)
{
    d->tooltipShowFileSize = val;
}

bool ApplicationSettings::getToolTipsShowFileSize() const
{
    return d->tooltipShowFileSize;
}

void ApplicationSettings::setToolTipsShowImageType(bool val)
{
    d->tooltipShowImageType = val;
}

bool ApplicationSettings::getToolTipsShowImageType() const
{
    return d->tooltipShowImageType;
}

void ApplicationSettings::setToolTipsShowImageDim(bool val)
{
    d->tooltipShowImageDim = val;
}

bool ApplicationSettings::getToolTipsShowImageDim() const
{
    return d->tooltipShowImageDim;
}

void ApplicationSettings::setToolTipsShowImageAR(bool val)
{
    d->tooltipShowImageAR = val;
}

bool ApplicationSettings::getToolTipsShowImageAR() const
{
    return d->tooltipShowImageAR;
}

void ApplicationSettings::setToolTipsShowPhotoMake(bool val)
{
    d->tooltipShowPhotoMake = val;
}

bool ApplicationSettings::getToolTipsShowPhotoMake() const
{
    return d->tooltipShowPhotoMake;
}

void ApplicationSettings::setToolTipsShowPhotoDate(bool val)
{
    d->tooltipShowPhotoDate = val;
}

bool ApplicationSettings::getToolTipsShowPhotoDate() const
{
    return d->tooltipShowPhotoDate;
}

void ApplicationSettings::setToolTipsShowPhotoFocal(bool val)
{
    d->tooltipShowPhotoFocal = val;
}

bool ApplicationSettings::getToolTipsShowPhotoFocal() const
{
    return d->tooltipShowPhotoFocal;
}

void ApplicationSettings::setToolTipsShowPhotoExpo(bool val)
{
    d->tooltipShowPhotoExpo = val;
}

bool ApplicationSettings::getToolTipsShowPhotoExpo() const
{
    return d->tooltipShowPhotoExpo;
}

void ApplicationSettings::setToolTipsShowPhotoMode(bool val)
{
    d->tooltipShowPhotoMode = val;
}

bool ApplicationSettings::getToolTipsShowPhotoMode() const
{
    return d->tooltipShowPhotoMode;
}

void ApplicationSettings::setToolTipsShowPhotoFlash(bool val)
{
    d->tooltipShowPhotoFlash = val;
}

bool ApplicationSettings::getToolTipsShowPhotoFlash() const
{
    return d->tooltipShowPhotoFlash;
}

void ApplicationSettings::setToolTipsShowPhotoWB(bool val)
{
    d->tooltipShowPhotoWb = val;
}

bool ApplicationSettings::getToolTipsShowPhotoWB() const
{
    return d->tooltipShowPhotoWb;
}

void ApplicationSettings::setToolTipsShowAlbumName(bool val)
{
    d->tooltipShowAlbumName = val;
}

bool ApplicationSettings::getToolTipsShowAlbumName() const
{
    return d->tooltipShowAlbumName;
}

void ApplicationSettings::setToolTipsShowComments(bool val)
{
    d->tooltipShowComments = val;
}

bool ApplicationSettings::getToolTipsShowComments() const
{
    return d->tooltipShowComments;
}

void ApplicationSettings::setToolTipsShowTags(bool val)
{
    d->tooltipShowTags = val;
}

bool ApplicationSettings::getToolTipsShowTags() const
{
    return d->tooltipShowTags;
}

void ApplicationSettings::setToolTipsShowLabelRating(bool val)
{
    d->tooltipShowLabelRating = val;
}

bool ApplicationSettings::getToolTipsShowLabelRating() const
{
    return d->tooltipShowLabelRating;
}

void ApplicationSettings::setShowAlbumToolTips(bool val)
{
    d->showAlbumToolTips = val;
}

bool ApplicationSettings::getShowAlbumToolTips() const
{
    return d->showAlbumToolTips;
}

void ApplicationSettings::setToolTipsShowAlbumTitle(bool val)
{
    d->tooltipShowAlbumTitle = val;
}

bool ApplicationSettings::getToolTipsShowAlbumTitle() const
{
    return d->tooltipShowAlbumTitle;
}

void ApplicationSettings::setToolTipsShowAlbumDate(bool val)
{
    d->tooltipShowAlbumDate = val;
}

bool ApplicationSettings::getToolTipsShowAlbumDate() const
{
    return d->tooltipShowAlbumDate;
}

void ApplicationSettings::setToolTipsShowAlbumCollection(bool val)
{
    d->tooltipShowAlbumCollection = val;
}

bool ApplicationSettings::getToolTipsShowAlbumCollection() const
{
    return d->tooltipShowAlbumCollection;
}

void ApplicationSettings::setToolTipsShowAlbumCategory(bool val)
{
    d->tooltipShowAlbumCategory = val;
}

bool ApplicationSettings::getToolTipsShowAlbumCategory() const
{
    return d->tooltipShowAlbumCategory;
}

void ApplicationSettings::setToolTipsShowAlbumCaption(bool val)
{
    d->tooltipShowAlbumCaption = val;
}

bool ApplicationSettings::getToolTipsShowAlbumCaption() const
{
    return d->tooltipShowAlbumCaption;
}

void ApplicationSettings::setToolTipsShowVideoAspectRatio(bool val)
{
    d->tooltipShowVideoAspectRatio = val;
}

bool ApplicationSettings::getToolTipsShowVideoAspectRatio() const
{
    return d->tooltipShowVideoAspectRatio;
}

void ApplicationSettings::setToolTipsShowVideoAudioBitRate(bool val)
{
    d->tooltipShowVideoAudioBitRate = val;
}

bool ApplicationSettings::getToolTipsShowVideoAudioBitRate() const
{
    return d->tooltipShowVideoAudioBitRate;
}

void ApplicationSettings::setToolTipsShowVideoAudioChannelType(bool val)
{
    d->tooltipShowVideoAudioChannelType = val;
}

bool ApplicationSettings::getToolTipsShowVideoAudioChannelType() const
{
    return d->tooltipShowVideoAudioChannelType;
}

void ApplicationSettings::setToolTipsShowVideoAudioCompressor(bool val)
{
    d->tooltipShowVideoAudioCompressor = val;
}

bool ApplicationSettings::getToolTipsShowVideoAudioCompressor() const
{
    return d->tooltipShowVideoAudioCompressor;
}

void ApplicationSettings::setToolTipsShowVideoDuration(bool val)
{
    d->tooltipShowVideoDuration = val;
}

bool ApplicationSettings::getToolTipsShowVideoDuration() const
{
    return d->tooltipShowVideoDuration;
}

void ApplicationSettings::setToolTipsShowVideoFrameRate(bool val)
{
    d->tooltipShowVideoFrameRate = val;
}

bool ApplicationSettings::getToolTipsShowVideoFrameRate() const
{
    return d->tooltipShowVideoFrameRate;
}

void ApplicationSettings::setToolTipsShowVideoVideoCodec(bool val)
{
    d->tooltipShowVideoVideoCodec = val;
}

bool ApplicationSettings::getToolTipsShowVideoVideoCodec() const
{
    return d->tooltipShowVideoVideoCodec;
}

void ApplicationSettings::setCurrentTheme(const QString& theme)
{
    d->currentTheme = theme;
}

QString ApplicationSettings::getCurrentTheme() const
{
    return d->currentTheme;
}

void ApplicationSettings::setSidebarTitleStyle(KMultiTabBar::KMultiTabBarStyle style)
{
    d->sidebarTitleStyle = style;
}

KMultiTabBar::KMultiTabBarStyle ApplicationSettings::getSidebarTitleStyle() const
{
    return d->sidebarTitleStyle;
}

void ApplicationSettings::setUseTrash(bool val)
{
    d->useTrash = val;
}

bool ApplicationSettings::getUseTrash() const
{
    return d->useTrash;
}

void ApplicationSettings::setShowTrashDeleteDialog(bool val)
{
    d->showTrashDeleteDialog = val;
}

bool ApplicationSettings::getShowTrashDeleteDialog() const
{
    return d->showTrashDeleteDialog;
}

void ApplicationSettings::setShowPermanentDeleteDialog(bool val)
{
    d->showPermanentDeleteDialog = val;
}

bool ApplicationSettings::getShowPermanentDeleteDialog() const
{
    return d->showPermanentDeleteDialog;
}

void ApplicationSettings::setApplySidebarChangesDirectly(bool val)
{
    d->sidebarApplyDirectly= val;
}

bool ApplicationSettings::getApplySidebarChangesDirectly() const
{
    return d->sidebarApplyDirectly;
}

bool ApplicationSettings::showToolTipsIsValid() const
{
    if (d->showToolTips)
    {
        if (d->tooltipShowFileName   ||
            d->tooltipShowFileDate   ||
            d->tooltipShowFileSize   ||
            d->tooltipShowImageType  ||
            d->tooltipShowImageDim   ||
            d->tooltipShowImageAR    ||
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

bool ApplicationSettings::showAlbumToolTipsIsValid() const
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

void ApplicationSettings::setPreviewLoadFullImageSize(bool val)
{
    d->previewLoadFullImageSize = val;
}

bool ApplicationSettings::getPreviewLoadFullImageSize() const
{
    return d->previewLoadFullImageSize;
}

void ApplicationSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ApplicationSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

void ApplicationSettings::setRecurseAlbums(bool val)
{
    d->recursiveAlbums = val;
    emit recurseSettingsChanged();
}

bool ApplicationSettings::getRecurseAlbums() const
{
    return d->recursiveAlbums;
}

void ApplicationSettings::setRecurseTags(bool val)
{
    d->recursiveTags = val;
    emit recurseSettingsChanged();
}

bool ApplicationSettings::getRecurseTags() const
{
    return d->recursiveTags;
}

void ApplicationSettings::setShowFolderTreeViewItemsCount(bool val)
{
    d->showFolderTreeViewItemsCount = val;
}

bool ApplicationSettings::getShowFolderTreeViewItemsCount() const
{
    return d->showFolderTreeViewItemsCount;
}

void ApplicationSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool ApplicationSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void ApplicationSettings::setSyncBalooToDigikam(bool val)
{
    d->syncToDigikam = val;
    emit balooSettingsChanged();
}

bool ApplicationSettings::getSyncBalooToDigikam() const
{
    return d->syncToDigikam;
}

void ApplicationSettings::setSyncDigikamToBaloo(bool val)
{
    d->syncToBaloo = val;
    emit balooSettingsChanged();
}

bool ApplicationSettings::getSyncDigikamToBaloo() const
{
    return d->syncToBaloo;
}

DatabaseParameters ApplicationSettings::getDatabaseParameters() const
{
    return d->databaseParams;
}

void ApplicationSettings::setDatabaseParameters(const DatabaseParameters& params)
{
    d->databaseParams = params;
}

QString ApplicationSettings::getDatabaseType() const
{
    return d->databaseParams.databaseType;
}

void ApplicationSettings::setDatabaseType(const QString& databaseType)
{
    d->databaseParams.databaseType = databaseType;
}

QString ApplicationSettings::getDatabaseConnectoptions() const
{
    return d->databaseParams.connectOptions;
}

QString ApplicationSettings::getDatabaseName() const
{
    return d->databaseParams.databaseName;
}

QString ApplicationSettings::getDatabaseNameThumbnails() const
{
    return d->databaseParams.databaseNameThumbnails;
}

QString ApplicationSettings::getDatabaseHostName() const
{
    return d->databaseParams.hostName;
}

QString ApplicationSettings::getDatabasePassword() const
{
    return d->databaseParams.password;
}

int ApplicationSettings::getDatabasePort() const
{
    return d->databaseParams.port;
}

QString ApplicationSettings::getDatabaseUserName() const
{
    return d->databaseParams.userName;
}

bool ApplicationSettings::getInternalDatabaseServer() const
{
    return d->databaseParams.internalServer;
}

void ApplicationSettings::setDatabaseConnectoptions(const QString& connectoptions)
{
    d->databaseParams.connectOptions = connectoptions;
}

void ApplicationSettings::setDatabaseName(const QString& databaseName)
{
    d->databaseParams.databaseName = databaseName;
}

void ApplicationSettings::setDatabaseNameThumbnails(const QString& databaseNameThumbnails)
{
    d->databaseParams.databaseNameThumbnails = databaseNameThumbnails;
}

void ApplicationSettings::setDatabaseHostName(const QString& hostName)
{
    d->databaseParams.hostName = hostName;
}

void ApplicationSettings::setDatabasePassword(const QString& password)
{
    d->databaseParams.password = password;
}

void ApplicationSettings::setDatabasePort(int port)
{
    d->databaseParams.port = port;
}

void ApplicationSettings::setDatabaseUserName(const QString& userName)
{
    d->databaseParams.userName = userName;
}

void ApplicationSettings::setInternalDatabaseServer(const bool useInternalDBServer)
{
    d->databaseParams.internalServer = useInternalDBServer;
}

void ApplicationSettings::setStringComparisonType(ApplicationSettings::StringComparisonType val)
{
    d->stringComparisonType = val;
}

ApplicationSettings::StringComparisonType ApplicationSettings::getStringComparisonType() const
{
    return d->stringComparisonType;
}

void ApplicationSettings::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    d->versionSettings = settings;
}

VersionManagerSettings ApplicationSettings::getVersionManagerSettings() const
{
    return d->versionSettings;
}

double ApplicationSettings::getFaceDetectionAccuracy() const
{
    return d->faceDetectionAccuracy;
}

void ApplicationSettings::setFaceDetectionAccuracy(double value)
{
    d->faceDetectionAccuracy = value;
}

void ApplicationSettings::setApplicationStyle(const QString& style)
{
    if (d->applicationStyle != style)
    {
        d->applicationStyle = style;
        kapp->setStyle(d->applicationStyle);
    }
}

QString ApplicationSettings::getApplicationStyle() const
{
    return d->applicationStyle;
}

void ApplicationSettings::setAlbumSortChanged(bool val)
{
    d->albumSortChanged = val;
}

bool ApplicationSettings::getAlbumSortChanged() const
{
    return d->albumSortChanged;
}

}  // namespace Digikam
