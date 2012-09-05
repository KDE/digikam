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

#ifndef IMPORTSETTINGS_H
#define IMPORTSETTINGS_H

// Qt includes

#include <QObject>
#include <QFont>

namespace Digikam
{

class ImportSettings : public QObject
{
    Q_OBJECT

public:

    enum ItemLeftClickAction
    {
        ShowPreview = 0,
        StartEditor
    };

Q_SIGNALS:

    void setupChanged();

public:

    static ImportSettings* instance();

    void readSettings();
    void saveSettings();

    void emitSetupChanged();

    bool showToolTipsIsValid() const;

    void setShowThumbbar(bool val);
    bool getShowThumbbar() const;

    void setPreviewLoadFullImageSize(bool val);
    bool getPreviewLoadFullImageSize() const;

    void setPreviewItemsWhileDownload(bool val);
    bool getPreviewItemsWhileDownload() const;

    void setPreviewShowIcons(bool val);
    bool getPreviewShowIcons() const;

    void setImageSortOrder(int order);
    int getImageSortOrder() const;

    void setImageSorting(int sorting);
    int getImageSorting() const;

    void setImageGroupMode(int mode);
    int getImageGroupMode() const;

    void setItemLeftClickAction(const ItemLeftClickAction action);
    ItemLeftClickAction getItemLeftClickAction() const;

    void setDefaultIconSize(int val);
    int getDefaultIconSize() const;

    void setIconViewFont(const QFont& font);
    QFont getIconViewFont() const;

    void setIconShowName(bool val);
    bool getIconShowName() const;

    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowTitle(bool val);
    bool getIconShowTitle() const;

    void setIconShowTags(bool val);
    bool getIconShowTags() const;

    void setIconShowModDate(bool val);
    bool getIconShowModDate() const;

    void setIconShowRating(bool val);
    bool getIconShowRating() const;

    void setIconShowImageFormat(bool val);
    bool getIconShowImageFormat() const;

    //TODO: void setIconShowOverlays(bool val);
    //TODO: bool getIconShowOverlays() const;

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

    void setToolTipsShowPhotoFocal(bool val);
    bool getToolTipsShowPhotoFocal() const;

    void setToolTipsShowPhotoExpo(bool val);
    bool getToolTipsShowPhotoExpo() const;

    void setToolTipsShowPhotoFlash(bool val);
    bool getToolTipsShowPhotoFlash() const;

    void setToolTipsShowPhotoWB(bool val);
    bool getToolTipsShowPhotoWB() const;

    //TODO: void setToolTipsShowTags(bool val);
    //TODO: bool getToolTipsShowTags() const;

    //TODO: void setToolTipsShowLabelRating(bool val);
    //TODO: bool getToolTipsShowLabelRating() const;

private:

    ImportSettings();
    ~ImportSettings();

    void init();

private:

    class Private;
    Private* const d;

    friend class ImportSettingsCreator;
};

} // namespace Digikam

#endif // IMPORTSETTINGS_H
