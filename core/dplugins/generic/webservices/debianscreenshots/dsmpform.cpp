/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-03
 * Description : a tool to export images to Debian Screenshots site
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2010      by Pau Garcia i Quiles <pgquiles at elpauer dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dsmpform.h"

// Qt includes

#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>
#include <QString>

// Local includes

#include "digikam_debug.h"
#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericDebianScreenshotsPlugin
{

DSMPForm::DSMPForm()
{
    m_boundary = WSToolUtils::randomString(42 + 13).toLatin1();
    reset();
}

DSMPForm::~DSMPForm()
{
}

void DSMPForm::reset()
{
    m_buffer.resize(0);
    QByteArray str(contentType().toLatin1());
    str += "\r\n";
    str += "MIME-version: 1.0";
    str += "\r\n\r\n";
    m_buffer.append(str);
}

void DSMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

void DSMPForm::addPair(const QString& name, const QString& value)
{
    QByteArray str;
    QString content_length = QString::number(value.length());

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toLatin1();
        str += "\"\r\n";
    }

    str += "\r\n";
    str += value.toUtf8();
    str += "\r\n";

    m_buffer.append(str);
}

bool DSMPForm::addFile(const QString& fileName, const QString& path, const QString& fieldName)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();

    if (mime.isEmpty())
        return false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "mime = " << mime.toLatin1(); 

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray imageData = imageFile.readAll();

    imageFile.close();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; ";

    if (!fieldName.isEmpty())
    {
        str += "name=\"" + QByteArray(fieldName.toLatin1()) + "\"; ";
    }

    str += "filename=\"";
    str += QFile::encodeName(fileName);
    str += "\"\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString DSMPForm::contentType() const
{
    return QLatin1String("Content-Type: multipart/form-data; boundary=") + QLatin1String(m_boundary);
}

QString DSMPForm::boundary() const
{
    return QLatin1String(m_boundary);
}

QByteArray DSMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericDebianScreenshotsPlugin
