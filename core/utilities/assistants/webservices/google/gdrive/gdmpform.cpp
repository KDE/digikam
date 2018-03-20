/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gdmpform.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QUrl>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTime>

// local includes

#include "digikam_debug.h"
#include "wstoolutils.h"

namespace Digikam
{

GDMPForm::GDMPForm()
    : m_boundary(WSToolUtils::randomString(42 + 13).toLatin1())
{
    reset();
}

GDMPForm::~GDMPForm()
{
}

void GDMPForm::reset()
{
    m_buffer.resize(0);
}

void GDMPForm::finish()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in finish";
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "finish:" << m_buffer;
}

void GDMPForm::addPair(const QString& name,
                       const QString& description,
                       const QString& path,
                       const QString& id)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in add pair:"
                                 << name << " "
                                 << description
                                 << " " << path
                                 << " " << id
                                 << " " << mime;

    // Generate JSON
    QJsonObject photoInfo;
    photoInfo.insert(QString::fromLatin1("title"),       QJsonValue(name));
    photoInfo.insert(QString::fromLatin1("description"), QJsonValue(description));
    photoInfo.insert(QString::fromLatin1("mimeType"),    QJsonValue(mime));

    QVariantMap parentId;
    parentId.insert(QString::fromLatin1("id"), id);
    QVariantList parents;
    parents << parentId;
    photoInfo.insert(QString::fromLatin1("parents"),     QJsonValue(QJsonArray::fromVariantList(parents)));

    QJsonDocument doc(photoInfo);
    QByteArray json = doc.toJson();

    // Append to the multipart
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Type:application/json; charset=UTF-8\r\n\r\n";
    str += json;
    str += "\r\n";
    m_buffer.append(str);
}

bool GDMPForm::addFile(const QString& path)
{
    QByteArray str;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in addfile" << path;

    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray imageData = imageFile.readAll();
    m_file_size          = QString::number(imageFile.size());

    imageFile.close();

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QByteArray GDMPForm::formData() const
{
    return m_buffer;
}

QString GDMPForm::boundary() const
{
    return QString::fromLatin1(m_boundary);
}

QString GDMPForm::contentType() const
{
    return QString::fromLatin1("multipart/related;boundary=") + 
           QString::fromLatin1(m_boundary);
}

QString GDMPForm::getFileSize() const
{
    return m_file_size;
}

} // namespace Digikam
