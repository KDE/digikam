/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-12-20
 * Description : Settings for Showfoto
 *
 * Copyright (C) 2013-2014 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHOWFOTOSETTINGS_H
#define SHOWFOTOSETTINGS_H

// Qt includes

#include <QObject>
#include <QFont>
#include <QString>

namespace ShowFoto
{

class ShowfotoSettings : public QObject
{
    Q_OBJECT

public:

    static ShowfotoSettings* instance();

    void readSettings();
    void syncConfig();

    // -- Misc. Settings ---------------------------------------

    QString getLastOpenedDir() const;
    void setLastOpenedDir(const QString& dir);

    bool getDeleteItem2Trash() const;
    void setDeleteItem2Trash(bool D2t);

    QString getCurrentTheme() const;
    void setCurrentTheme(const QString& theme);

    int getRightSideBarStyle() const;
    void setRightSideBarStyle(int style);

    QString getApplicationStyle() const;
    void setApplicationStyle(const QString& style);

    QString getIconTheme() const;
    void setIconTheme(const QString& theme);

    bool getShowFormatOverThumbnail() const;
    void setShowFormatOverThumbnail(bool show);

    bool getShowCoordinates() const;
    void setShowCoordinates(bool show);

    bool getShowSplash() const;
    void setShowSplash(bool show);

    bool getNativeFileDialog() const;
    void setNativeFileDialog(bool item);

    bool getItemCenter() const;
    void setItemCenter(bool item);

    int  getSortRole() const;
    void setSortRole(int order);

    bool getReverseSort() const;
    void setReverseSort(bool reverse);

    // -- ToolTip Settings --------------------------------------

    bool getShowToolTip() const;
    void setShowToolTip(bool show);

    bool getShowFileName() const;
    void setShowFileName(bool show);

    bool getShowFileDate() const;
    void setShowFileDate(bool show);

    bool getShowFileSize() const;
    void setShowFileSize(bool show);

    bool getShowFileType() const;
    void setShowFileType(bool show);

    bool getShowFileDim() const;
    void setShowFileDim(bool show);

    bool getShowPhotoMake() const;
    void setShowPhotoMake(bool show);

    bool getShowPhotoLens() const;
    void setShowPhotoLens(bool show);

    bool getShowPhotoFocal() const;
    void setShowPhotoFocal(bool show);

    bool getShowPhotoExpo() const;
    void setShowPhotoExpo(bool show);

    bool getShowPhotoFlash() const;
    void setShowPhotoFlash(bool show);

    bool getShowPhotoWB() const;
    void setShowPhotoWB(bool show);

    bool getShowPhotoDate() const;
    void setShowPhotoDate(bool show);

    bool getShowPhotoMode() const;
    void setShowPhotoMode(bool show);

    QFont getToolTipFont() const;
    void setToolTipFont(QFont font);

private:

    ShowfotoSettings();
    ~ShowfotoSettings();

    void init();

private:

    class Private;
    Private* const d;

    friend class ShowfotoSettingsCreator;
};

} // namespace Digikam

#endif // SHOWFOTOSETTINGS_H
