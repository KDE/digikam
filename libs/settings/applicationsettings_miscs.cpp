/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

// Qt includes

#include <QApplication>

// Local includes

#include "applicationsettings.h"
#include "applicationsettings_p.h"
#include "digikam_debug.h"

namespace Digikam
{

void ApplicationSettings::setCurrentTheme(const QString& theme)
{
    d->currentTheme = theme;
}

QString ApplicationSettings::getCurrentTheme() const
{
    return d->currentTheme;
}

void ApplicationSettings::setShowSplashScreen(bool val)
{
    d->showSplash = val;
}

bool ApplicationSettings::getShowSplashScreen() const
{
    return d->showSplash;
}

void ApplicationSettings::setSidebarTitleStyle(DMultiTabBar::TextStyle style)
{
    d->sidebarTitleStyle = style;
}

DMultiTabBar::TextStyle ApplicationSettings::getSidebarTitleStyle() const
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
    d->sidebarApplyDirectly = val;
}

bool ApplicationSettings::getApplySidebarChangesDirectly() const
{
    return d->sidebarApplyDirectly;
}

void ApplicationSettings::setScrollItemToCenter(bool val)
{
    d->scrollItemToCenter = val;
}

bool ApplicationSettings::getScrollItemToCenter() const
{
    return d->scrollItemToCenter;
}

void ApplicationSettings::setShowOnlyPersonTagsInPeopleSidebar(bool val)
{
    d->showOnlyPersonTagsInPeopleSidebar = val;
}

bool ApplicationSettings::showOnlyPersonTagsInPeopleSidebar() const
{
    return d->showOnlyPersonTagsInPeopleSidebar;
}

void ApplicationSettings::setStringComparisonType(ApplicationSettings::StringComparisonType val)
{
    d->stringComparisonType = val;
}

ApplicationSettings::StringComparisonType ApplicationSettings::getStringComparisonType() const
{
    return d->stringComparisonType;
}

bool ApplicationSettings::isStringTypeNatural() const
{
    return (d->stringComparisonType == ApplicationSettings::Natural);
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
    if (d->applicationStyle.compare(style, Qt::CaseInsensitive) != 0)
    {
        d->applicationStyle = style;
        qApp->setStyle(d->applicationStyle);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to widget style: " << d->applicationStyle;
    }
}

QString ApplicationSettings::getApplicationStyle() const
{
    return d->applicationStyle;
}

void ApplicationSettings::setIconTheme(const QString& theme)
{
    d->iconTheme = theme;
}

QString ApplicationSettings::getIconTheme() const
{
    return d->iconTheme;
}

void ApplicationSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool ApplicationSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void ApplicationSettings::setRatingFilterCond(int val)
{
    d->ratingFilterCond = val;
}

int ApplicationSettings::getRatingFilterCond() const
{
    return d->ratingFilterCond;
}

void ApplicationSettings::setScanAtStart(bool val)
{
    d->scanAtStart = val;
}

bool ApplicationSettings::getScanAtStart() const
{
    return d->scanAtStart;
}

void ApplicationSettings::setMinimumSimilarityBound(int val)
{
    d->minimumSimilarityBound = val;
}

int  ApplicationSettings::getMinimumSimilarityBound() const
{
    return d->minimumSimilarityBound;
}

void ApplicationSettings::setDuplicatesSearchLastMinSimilarity(int val)
{
    d->duplicatesSearchLastMinSimilarity = val;
}

int  ApplicationSettings::getDuplicatesSearchLastMinSimilarity() const
{
    return d->duplicatesSearchLastMinSimilarity;
}

void ApplicationSettings::setDuplicatesSearchLastMaxSimilarity(int val)
{
    d->duplicatesSearchLastMaxSimilarity = val;
}

int  ApplicationSettings::getDuplicatesSearchLastMaxSimilarity() const
{
    return d->duplicatesSearchLastMaxSimilarity;
}

}  // namespace Digikam
