/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-12-21
 * Description : abstract camera interface class
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qdeepcopy.h>

// Local includes.

#include "albumsettings.h"
#include "dkcamera.h"

namespace Digikam
{

DKCamera::DKCamera(const QString& model, const QString& port, const QString& path)
{
    m_model = model;
    m_port  = port;
    m_path  = path;

    AlbumSettings* settings = AlbumSettings::instance();
    m_imageFilter = QDeepCopy<QString>(settings->getImageFileFilter());
    m_movieFilter = QDeepCopy<QString>(settings->getMovieFileFilter());
    m_audioFilter = QDeepCopy<QString>(settings->getAudioFileFilter());
    m_rawFilter   = QDeepCopy<QString>(settings->getRawFileFilter());

    m_imageFilter = m_imageFilter.lower();
    m_movieFilter = m_movieFilter.lower();
    m_audioFilter = m_audioFilter.lower();
    m_rawFilter   = m_rawFilter.lower();
}

DKCamera::~DKCamera()
{
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

QString DKCamera::mimeType(const QString& fileext) const
{
    QString ext = fileext;
    
    // massage known variations of known mimetypes into kde specific ones
    if (ext == "jpg")
        ext = "jpeg";
    else if (ext == "tif")
        ext = "tiff";
    
    if (m_imageFilter.contains(ext))
    {
        return QString("image/") + ext;
    }
    else if (m_movieFilter.contains(ext))
    {
        return QString("video/") + ext;
    }
    else if (m_audioFilter.contains(ext))
    {
        return QString("audio/") + ext;
    }
    else if (m_rawFilter.contains(ext))
    {
        return QString("image/x-raw");
    }
    else
    {
        return QString();
    }
}

}  // namespace Digikam
