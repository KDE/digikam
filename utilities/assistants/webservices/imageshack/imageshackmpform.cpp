/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "imageshackmpform.h"

// Qt includes

#include <QFile>
#include <QUrl>
#include <QMimeDatabase>
#include <QMimeType>

// Local includes

#include "digikam_debug.h"
#include "wstoolutils.h"

namespace Digikam
{

ImageShackMPForm::ImageShackMPForm()
{
    m_boundary = WSToolUtils::randomString(42 + 13).toLatin1();
    reset();
}

ImageShackMPForm::~ImageShackMPForm()
{
}

void ImageShackMPForm::reset()
{
    m_buffer.resize(0);
    QByteArray str(contentType().toLatin1());
    str += "\r\nMIME-version: 1.0\r\n\r\n";
    m_buffer.append(str);
}

void ImageShackMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

void ImageShackMPForm::addPair(const QString& name, const QString& value)
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

bool ImageShackMPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime = ptr.name();

    if (mime.isEmpty())
        return false;

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray imageData = imageFile.readAll();

    QString file_size = QString::number(imageFile.size());
    imageFile.close();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"fileupload\"; filename=\"";
    str += QFile::encodeName(name);
    str += "\"\r\n";
    str += "Content-Length: ";
    str += file_size.toLatin1();
    str += "\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString ImageShackMPForm::contentType() const
{
    return QString::fromLatin1("multipart/form-data; boundary=") + QString::fromLatin1(m_boundary);
}

QString ImageShackMPForm::boundary() const
{
    return QString::fromLatin1(m_boundary);
}

QByteArray ImageShackMPForm::formData() const
{
    return m_buffer;
}

} // namespace Digikam
