/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2016 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "gpmpform.h"

// Qt includes

#include <QByteArray>
#include <QUrl>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTime>

//local includes

#include "wstoolutils.h"

namespace Digikam
{

GPMPForm::GPMPForm()
    : m_boundary(QByteArray("----------") + WSToolUtils::randomString(42 + 13).toLatin1())
{
}

GPMPForm::~GPMPForm()
{
}

void GPMPForm::reset()
{
    m_buffer.resize(0);
}

void GPMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";

    m_buffer.append(str);
}

bool GPMPForm::addPair(const QString& name,
                       const QString& value,
                       const QString& contentType)
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

    if (!contentType.isEmpty())
    {
        str += "Content-Type: "+ QByteArray(contentType.toLatin1());
        str += "\r\n";
        str += "Mime-version: 1.0 ";
        str += "\r\n";
    }

    str += "Content-Length: " ;
    str += content_length.toLatin1();
    str += "\r\n\r\n";
    str += value.toUtf8();
    str += "\r\n";

    m_buffer.append(str);

    return true;
}

bool GPMPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();

    if (mime.isEmpty())
    {
        // if we ourselves can't determine the mime of the local file,
        // very unlikely the remote site will be able to identify it
        return false;
    }

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray imageData = imageFile.readAll();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += name.toLatin1();
    str += "\"; ";
    str += "filename=\"";
    str += QUrl::fromLocalFile(path).fileName().toLatin1();
    str += "\"\r\n";
    str += "Content-Length: ";
    str += QString::number(imageFile.size()).toLatin1();
    str += "\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    imageFile.close();

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString GPMPForm::contentType() const
{
    return QString::fromLatin1("multipart/related; boundary=") + 
           QString::fromLatin1(m_boundary);
}

QString GPMPForm::boundary() const
{
    return QString::fromLatin1(m_boundary);
}

QByteArray GPMPForm::formData() const
{
    return m_buffer;
}

} // namespace Digikam
