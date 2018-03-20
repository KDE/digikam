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

void ApplicationSettings::setToolTipsFont(const QFont& font)
{
    d->toolTipsFont = font;
}

QFont ApplicationSettings::getToolTipsFont() const
{
    return d->toolTipsFont;
}

void ApplicationSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool ApplicationSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void ApplicationSettings::setToolTipsShowFileName(bool val)
{
    d->tooltipShowFileName = val;
}

bool ApplicationSettings::getToolTipsShowFileName() const
{
    return d->tooltipShowFileName;
}

void ApplicationSettings::setToolTipsShowFileDate(bool val)
{
    d->tooltipShowFileDate = val;
}

bool ApplicationSettings::getToolTipsShowFileDate() const
{
    return d->tooltipShowFileDate;
}

void ApplicationSettings::setToolTipsShowFileSize(bool val)
{
    d->tooltipShowFileSize = val;
}

bool ApplicationSettings::getToolTipsShowFileSize() const
{
    return d->tooltipShowFileSize;
}

void ApplicationSettings::setToolTipsShowImageType(bool val)
{
    d->tooltipShowImageType = val;
}

bool ApplicationSettings::getToolTipsShowImageType() const
{
    return d->tooltipShowImageType;
}

void ApplicationSettings::setToolTipsShowImageDim(bool val)
{
    d->tooltipShowImageDim = val;
}

bool ApplicationSettings::getToolTipsShowImageDim() const
{
    return d->tooltipShowImageDim;
}

void ApplicationSettings::setToolTipsShowImageAR(bool val)
{
    d->tooltipShowImageAR = val;
}

bool ApplicationSettings::getToolTipsShowImageAR() const
{
    return d->tooltipShowImageAR;
}

void ApplicationSettings::setToolTipsShowPhotoMake(bool val)
{
    d->tooltipShowPhotoMake = val;
}

bool ApplicationSettings::getToolTipsShowPhotoMake() const
{
    return d->tooltipShowPhotoMake;
}

void ApplicationSettings::setToolTipsShowPhotoLens(bool val)
{
    d->tooltipShowPhotoLens = val;
}

bool ApplicationSettings::getToolTipsShowPhotoLens() const
{
    return d->tooltipShowPhotoLens;
}

void ApplicationSettings::setToolTipsShowPhotoDate(bool val)
{
    d->tooltipShowPhotoDate = val;
}

bool ApplicationSettings::getToolTipsShowPhotoDate() const
{
    return d->tooltipShowPhotoDate;
}

void ApplicationSettings::setToolTipsShowPhotoFocal(bool val)
{
    d->tooltipShowPhotoFocal = val;
}

bool ApplicationSettings::getToolTipsShowPhotoFocal() const
{
    return d->tooltipShowPhotoFocal;
}

void ApplicationSettings::setToolTipsShowPhotoExpo(bool val)
{
    d->tooltipShowPhotoExpo = val;
}

bool ApplicationSettings::getToolTipsShowPhotoExpo() const
{
    return d->tooltipShowPhotoExpo;
}

void ApplicationSettings::setToolTipsShowPhotoMode(bool val)
{
    d->tooltipShowPhotoMode = val;
}

bool ApplicationSettings::getToolTipsShowPhotoMode() const
{
    return d->tooltipShowPhotoMode;
}

void ApplicationSettings::setToolTipsShowPhotoFlash(bool val)
{
    d->tooltipShowPhotoFlash = val;
}

bool ApplicationSettings::getToolTipsShowPhotoFlash() const
{
    return d->tooltipShowPhotoFlash;
}

void ApplicationSettings::setToolTipsShowPhotoWB(bool val)
{
    d->tooltipShowPhotoWb = val;
}

bool ApplicationSettings::getToolTipsShowPhotoWB() const
{
    return d->tooltipShowPhotoWb;
}

void ApplicationSettings::setToolTipsShowAlbumName(bool val)
{
    d->tooltipShowAlbumName = val;
}

bool ApplicationSettings::getToolTipsShowAlbumName() const
{
    return d->tooltipShowAlbumName;
}

void ApplicationSettings::setToolTipsShowTitles(bool val)
{
    d->tooltipShowTitles = val;
}

bool ApplicationSettings::getToolTipsShowTitles() const
{
    return d->tooltipShowTitles;
}

void ApplicationSettings::setToolTipsShowComments(bool val)
{
    d->tooltipShowComments = val;
}

bool ApplicationSettings::getToolTipsShowComments() const
{
    return d->tooltipShowComments;
}

void ApplicationSettings::setToolTipsShowTags(bool val)
{
    d->tooltipShowTags = val;
}

bool ApplicationSettings::getToolTipsShowTags() const
{
    return d->tooltipShowTags;
}

void ApplicationSettings::setToolTipsShowLabelRating(bool val)
{
    d->tooltipShowLabelRating = val;
}

bool ApplicationSettings::getToolTipsShowLabelRating() const
{
    return d->tooltipShowLabelRating;
}

void ApplicationSettings::setShowAlbumToolTips(bool val)
{
    d->showAlbumToolTips = val;
}

bool ApplicationSettings::getShowAlbumToolTips() const
{
    return d->showAlbumToolTips;
}

void ApplicationSettings::setToolTipsShowAlbumTitle(bool val)
{
    d->tooltipShowAlbumTitle = val;
}

bool ApplicationSettings::getToolTipsShowAlbumTitle() const
{
    return d->tooltipShowAlbumTitle;
}

void ApplicationSettings::setToolTipsShowAlbumDate(bool val)
{
    d->tooltipShowAlbumDate = val;
}

bool ApplicationSettings::getToolTipsShowAlbumDate() const
{
    return d->tooltipShowAlbumDate;
}

void ApplicationSettings::setToolTipsShowAlbumCollection(bool val)
{
    d->tooltipShowAlbumCollection = val;
}

bool ApplicationSettings::getToolTipsShowAlbumCollection() const
{
    return d->tooltipShowAlbumCollection;
}

void ApplicationSettings::setToolTipsShowAlbumCategory(bool val)
{
    d->tooltipShowAlbumCategory = val;
}

bool ApplicationSettings::getToolTipsShowAlbumCategory() const
{
    return d->tooltipShowAlbumCategory;
}

void ApplicationSettings::setToolTipsShowAlbumCaption(bool val)
{
    d->tooltipShowAlbumCaption = val;
}

bool ApplicationSettings::getToolTipsShowAlbumCaption() const
{
    return d->tooltipShowAlbumCaption;
}

void ApplicationSettings::setToolTipsShowAlbumPreview(bool val)
{
    d->tooltipShowAlbumPreview = val;
}

bool ApplicationSettings::getToolTipsShowAlbumPreview() const
{
    return d->tooltipShowAlbumPreview;
}

void ApplicationSettings::setToolTipsShowVideoAspectRatio(bool val)
{
    d->tooltipShowVideoAspectRatio = val;
}

bool ApplicationSettings::getToolTipsShowVideoAspectRatio() const
{
    return d->tooltipShowVideoAspectRatio;
}

void ApplicationSettings::setToolTipsShowVideoAudioBitRate(bool val)
{
    d->tooltipShowVideoAudioBitRate = val;
}

bool ApplicationSettings::getToolTipsShowVideoAudioBitRate() const
{
    return d->tooltipShowVideoAudioBitRate;
}

void ApplicationSettings::setToolTipsShowVideoAudioChannelType(bool val)
{
    d->tooltipShowVideoAudioChannelType = val;
}

bool ApplicationSettings::getToolTipsShowVideoAudioChannelType() const
{
    return d->tooltipShowVideoAudioChannelType;
}

void ApplicationSettings::setToolTipsShowVideoAudioCodec(bool val)
{
    d->tooltipShowVideoAudioCodec = val;
}

bool ApplicationSettings::getToolTipsShowVideoAudioCodec() const
{
    return d->tooltipShowVideoAudioCodec;
}

void ApplicationSettings::setToolTipsShowVideoDuration(bool val)
{
    d->tooltipShowVideoDuration = val;
}

bool ApplicationSettings::getToolTipsShowVideoDuration() const
{
    return d->tooltipShowVideoDuration;
}

void ApplicationSettings::setToolTipsShowVideoFrameRate(bool val)
{
    d->tooltipShowVideoFrameRate = val;
}

bool ApplicationSettings::getToolTipsShowVideoFrameRate() const
{
    return d->tooltipShowVideoFrameRate;
}

void ApplicationSettings::setToolTipsShowVideoVideoCodec(bool val)
{
    d->tooltipShowVideoVideoCodec = val;
}

bool ApplicationSettings::getToolTipsShowVideoVideoCodec() const
{
    return d->tooltipShowVideoVideoCodec;
}

bool ApplicationSettings::showToolTipsIsValid() const
{
    if (d->showToolTips)
    {
        if (d->tooltipShowFileName   ||
            d->tooltipShowFileDate   ||
            d->tooltipShowFileSize   ||
            d->tooltipShowImageType  ||
            d->tooltipShowImageDim   ||
            d->tooltipShowImageAR    ||
            d->tooltipShowPhotoMake  ||
            d->tooltipShowPhotoLens  ||
            d->tooltipShowPhotoDate  ||
            d->tooltipShowPhotoFocal ||
            d->tooltipShowPhotoExpo  ||
            d->tooltipShowPhotoMode  ||
            d->tooltipShowPhotoFlash ||
            d->tooltipShowPhotoWb    ||
            d->tooltipShowAlbumName  ||
            d->tooltipShowComments   ||
            d->tooltipShowTags       ||
            d->tooltipShowLabelRating)
        {
            return true;
        }
    }

    return false;
}

bool ApplicationSettings::showAlbumToolTipsIsValid() const
{
    if (d->showAlbumToolTips)
    {
        if (
            d->tooltipShowAlbumTitle      ||
            d->tooltipShowAlbumDate       ||
            d->tooltipShowAlbumCollection ||
            d->tooltipShowAlbumCaption    ||
            d->tooltipShowAlbumCategory
        )
        {
            return true;
        }
    }

    return false;
}

}  // namespace Digikam
