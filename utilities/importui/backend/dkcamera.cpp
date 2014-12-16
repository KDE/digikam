/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : abstract camera interface class
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dkcamera.moc"

// Local includes

#include "applicationsettings.h"
#include "dmetadata.h"

namespace Digikam
{

DKCamera::DKCamera(const QString& title, const QString& model, const QString& port, const QString& path) : QObject()
{
    m_title                       = title;
    m_model                       = model;
    m_port                        = port;
    m_path                        = path;
    m_thumbnailSupport            = false;
    m_deleteSupport               = false;
    m_uploadSupport               = false;
    m_mkDirSupport                = false;
    m_delDirSupport               = false;
    m_captureImageSupport         = false;
    m_captureImagePreviewSupport  = false;

    ApplicationSettings* const settings = ApplicationSettings::instance();
    m_imageFilter                 = settings->getImageFileFilter();
    m_movieFilter                 = settings->getMovieFileFilter();
    m_audioFilter                 = settings->getAudioFileFilter();
    m_rawFilter                   = settings->getRawFileFilter();
    m_imageFilter                 = m_imageFilter.toLower();
    m_movieFilter                 = m_movieFilter.toLower();
    m_audioFilter                 = m_audioFilter.toLower();
    m_rawFilter                   = m_rawFilter.toLower();
}

DKCamera::~DKCamera()
{
}

QString DKCamera::title() const
{
    return m_title;
}

QString DKCamera::model() const
{
    return m_model;
}

QString DKCamera::port() const
{
    return m_port;
}

QString DKCamera::path() const
{
    return m_path;
}

QString DKCamera::uuid() const
{
    return m_uuid;
}

bool DKCamera::thumbnailSupport() const
{
    return m_thumbnailSupport;
}

bool DKCamera::deleteSupport() const
{
    return m_deleteSupport;
}

bool DKCamera::uploadSupport() const
{
    return m_uploadSupport;
}

bool DKCamera::mkDirSupport() const
{
    return m_mkDirSupport;
}

bool DKCamera::delDirSupport() const
{
    return m_delDirSupport;
}

bool DKCamera::captureImageSupport() const
{
    return m_captureImageSupport;
}

bool DKCamera::captureImagePreviewSupport() const
{
    return m_captureImagePreviewSupport;
}

QString DKCamera::mimeType(const QString& fileext) const
{
    if (fileext.isEmpty())
    {
        return QString();
    }

    QString ext = fileext;
    QString mime;

    // Massage known variations of known mimetypes into KDE specific ones
    if (ext == "jpg" || ext == "jpe")
    {
        ext = "jpeg";
    }
    else if (ext == "tif")
    {
        ext = "tiff";
    }

    if (m_rawFilter.contains(ext))
    {
        mime = QString("image/x-raw");
    }
    else if (m_imageFilter.contains(ext))
    {
        mime = QString("image/") + ext;
    }
    else if (m_movieFilter.contains(ext))
    {
        mime = QString("video/") + ext;
    }
    else if (m_audioFilter.contains(ext))
    {
        mime = QString("audio/") + ext;
    }

    return mime;
}

void DKCamera::fillItemInfoFromMetadata(CamItemInfo& info, const DMetadata& meta) const
{
    QSize dims     = meta.getImageDimensions();
    info.ctime     = meta.getImageDateTime();
    //NOTE: see bug #246401 to sort based on milliseconds for items  taken quickly.
    info.ctime.setTime(info.ctime.time().addMSecs(meta.getMSecsInfo()));
    info.width     = dims.width();
    info.height    = dims.height();
    info.photoInfo = meta.getPhotographInformation();
}

}  // namespace Digikam
