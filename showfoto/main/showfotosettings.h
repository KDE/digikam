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

    QString getLastOpenedDir() const;
    bool getDeleteItem2Trash() const;
    QString getCurrentTheme() const;
    int getRightSideBarStyle() const;
    QString getApplicationStyle() const;

    bool getShowFormatOverThumbnail() const;

    bool getShowToolTip() const;

    bool getShowFileName() const;
    bool getShowFileDate() const;
    bool getShowFileSize() const;
    bool getShowFileType() const;
    bool getShowFileDim()  const;

    bool getShowPhotoMake()  const;
    bool getShowPhotoFocal() const;
    bool getShowPhotoExpo()  const;
    bool getShowPhotoFlash() const;
    bool getShowPhotoWB()    const;
    bool getShowPhotoDate()  const;
    bool getShowPhotoMode()  const;

    QFont getToolTipFont() const;

    void setShowToolTip(bool show);

    void setShowFileName(bool show);
    void setShowFileDate(bool show);
    void setShowFileSize(bool show);
    void setShowFileType(bool show);
    void setShowFileDim(bool show);

    void setShowPhotoMake(bool show)  ;
    void setShowPhotoFocal(bool show) ;
    void setShowPhotoExpo(bool show)  ;
    void setShowPhotoFlash(bool show) ;
    void setShowPhotoWB(bool show)    ;
    void setShowPhotoDate(bool show)  ;
    void setShowPhotoMode(bool show)  ;

    void setToolTipFont(QFont font);

    void setLastOpenedDir(const QString& dir);
    void setCurrentTheme(const QString& theme);

    void syncConfig();

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
