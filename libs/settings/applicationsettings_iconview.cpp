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

// Local includes

#include "applicationsettings.h"
#include "applicationsettings_p.h"

namespace Digikam
{

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

void ApplicationSettings::setImageSeparationMode(int mode)
{
    d->imageSeparationMode = mode;
}

int ApplicationSettings::getImageSeparationMode() const
{
    return d->imageSeparationMode;
}

void ApplicationSettings::setImageSeparationSortOrder(int order)
{
    d->imageSeparationSortOrder = order;
}

int ApplicationSettings::getImageSeparationSortOrder() const
{
    return d->imageSeparationSortOrder;
}

void ApplicationSettings::setItemLeftClickAction(const ItemLeftClickAction action)
{
    d->itemLeftClickAction = action;
}

ApplicationSettings::ItemLeftClickAction ApplicationSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

void ApplicationSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int ApplicationSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void ApplicationSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont ApplicationSettings::getIconViewFont() const
{
    return d->iconviewFont;
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

void ApplicationSettings::setIconShowFullscreen(bool val)
{
   d->iconShowFullscreen = val;
}

bool ApplicationSettings::getIconShowFullscreen() const
{
   return d->iconShowFullscreen;
}

void ApplicationSettings::setPreviewSettings(const PreviewSettings& settings)
{
    d->previewSettings = settings;
}

PreviewSettings ApplicationSettings::getPreviewSettings() const
{
    return d->previewSettings;
}

void ApplicationSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ApplicationSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

}  // namespace Digikam
