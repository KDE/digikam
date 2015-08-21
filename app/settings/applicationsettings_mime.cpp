/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "applicationsettings.h"
#include "applicationsettings_p.h"
#include "databaseaccess.h"
#include "albumdb.h"

using namespace KDcrawIface;

namespace Digikam
{

void ApplicationSettings::addToImageFileFilter(const QString& extensions)
{
    DatabaseAccess().db()->addToUserImageFilterSettings(extensions);
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
    DatabaseAccess().db()->getFilterSettings(&imageFilter, &videoFilter, &audioFilter);
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

}  // namespace Digikam
