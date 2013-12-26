/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-12-20
 * Description : Settings for Showfoto
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

    QString getLastOpenedDir();
    bool getDeleteItem2Trash();
    QString getCurrentTheme();
    int getRightSideBarStyle();
    QString getApplicationStyle();

    bool getShowFormatOverThumbnail();

    bool getShowToolTip();

    bool getShowFileName();
    bool getShowFileDate();
    bool getShowFileSize();
    bool getShowFileType();
    bool getShowFileDim();

    bool getShowPhotoMake();
    bool getShowPhotoFocal();
    bool getShowPhotoExpo();
    bool getShowPhotoFlash();
    bool getShowPhotoWB();
    bool getShowPhotoDate();

    QFont getToolTipFont();

    void setLastOpenedDir(QString dir);
    void setCurrentTheme(QString theme);

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
