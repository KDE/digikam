/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-12-20
 * Description : Settings for Showfoto
 *
 * Copyright (C) 2013-2014 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
 * Copyright (C) 2013-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotosettings.moc"

// Qt includes

#include <QStyle>

// KDE includes

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kapplication.h>

// Local includes

#include "setupmisc.h"
#include "thememanager.h"

namespace ShowFoto
{

class ShowfotoSettings::Private
{

public:

    Private() :
        deleteItem2Trash(true),
        showFormatOverThumbnail(false),
        showCoordinates(false),
        showSplash(true),
        reverseSort(false),
        showToolTip(true),
        showFileName(true),
        showFileDate(false),
        showFileSize(false),
        showFileType(false),
        showFileDim(true),
        showPhotoMake(true),
        showPhotoFocal(true),
        showPhotoExpo(true),
        showPhotoFlash(false),
        showPhotoWB(false),
        showPhotoDate(true),
        showPhotoMode(false),
        rightSideBarStyle(0),
        sortOrder(0)
    {
    }

    static const QString configGroupDefault;

    static const QString configLastOpenedDir;
    static const QString configDeleteItem2Trash;
    static const QString configCurrentTheme;
    static const QString configRightSideBarStyle;
    static const QString configApplicationStyle;
    static const QString configShowFormatOverThumbnail;
    static const QString configShowCoordinates;
    static const QString configShowSplash;
    static const QString configSortOrder;
    static const QString configReverseSort;

    static const QString configShowToolTip;

    static const QString configShowFileName;
    static const QString configShowFileDate;
    static const QString configShowFileSize;
    static const QString configShowFileType;
    static const QString configShowFileDim;

    static const QString configShowPhotoMake;
    static const QString configShowPhotoFocal;
    static const QString configShowPhotoExpo;
    static const QString configShowPhotoFlash;
    static const QString configShowPhotoWB;
    static const QString configShowPhotoDate;
    static const QString configShowPhotoMode;

    static const QString configToolTipsFont;

    bool                 deleteItem2Trash;
    bool                 showFormatOverThumbnail;
    bool                 showCoordinates;
    bool                 showSplash;
    bool                 reverseSort;

    bool                 showToolTip;

    bool                 showFileName;
    bool                 showFileDate;
    bool                 showFileSize;
    bool                 showFileType;
    bool                 showFileDim;

    bool                 showPhotoMake;
    bool                 showPhotoFocal;
    bool                 showPhotoExpo;
    bool                 showPhotoFlash;
    bool                 showPhotoWB;
    bool                 showPhotoDate;
    bool                 showPhotoMode;

    int                  rightSideBarStyle;
    int                  sortOrder;

    QFont                toolTipsFont;

    QString              lastOpenedDir;
    QString              theme;
    QString              applicationStyle;

    KSharedConfigPtr     config;
    KConfigGroup         group;
};

//Configuration Group
const QString ShowfotoSettings::Private::configGroupDefault("ImageViewer Settings");

//Misc. & Showfoto Generals Settings
const QString ShowfotoSettings::Private::configLastOpenedDir("Last Opened Directory");
const QString ShowfotoSettings::Private::configDeleteItem2Trash("DeleteItem2Trash");
const QString ShowfotoSettings::Private::configCurrentTheme("Theme");
const QString ShowfotoSettings::Private::configRightSideBarStyle("Sidebar Title Style");
const QString ShowfotoSettings::Private::configApplicationStyle("Application Style");
const QString ShowfotoSettings::Private::configShowFormatOverThumbnail("ShowMimeOverImage");
const QString ShowfotoSettings::Private::configShowCoordinates("Show Coordinates");
const QString ShowfotoSettings::Private::configShowSplash("ShowSplash");
const QString ShowfotoSettings::Private::configSortOrder("SortOrder");
const QString ShowfotoSettings::Private::configReverseSort("ReverseSort");

//Tool Tip Enable/Disable
const QString ShowfotoSettings::Private::configShowToolTip("Show ToolTips");

//Tool Tip File Properties
const QString ShowfotoSettings::Private::configShowFileName("ToolTips Show File Name");
const QString ShowfotoSettings::Private::configShowFileDate("ToolTips Show File Date");
const QString ShowfotoSettings::Private::configShowFileSize("ToolTips Show File Size");
const QString ShowfotoSettings::Private::configShowFileType("ToolTips Show Image Type");
const QString ShowfotoSettings::Private::configShowFileDim("ToolTips Show Image Dim");

//Tool Tip Photograph Info
const QString ShowfotoSettings::Private::configShowPhotoMake("ToolTips Show Photo Make");
const QString ShowfotoSettings::Private::configShowPhotoFocal("ToolTips Show Photo Focal");
const QString ShowfotoSettings::Private::configShowPhotoExpo("ToolTips Show Photo Expo");
const QString ShowfotoSettings::Private::configShowPhotoFlash("ToolTips Show Photo Flash");
const QString ShowfotoSettings::Private::configShowPhotoWB("ToolTips Show Photo WB");
const QString ShowfotoSettings::Private::configShowPhotoDate("ToolTips Show Photo Date");
const QString ShowfotoSettings::Private::configShowPhotoMode("ToolTips Show Photo Mode");

//Tool Tips Font
const QString ShowfotoSettings::Private::configToolTipsFont("ToolTips Font");

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
    d->group  = d->config->group(d->configGroupDefault);
    init();
    readSettings();
}

ShowfotoSettings::~ShowfotoSettings()
{
    delete d;
}

void ShowfotoSettings::init()
{
    d->rightSideBarStyle       = 0;
    d->sortOrder               = 0;
    d->deleteItem2Trash        = true;
    d->showSplash              = true;
    d->reverseSort             = false;

    d->showFormatOverThumbnail = false;
    d->showCoordinates         = false;

    d->showToolTip             = true;

    d->showFileName            = true;
    d->showFileDate            = false;
    d->showFileSize            = false;
    d->showFileType            = false;
    d->showFileDim             = true;

    d->showPhotoMake           = true;
    d->showPhotoFocal          = true;
    d->showPhotoExpo           = true;
    d->showPhotoFlash          = false;
    d->showPhotoWB             = false;
    d->showPhotoDate           = true;
    d->showPhotoMode           = true;
}

void ShowfotoSettings::readSettings()
{
    KConfigGroup group         = d->group;

    d->lastOpenedDir           = group.readEntry(d->configLastOpenedDir, QString());
    d->deleteItem2Trash        = group.readEntry(d->configDeleteItem2Trash, true);
    d->theme                   = group.readEntry(d->configCurrentTheme, Digikam::ThemeManager::instance()->defaultThemeName());
    d->rightSideBarStyle       = group.readEntry(d->configRightSideBarStyle, 0);
    d->applicationStyle        = group.readEntry(d->configApplicationStyle, kapp->style()->objectName());
    d->showSplash              = group.readEntry(d->configShowSplash, true);
    d->sortOrder               = group.readEntry(d->configSortOrder, 0);
    d->reverseSort             = group.readEntry(d->configReverseSort, false);
    d->showFormatOverThumbnail = group.readEntry(d->configShowFormatOverThumbnail, false);
    d->showCoordinates         = group.readEntry(d->configShowCoordinates, false);

    d->showToolTip             = group.readEntry(d->configShowToolTip, true);

    d->showFileName            = group.readEntry(d->configShowFileName, true);
    d->showFileDate            = group.readEntry(d->configShowFileDate, false);
    d->showFileSize            = group.readEntry(d->configShowFileSize, false);
    d->showFileType            = group.readEntry(d->configShowFileType, false);
    d->showFileDim             = group.readEntry(d->configShowFileDim,  true);

    d->showPhotoMake           = group.readEntry(d->configShowPhotoMake,  true);
    d->showPhotoFocal          = group.readEntry(d->configShowPhotoFocal, true);
    d->showPhotoExpo           = group.readEntry(d->configShowPhotoExpo,  true);
    d->showPhotoFlash          = group.readEntry(d->configShowPhotoFlash, false);
    d->showPhotoWB             = group.readEntry(d->configShowPhotoWB,    false);
    d->showPhotoDate           = group.readEntry(d->configShowPhotoDate,  true);
    d->showPhotoMode           = group.readEntry(d->configShowPhotoMode,  true);

    d->toolTipsFont            = group.readEntry(d->configToolTipsFont,KGlobalSettings::generalFont());
}

QString ShowfotoSettings::getLastOpenedDir() const
{
    return d->lastOpenedDir;
}

bool ShowfotoSettings::getDeleteItem2Trash() const
{
    return d->deleteItem2Trash;
}

QString ShowfotoSettings::getCurrentTheme() const
{
    return d->theme;
}

int ShowfotoSettings::getRightSideBarStyle() const
{
    return d->rightSideBarStyle;
}

bool ShowfotoSettings::getShowFormatOverThumbnail() const
{
    return d->showFormatOverThumbnail;
}

bool ShowfotoSettings::getShowCoordinates() const
{
    return d->showCoordinates;
}

QString ShowfotoSettings::getApplicationStyle() const
{
    return d->applicationStyle;
}

bool ShowfotoSettings::getShowSplash() const
{
    return d->showSplash;
}

int ShowfotoSettings::getSortRole() const
{    
    return d->sortOrder;
}

bool ShowfotoSettings::getReverseSort() const
{
    return d->reverseSort;
}

bool ShowfotoSettings::getShowToolTip() const
{
    return d->showToolTip;
}

bool ShowfotoSettings::getShowFileName() const
{
    return d->showFileName;
}

bool ShowfotoSettings::getShowFileDate() const
{
    return d->showFileDate;
}

bool ShowfotoSettings::getShowFileSize() const
{
    return d->showFileSize;
}

bool ShowfotoSettings::getShowFileType() const
{
    return d->showFileType;
}

bool ShowfotoSettings::getShowFileDim() const
{
    return d->showFileDim;
}

bool ShowfotoSettings::getShowPhotoMake() const
{
    return d->showPhotoMake;
}

bool ShowfotoSettings::getShowPhotoFocal() const
{
    return d->showPhotoFocal;
}

bool ShowfotoSettings::getShowPhotoExpo() const
{
    return d->showPhotoExpo;
}

bool ShowfotoSettings::getShowPhotoFlash() const
{
    return d->showPhotoFlash;
}

bool ShowfotoSettings::getShowPhotoWB() const
{
    return d->showPhotoWB;
}

bool ShowfotoSettings::getShowPhotoDate() const
{
    return d->showPhotoDate;
}

bool ShowfotoSettings::getShowPhotoMode() const
{
    return d->showPhotoMode;
}

QFont ShowfotoSettings::getToolTipFont() const
{
    return d->toolTipsFont;
}

void ShowfotoSettings::setShowToolTip(bool show)
{
    d->group.writeEntry(d->configShowToolTip, show);
}

void ShowfotoSettings::setShowFileName(bool show)
{
    d->group.writeEntry(d->configShowFileName, show);
}

void ShowfotoSettings::setShowFileDate(bool show)
{
    d->group.writeEntry(d->configShowFileDate, show);
}

void ShowfotoSettings::setShowFileSize(bool show)
{
    d->group.writeEntry(d->configShowFileSize, show);
}

void ShowfotoSettings::setShowFileType(bool show)
{
    d->group.writeEntry(d->configShowFileType, show);
}

void ShowfotoSettings::setShowFileDim(bool show)
{
    d->group.writeEntry(d->configShowFileDim, show);
}

void ShowfotoSettings::setShowPhotoMake(bool show)
{
    d->group.writeEntry(d->configShowPhotoMake, show);
}

void ShowfotoSettings::setShowPhotoFocal(bool show)
{
    d->group.writeEntry(d->configShowPhotoFocal, show);
}

void ShowfotoSettings::setShowPhotoExpo(bool show)
{
    d->group.writeEntry(d->configShowPhotoExpo, show);
}

void ShowfotoSettings::setShowPhotoFlash(bool show)
{
    d->group.writeEntry(d->configShowPhotoFlash, show);
}

void ShowfotoSettings::setShowPhotoWB(bool show)
{
    d->group.writeEntry(d->configShowPhotoWB, show);
}

void ShowfotoSettings::setShowPhotoDate(bool show)
{
    d->group.writeEntry(d->configShowPhotoDate, show);
}

void ShowfotoSettings::setShowPhotoMode(bool show)
{
    d->group.writeEntry(d->configShowPhotoMode, show);
}

void ShowfotoSettings::setToolTipFont(QFont font)
{
    d->group.writeEntry(d->configToolTipsFont, font);
}

void ShowfotoSettings::setLastOpenedDir(const QString& dir)
{
    d->group.writeEntry(d->configLastOpenedDir,dir);
}

void ShowfotoSettings::setDeleteItem2Trash(bool D2t)
{
    d->group.writeEntry(d->configDeleteItem2Trash, D2t);
}

void ShowfotoSettings::setCurrentTheme(const QString& theme)
{    
    d->group.writeEntry(d->configCurrentTheme, theme);
}

void ShowfotoSettings::setRightSideBarStyle(int style)
{
    d->group.writeEntry(d->configRightSideBarStyle, style);
}

void ShowfotoSettings::setApplicationStyle(const QString& style)
{
    d->group.writeEntry(d->configApplicationStyle, style);
}

void ShowfotoSettings::setShowFormatOverThumbnail(bool show)
{
    d->group.writeEntry(d->configShowFormatOverThumbnail, show);
}

void ShowfotoSettings::setShowCoordinates(bool show)
{
    d->group.writeEntry(d->configShowCoordinates, show);
}

void ShowfotoSettings::setShowSplash(bool show)
{
    d->group.writeEntry(d->configShowSplash, show);
}

void ShowfotoSettings::setSortRole(int order)
{
    d->group.writeEntry(d->configSortOrder, order);   
}

void ShowfotoSettings::setReverseSort(bool reverse)
{
    d->group.writeEntry(d->configReverseSort, reverse);
}

void ShowfotoSettings::syncConfig()
{
    d->config->sync();
}

} // namespace Showfoto
