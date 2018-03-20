/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2014-2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "applicationsettings.h"

// Qt includes

#include <QApplication>
#include <QFontDatabase>

// Local includes

#include "digikam_config.h"
#include "imagefiltersettings.h"
#include "imagesortsettings.h"
#include "mimefilter.h"
#include "thumbnailsize.h"
#include "thememanager.h"
#include "applicationsettings_p.h"

#ifdef HAVE_KFILEMETADATA
#include "baloowrap.h"
#endif

namespace Digikam
{

class ApplicationSettingsCreator
{
public:

    ApplicationSettings object;
};

Q_GLOBAL_STATIC(ApplicationSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ApplicationSettings* ApplicationSettings::instance()
{
    return &creator->object;
}

ApplicationSettings::ApplicationSettings()
    : QObject(),
      d(new Private(this))
{
    d->config = KSharedConfig::openConfig();
    d->init();
    readSettings();

    // Init Max Thumbnail Size at startup.
    ThumbnailSize::readSettings(generalConfigGroup());
}

ApplicationSettings::~ApplicationSettings()
{
    delete d;
}

QString ApplicationSettings::generalConfigGroupName() const
{
    return d->configGroupGeneral;
}

KConfigGroup ApplicationSettings::generalConfigGroup() const
{
    return d->config->group(generalConfigGroupName());
}

void ApplicationSettings::emitSetupChanged()
{
    emit setupChanged();
}

void ApplicationSettings::applyBalooSettings()
{
#ifdef HAVE_KFILEMETADATA
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

    d->albumSortRole                     = ApplicationSettings::AlbumSortRole(group.readEntry(d->configAlbumSortRoleEntry,
                                           (int)ApplicationSettings::ByFolder));

    d->imageSortOrder                    = group.readEntry(d->configImageSortOrderEntry,      (int)ImageSortSettings::SortByFileName);
    d->imageSorting                      = group.readEntry(d->configImageSortingEntry,        (int)ImageSortSettings::AscendingOrder);
    d->imageSeparationMode                = group.readEntry(d->configImageSeparationModeEntry,      (int)ImageSortSettings::CategoryByAlbum);
    d->imageSeparationSortOrder           = group.readEntry(d->configImageSeparationSortOrderEntry, (int)ImageSortSettings::AscendingOrder);

    d->itemLeftClickAction               = ApplicationSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                           (int)ApplicationSettings::ShowPreview));

    d->thumbnailSize                     = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);
    d->treeThumbnailSize                 = group.readEntry(d->configDefaultTreeIconSizeEntry,          22);
    d->treeviewFont                      = group.readEntry(d->configTreeViewFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    d->currentTheme                      = group.readEntry(d->configThemeEntry,                        ThemeManager::instance()->defaultThemeName());

    d->sidebarTitleStyle                 = (DMultiTabBar::TextStyle)group.readEntry(d->configSidebarTitleStyleEntry,
                                           (int)DMultiTabBar::AllIconsText);

    d->ratingFilterCond                  = group.readEntry(d->configRatingFilterConditionEntry,
                                           (int)ImageFilterSettings::GreaterEqualCondition);

    d->recursiveAlbums                   = group.readEntry(d->configRecursiveAlbumsEntry,              false);
    d->recursiveTags                     = group.readEntry(d->configRecursiveTagsEntry,                true);

    d->iconShowName                      = group.readEntry(d->configIconShowNameEntry,                 false);
    d->iconShowResolution                = group.readEntry(d->configIconShowResolutionEntry,           false);
    d->iconShowAspectRatio               = group.readEntry(d->configIconShowAspectRatioEntry,          false);
    d->iconShowSize                      = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                      = group.readEntry(d->configIconShowDateEntry,                 false);
    d->iconShowModDate                   = group.readEntry(d->configIconShowModificationDateEntry,     false);
    d->iconShowTitle                     = group.readEntry(d->configIconShowTitleEntry,                true);
    d->iconShowComments                  = group.readEntry(d->configIconShowCommentsEntry,             true);
    d->iconShowTags                      = group.readEntry(d->configIconShowTagsEntry,                 true);
    d->iconShowOverlays                  = group.readEntry(d->configIconShowOverlaysEntry,             true);
    d->iconShowFullscreen                = group.readEntry(d->configIconShowFullscreenEntry,           true);
    d->iconShowRating                    = group.readEntry(d->configIconShowRatingEntry,               true);
    d->iconShowImageFormat               = group.readEntry(d->configIconShowImageFormatEntry,          true);
    d->iconShowCoordinates               = group.readEntry(d->configIconShowCoordinatesEntry,          true);
    d->iconviewFont                      = group.readEntry(d->configIconViewFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    d->toolTipsFont                      = group.readEntry(d->configToolTipsFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    d->showToolTips                      = group.readEntry(d->configShowToolTipsEntry,                 false);
    d->tooltipShowFileName               = group.readEntry(d->configToolTipsShowFileNameEntry,         true);
    d->tooltipShowFileDate               = group.readEntry(d->configToolTipsShowFileDateEntry,         false);
    d->tooltipShowFileSize               = group.readEntry(d->configToolTipsShowFileSizeEntry,         false);
    d->tooltipShowImageType              = group.readEntry(d->configToolTipsShowImageTypeEntry,        false);
    d->tooltipShowImageDim               = group.readEntry(d->configToolTipsShowImageDimEntry,         true);
    d->tooltipShowImageAR                = group.readEntry(d->configToolTipsShowImageAREntry,          true);
    d->tooltipShowPhotoMake              = group.readEntry(d->configToolTipsShowPhotoMakeEntry,        true);
    d->tooltipShowPhotoLens              = group.readEntry(d->configToolTipsShowPhotoLensEntry,        true);
    d->tooltipShowPhotoDate              = group.readEntry(d->configToolTipsShowPhotoDateEntry,        true);
    d->tooltipShowPhotoFocal             = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo              = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
    d->tooltipShowPhotoMode              = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
    d->tooltipShowPhotoFlash             = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb                = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowAlbumName              = group.readEntry(d->configToolTipsShowAlbumNameEntry,        false);
    d->tooltipShowTitles                 = group.readEntry(d->configToolTipsShowTitlesEntry,           true);
    d->tooltipShowComments               = group.readEntry(d->configToolTipsShowCommentsEntry,         true);
    d->tooltipShowTags                   = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    d->tooltipShowLabelRating            = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->tooltipShowVideoAspectRatio       = group.readEntry(d->configToolTipsShowVideoAspectRatioEntry,      true);
    d->tooltipShowVideoAudioBitRate      = group.readEntry(d->configToolTipsShowVideoAudioBitRateEntry,     true);
    d->tooltipShowVideoAudioChannelType  = group.readEntry(d->configToolTipsShowVideoAudioChannelTypeEntry, true);
    d->tooltipShowVideoAudioCodec   = group.readEntry(d->configToolTipsShowVideoAudioCodecEntry,  true);
    d->tooltipShowVideoDuration          = group.readEntry(d->configToolTipsShowVideoDurationEntry,         true);
    d->tooltipShowVideoFrameRate         = group.readEntry(d->configToolTipsShowVideoFrameRateEntry,        true);
    d->tooltipShowVideoVideoCodec        = group.readEntry(d->configToolTipsShowVideoVideoCodecEntry,       true);

    d->showAlbumToolTips                 = group.readEntry(d->configShowAlbumToolTipsEntry,            false);
    d->tooltipShowAlbumTitle             = group.readEntry(d->configToolTipsShowAlbumTitleEntry,       true);
    d->tooltipShowAlbumDate              = group.readEntry(d->configToolTipsShowAlbumDateEntry,        true);
    d->tooltipShowAlbumCollection        = group.readEntry(d->configToolTipsShowAlbumCollectionEntry,  true);
    d->tooltipShowAlbumCategory          = group.readEntry(d->configToolTipsShowAlbumCategoryEntry,    true);
    d->tooltipShowAlbumCaption           = group.readEntry(d->configToolTipsShowAlbumCaptionEntry,     true);
    d->tooltipShowAlbumPreview           = group.readEntry(d->configToolTipsShowAlbumPreviewEntry,     false);

    if (group.readEntry(d->configPreviewLoadFullItemSizeEntry, true))
    {
        d->previewSettings.quality = PreviewSettings::HighQualityPreview;
        if (group.readEntry(d->configPreviewRawUseEmbeddedPreview, false))
        {
            d->previewSettings.rawLoading = PreviewSettings::RawPreviewFromEmbeddedPreview;
        }
        else if (group.readEntry(d->configPreviewRawUseHalfSizeData, false))
        {
            d->previewSettings.rawLoading = PreviewSettings::RawPreviewFromRawHalfSize;
        }
        else
        {
            d->previewSettings.rawLoading = PreviewSettings::RawPreviewAutomatic;
        }
    }
    else
    {
        d->previewSettings.quality = PreviewSettings::FastPreview;
    }

    d->previewSettings.convertToEightBit = group.readEntry(d->configPreviewConvertToEightBitEntry,     true);
    d->previewSettings.zoomOrgSize       = group.readEntry(d->configPreviewZoomOrgSizeEntry,           true);
    d->previewShowIcons                  = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->showThumbbar                      = group.readEntry(d->configShowThumbbarEntry,                 true);

    d->showFolderTreeViewItemsCount      = group.readEntry(d->configShowFolderTreeViewItemsCountEntry, false);

    // ---------------------------------------------------------------------

    group                                = generalConfigGroup();

    d->showSplash                        = group.readEntry(d->configShowSplashEntry,                                  true);
    d->useTrash                          = group.readEntry(d->configUseTrashEntry,                                    true);
    d->showTrashDeleteDialog             = group.readEntry(d->configShowTrashDeleteDialogEntry,                       true);
    d->showPermanentDeleteDialog         = group.readEntry(d->configShowPermanentDeleteDialogEntry,                   true);
    d->sidebarApplyDirectly              = group.readEntry(d->configApplySidebarChangesDirectlyEntry,                 false);
    d->useNativeFileDialog               = group.readEntry(d->configUseNativeFileDialogEntry,                         false);
    d->drawFramesToGrouped               = group.readEntry(d->configDrawFramesToGroupedEntry,                         true);
    d->scrollItemToCenter                = group.readEntry(d->configScrollItemToCenterEntry,                          false);
    d->showOnlyPersonTagsInPeopleSidebar = group.readEntry(d->configShowOnlyPersonTagsInPeopleSidebarEntry,           true);
    d->stringComparisonType              = (StringComparisonType) group.readEntry(d->configStringComparisonTypeEntry, (int) Natural);

#ifdef HAVE_APPSTYLE_SUPPORT
    setApplicationStyle(group.readEntry(d->configApplicationStyleEntry, qApp->style()->objectName()));
#else
    setApplicationStyle(QLatin1String("Fusion"));
#endif

    d->iconTheme                         = group.readEntry(d->configIconThemeEntry,                                   QString());
    d->scanAtStart                       = group.readEntry(d->configScanAtStartEntry,                                 true);
    d->cleanAtStart                      = group.readEntry(d->configCleanAtStartEntry,                                false);

    // ---------------------------------------------------------------------

    d->databaseParams.readFromConfig();

#ifdef HAVE_KFILEMETADATA

    group                                = config->group(d->configGroupBaloo);

    d->syncToDigikam                     = group.readEntry(d->configSyncBalootoDigikamEntry, false);
    d->syncToBaloo                       = group.readEntry(d->configSyncDigikamtoBalooEntry, false);

    emit balooSettingsChanged();

#endif // HAVE_KFILEMETADATA

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.readFromConfig(group);

    // ---------------------------------------------------------------------

    group                    = config->group(d->configGroupFaceDetection);
    d->faceDetectionAccuracy = group.readEntry(d->configFaceDetectionAccuracyEntry, double(0.8));

    // ---------------------------------------------------------------------

    group                    = config->group(d->configGroupDuplicatesSearch);

    d->minimumSimilarityBound                = group.readEntry(d->configMinimumSimilarityBound,               40);
    d->duplicatesSearchLastMinSimilarity     = group.readEntry(d->configDuplicatesSearchLastMinSimilarity,    90);
    d->duplicatesSearchLastMaxSimilarity     = group.readEntry(d->configDuplicatesSearchLastMaxSimilarity,    100);
    d->duplicatesSearchLastAlbumTagRelation  = group.readEntry(d->configDuplicatesSearchLastAlbumTagRelation, 0);
    d->duplicatesSearchLastRestrictions      = group.readEntry(d->configDuplicatesSearchLastRestrictions,     0);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupGrouping);

    for (ApplicationSettings::OperationModes::key_iterator it = d->groupingOperateOnAll.keyBegin();
         it != d->groupingOperateOnAll.keyEnd(); ++it)
    {
        d->groupingOperateOnAll.insert(*it, (ApplicationSettings::ApplyToEntireGroup)group.readEntry(
                                              d->configGroupingOperateOnAll.value(*it), (int)ApplicationSettings::Ask));
    }

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
    group.writeEntry(d->configAlbumSortRoleEntry,                      (int)d->albumSortRole);
    group.writeEntry(d->configImageSortOrderEntry,                     (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortingEntry,                       (int)d->imageSorting);
    group.writeEntry(d->configImageSeparationModeEntry,                     (int)d->imageSeparationMode);
    group.writeEntry(d->configImageSeparationSortOrderEntry,                (int)d->imageSeparationSortOrder);

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
    group.writeEntry(d->configIconShowFullscreenEntry,                 d->iconShowFullscreen);
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
    group.writeEntry(d->configToolTipsShowPhotoLensEntry,              d->tooltipShowPhotoLens);
    group.writeEntry(d->configToolTipsShowPhotoDateEntry,              d->tooltipShowPhotoDate);
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,             d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,              d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoModeEntry,              d->tooltipShowPhotoMode);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,             d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,                d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowAlbumNameEntry,              d->tooltipShowAlbumName);
    group.writeEntry(d->configToolTipsShowTitlesEntry,                 d->tooltipShowTitles);
    group.writeEntry(d->configToolTipsShowCommentsEntry,               d->tooltipShowComments);
    group.writeEntry(d->configToolTipsShowTagsEntry,                   d->tooltipShowTags);
    group.writeEntry(d->configToolTipsShowLabelRatingEntry,            d->tooltipShowLabelRating);

    group.writeEntry(d->configToolTipsShowVideoAspectRatioEntry,       d->tooltipShowVideoAspectRatio);
    group.writeEntry(d->configToolTipsShowVideoAudioBitRateEntry,      d->tooltipShowVideoAudioBitRate);
    group.writeEntry(d->configToolTipsShowVideoAudioChannelTypeEntry,  d->tooltipShowVideoAudioChannelType);
    group.writeEntry(d->configToolTipsShowVideoAudioCodecEntry,   d->tooltipShowVideoAudioCodec);
    group.writeEntry(d->configToolTipsShowVideoDurationEntry,          d->tooltipShowVideoDuration);
    group.writeEntry(d->configToolTipsShowVideoFrameRateEntry,         d->tooltipShowVideoFrameRate);
    group.writeEntry(d->configToolTipsShowVideoVideoCodecEntry,        d->tooltipShowVideoVideoCodec);

    group.writeEntry(d->configShowAlbumToolTipsEntry,                  d->showAlbumToolTips);
    group.writeEntry(d->configToolTipsShowAlbumTitleEntry,             d->tooltipShowAlbumTitle);
    group.writeEntry(d->configToolTipsShowAlbumDateEntry,              d->tooltipShowAlbumDate);
    group.writeEntry(d->configToolTipsShowAlbumCollectionEntry,        d->tooltipShowAlbumCollection);
    group.writeEntry(d->configToolTipsShowAlbumCategoryEntry,          d->tooltipShowAlbumCategory);
    group.writeEntry(d->configToolTipsShowAlbumCaptionEntry,           d->tooltipShowAlbumCaption);
    group.writeEntry(d->configToolTipsShowAlbumPreviewEntry,           d->tooltipShowAlbumPreview);

    if (d->previewSettings.quality == PreviewSettings::HighQualityPreview)
    {
        group.writeEntry(d->configPreviewLoadFullItemSizeEntry, true);
        switch (d->previewSettings.rawLoading)
        {
            case PreviewSettings::RawPreviewAutomatic:
                group.writeEntry(d->configPreviewRawUseEmbeddedPreview, false);
                group.writeEntry(d->configPreviewRawUseHalfSizeData,    false);
                break;
            case PreviewSettings::RawPreviewFromEmbeddedPreview:
                group.writeEntry(d->configPreviewRawUseEmbeddedPreview, true);
                group.writeEntry(d->configPreviewRawUseHalfSizeData,    false);
                break;
            case PreviewSettings::RawPreviewFromRawHalfSize:
                group.writeEntry(d->configPreviewRawUseEmbeddedPreview, false);
                group.writeEntry(d->configPreviewRawUseHalfSizeData,    true);
                break;
        }
    }
    else
    {
        group.writeEntry(d->configPreviewLoadFullItemSizeEntry, false);
    }

    group.writeEntry(d->configPreviewConvertToEightBitEntry,           d->previewSettings.convertToEightBit);
    group.writeEntry(d->configPreviewZoomOrgSizeEntry,                 d->previewSettings.zoomOrgSize);
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
    group.writeEntry(d->configUseNativeFileDialogEntry,                d->useNativeFileDialog);
    group.writeEntry(d->configDrawFramesToGroupedEntry,                d->drawFramesToGrouped);
    group.writeEntry(d->configScrollItemToCenterEntry,                 d->scrollItemToCenter);
    group.writeEntry(d->configShowOnlyPersonTagsInPeopleSidebarEntry,  d->showOnlyPersonTagsInPeopleSidebar);
    group.writeEntry(d->configStringComparisonTypeEntry,               (int) d->stringComparisonType);
#ifdef HAVE_APPSTYLE_SUPPORT
    group.writeEntry(d->configApplicationStyleEntry,                   d->applicationStyle);
#endif
    group.writeEntry(d->configIconThemeEntry,                          d->iconTheme);
    group.writeEntry(d->configScanAtStartEntry,                        d->scanAtStart);
    group.writeEntry(d->configCleanAtStartEntry,                       d->cleanAtStart);

    // ---------------------------------------------------------------------

    d->databaseParams.writeToConfig();

#ifdef HAVE_KFILEMETADATA

    group = config->group(d->configGroupBaloo);

    group.writeEntry(d->configSyncBalootoDigikamEntry,                 d->syncToDigikam);
    group.writeEntry(d->configSyncDigikamtoBalooEntry,                 d->syncToBaloo);

#endif // HAVE_KFILEMETADATA

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.writeToConfig(group);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupFaceDetection);

    group.writeEntry(d->configFaceDetectionAccuracyEntry,              d->faceDetectionAccuracy);

    group = config->group(d->configGroupDuplicatesSearch);

    group.writeEntry(d->configMinimumSimilarityBound,                  d->minimumSimilarityBound);
    group.writeEntry(d->configDuplicatesSearchLastMinSimilarity,       d->duplicatesSearchLastMinSimilarity);
    group.writeEntry(d->configDuplicatesSearchLastMaxSimilarity,       d->duplicatesSearchLastMaxSimilarity);
    group.writeEntry(d->configDuplicatesSearchLastAlbumTagRelation,    d->duplicatesSearchLastAlbumTagRelation);
    group.writeEntry(d->configDuplicatesSearchLastRestrictions,        d->duplicatesSearchLastRestrictions);

    group = config->group(d->configGroupGrouping);

    for (ApplicationSettings::OperationModes::key_iterator it = d->groupingOperateOnAll.keyBegin();
         it != d->groupingOperateOnAll.keyEnd(); ++it)
    {
        group.writeEntry(d->configGroupingOperateOnAll.value(*it),
                         (int)d->groupingOperateOnAll.value(*it));
    }

    config->sync();
}

}  // namespace Digikam
