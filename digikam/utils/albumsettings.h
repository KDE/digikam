/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : albums settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMSETTINGS_H
#define ALBUMSETTINGS_H

// Qt includes

#include <QStringList>
#include <QString>
#include <QFont>
#include <QObject>

// KDE includes

#include <kmultitabbar.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DatabaseParameters;
class VersionManagerSettings;

class AlbumSettings : public QObject
{
    Q_OBJECT

public:

    enum AlbumSortOrder
    {
        ByFolder = 0,
        ByCategory,
        ByDate
    };

    enum ItemLeftClickAction
    {
        ShowPreview = 0,
        StartEditor
    };

    /**
     * Possible ways of comparing strings.
     */
    enum StringComparisonType
    {

        /**
         * Natural compare using KStringHandler::naturalCompare.
         */
        Natural = 0,

        /**
         * Normal comparison using Qt's compare function.
         */
        Normal

    };

Q_SIGNALS:

    void setupChanged();

    void recurseSettingsChanged();
    void nepomukSettingsChanged();

public:

    static AlbumSettings* instance();

    void readSettings();
    void saveSettings();

    void emitSetupChanged();

    bool showToolTipsIsValid()      const;
    bool showAlbumToolTipsIsValid() const;

    void setImgDatabaseFilePath(const QString& path);
    QString getImgDatabaseFilePath() const;

    void setTmbDatabaseFilePath(const QString& path);
    QString getTmbDatabaseFilePath() const;

    void setShowSplashScreen(bool val);
    bool getShowSplashScreen() const;

    void setScanAtStart(bool val);
    bool getScanAtStart() const;

    void setAlbumCategoryNames(const QStringList& list);
    QStringList getAlbumCategoryNames();

    bool addAlbumCategoryName(const QString& name);
    bool delAlbumCategoryName(const QString& name);

    void setAlbumSortOrder(const AlbumSortOrder order);
    AlbumSortOrder getAlbumSortOrder() const;

    void setImageSortOrder(int order);
    int getImageSortOrder() const;

    // means ascending or descending
    void setImageSorting(int sorting);
    int getImageSorting() const;

    void setImageGroupMode(int mode);
    int getImageGroupMode() const;

    void setItemLeftClickAction(const ItemLeftClickAction action);
    ItemLeftClickAction getItemLeftClickAction() const;

    QString getImageFileFilter() const;
    void addToImageFileFilter(const QString& extensions);

    QString getMovieFileFilter() const;

    QString getAudioFileFilter() const;

    QString getRawFileFilter() const;

    bool    addImageFileExtension(const QString& ext);
    QString getAllFileFilter() const;

    void setDefaultIconSize(int val);
    int  getDefaultIconSize() const;

    void setTreeViewIconSize(int val);
    int  getTreeViewIconSize() const;

    void setTreeViewFont(const QFont& font);
    QFont getTreeViewFont() const;

    void setIconViewFont(const QFont& font);
    QFont getIconViewFont() const;

    void setRatingFilterCond(int val);
    int  getRatingFilterCond() const;

    void setIconShowName(bool val);
    bool getIconShowName() const;

    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowComments(bool val);
    bool getIconShowComments() const;

    void setIconShowResolution(bool val);
    bool getIconShowResolution() const;

    void setIconShowTags(bool val);
    bool getIconShowTags() const;

    void setIconShowDate(bool val);
    bool getIconShowDate() const;

    void setIconShowModDate(bool val);
    bool getIconShowModDate() const;

    void setIconShowRating(bool val);
    bool getIconShowRating() const;

    void setIconShowImageFormat(bool val);
    bool getIconShowImageFormat() const;

    /**
     * Sets the visibility of the overlay buttons on the image icons.
     */
    void setIconShowOverlays(bool val);
    /**
     * Determines whether the overlay buttons should be displayed on the icons.
     */
    bool getIconShowOverlays() const;

    void setToolTipsFont(const QFont& font);
    QFont getToolTipsFont() const;

    void setShowToolTips(bool val);
    bool getShowToolTips() const;

    void setToolTipsShowFileName(bool val);
    bool getToolTipsShowFileName() const;

    void setToolTipsShowFileDate(bool val);
    bool getToolTipsShowFileDate() const;

    void setToolTipsShowFileSize(bool val);
    bool getToolTipsShowFileSize() const;

    void setToolTipsShowImageType(bool val);
    bool getToolTipsShowImageType() const;

    void setToolTipsShowImageDim(bool val);
    bool getToolTipsShowImageDim() const;

    void setToolTipsShowPhotoMake(bool val);
    bool getToolTipsShowPhotoMake() const;

    void setToolTipsShowPhotoDate(bool val);
    bool getToolTipsShowPhotoDate() const;

    void setToolTipsShowPhotoFocal(bool val);
    bool getToolTipsShowPhotoFocal() const;

    void setToolTipsShowPhotoExpo(bool val);
    bool getToolTipsShowPhotoExpo() const;

    void setToolTipsShowPhotoMode(bool val);
    bool getToolTipsShowPhotoMode() const;

    void setToolTipsShowPhotoFlash(bool val);
    bool getToolTipsShowPhotoFlash() const;

    void setToolTipsShowPhotoWB(bool val);
    bool getToolTipsShowPhotoWB() const;

    void setToolTipsShowAlbumName(bool val);
    bool getToolTipsShowAlbumName() const;

    void setToolTipsShowComments(bool val);
    bool getToolTipsShowComments() const;

    void setToolTipsShowTags(bool val);
    bool getToolTipsShowTags() const;

    void setToolTipsShowLabelRating(bool val);
    bool getToolTipsShowLabelRating() const;

    void setShowAlbumToolTips(bool val);
    bool getShowAlbumToolTips() const;

    void setToolTipsShowAlbumTitle(bool val);
    bool getToolTipsShowAlbumTitle() const;

    void setToolTipsShowAlbumDate(bool val);
    bool getToolTipsShowAlbumDate() const;

    void setToolTipsShowAlbumCollection(bool val);
    bool getToolTipsShowAlbumCollection() const;

    void setToolTipsShowAlbumCategory(bool val);
    bool getToolTipsShowAlbumCategory() const;

    void setToolTipsShowAlbumCaption(bool val);
    bool getToolTipsShowAlbumCaption() const;

    void    setCurrentTheme(const QString& theme);
    QString getCurrentTheme() const;

    void setSidebarTitleStyle(KMultiTabBar::KMultiTabBarStyle style);
    KMultiTabBar::KMultiTabBarStyle getSidebarTitleStyle() const;

    void    setUseTrash(bool val);
    bool    getUseTrash() const;

    void    setShowTrashDeleteDialog(bool val);
    bool    getShowTrashDeleteDialog() const;

    void    setShowPermanentDeleteDialog(bool val);
    bool    getShowPermanentDeleteDialog() const;

    void    setApplySidebarChangesDirectly(bool val);
    bool    getApplySidebarChangesDirectly() const;

    void setPreviewLoadFullImageSize(bool val);
    bool getPreviewLoadFullImageSize() const;

    void setPreviewShowIcons(bool val);
    bool getPreviewShowIcons() const;

    void setShowFolderTreeViewItemsCount(bool val);
    bool getShowFolderTreeViewItemsCount() const;

    void setRecurseAlbums(bool val);
    bool getRecurseAlbums() const;

    void setRecurseTags(bool val);
    bool getRecurseTags() const;

    void setShowThumbbar(bool val);
    bool getShowThumbbar() const;

    void setSyncNepomukToDigikam(bool val);
    bool getSyncNepomukToDigikam() const;

    void setSyncDigikamToNepomuk(bool val);
    bool getSyncDigikamToNepomuk() const;

    /**
     * Defines the way in which string comparisons are performed.
     *
     * @param val new way to compare strings
     */
    void setStringComparisonType(AlbumSettings::StringComparisonType val);
    /**
     * Tells in which way strings are compared at the moment.
     *
     * @return string comparison type to use.
     */
    StringComparisonType getStringComparisonType() const;

    DatabaseParameters getDatabaseParameters() const;
    void setDatabaseParameters(const DatabaseParameters& params);

    QString getImgDatabaseType() const;
    void setImgDatabaseType(const QString& databaseType);

    QString getImgDatabaseConnectoptions() const;
    void setImgDatabaseConnectoptions(const QString& connectoptions);

    QString getImgDatabaseName() const;
    void setImgDatabaseName(const QString& databaseName);

    QString getImgDatabaseHostName() const;
    void setImgDatabaseHostName(const QString& hostName);

    QString getImgDatabasePassword() const;
    void setImgDatabasePassword(const QString& password);

    int getImgDatabasePort() const;
    void setImgDatabasePort(int port);

    QString getImgDatabaseUserName() const;
    void setImgDatabaseUserName(const QString& userName);

    QString getTmbDatabaseType() const;
    void setTmbDatabaseType(const QString& databaseType);

    QString getTmbDatabaseConnectoptions() const;
    void setTmbDatabaseConnectoptions(const QString& connectoptions);

    QString getTmbDatabaseName() const;
    void setTmbDatabaseName(const QString& databaseName);

    QString getTmbDatabaseHostName() const;
    void setTmbDatabaseHostName(const QString& hostName);

    QString getTmbDatabasePassword() const;
    void setTmbDatabasePassword(const QString& password);

    int getTmbDatabasePort() const;
    void setTmbDatabasePort(int port);

    QString getTmbDatabaseUserName() const;
    void setTmbDatabaseUserName(const QString& userName);

    bool getInternalDatabaseServer() const;
    void setInternalDatabaseServer(const bool useInternalDBServer);

    void setVersionManagerSettings(const VersionManagerSettings& settings);
    VersionManagerSettings getVersionManagerSettings() const;

    double getFaceDetectionAccuracy() const;
    void setFaceDetectionAccuracy(double value);

    void setApplicationStyle(const QString& style);
    QString getApplicationStyle() const;

public Q_SLOTS:

    void applyNepomukSettings() const;
    void triggerResyncWithNepomuk() const;

private:

    AlbumSettings();
    ~AlbumSettings();

    void init();

private:

    friend class AlbumSettingsCreator;

    class AlbumSettingsPrivate;
    AlbumSettingsPrivate* const d;
};

}  // namespace Digikam

#endif  // ALBUMSETTINGS_H
