/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a kipi plugin to export images to Google-Drive web service
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
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

#include "mpform_gdrive.h"

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

#include "kipiplugins_debug.h"
#include "kputil.h"

namespace KIPIGoogleServicesPlugin
{

MPForm_GDrive::MPForm_GDrive()
    : m_boundary(KIPIPlugins::KPRandomGenerator::randomString(42 + 13).toLatin1())
{
    reset();
}

MPForm_GDrive::~MPForm_GDrive()
{
}

void MPForm_GDrive::reset()
{
    m_buffer.resize(0);
}

void MPForm_GDrive::finish()
{
    qCDebug(KIPIPLUGINS_LOG) << "in finish";
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
    qCDebug(KIPIPLUGINS_LOG) << "finish:" << m_buffer;
}

void MPForm_GDrive::addPair(const QString& name, const QString& description, const QString& path,const QString& id)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();
    qCDebug(KIPIPLUGINS_LOG) << "in add pair:" << name << " " << description << " " << path << " " << id << " " << mime;

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

bool MPForm_GDrive::addFile(const QString& path)
{
    QByteArray str;
    qCDebug(KIPIPLUGINS_LOG) << "in addfile" << path;

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

QByteArray MPForm_GDrive::formData() const
{
    return m_buffer;
}

QString MPForm_GDrive::boundary() const
{
    return QString::fromLatin1(m_boundary);
}

QString MPForm_GDrive::contentType() const
{
    return QString::fromLatin1("multipart/related;boundary=") + QString::fromLatin1(m_boundary);
}

QString MPForm_GDrive::getFileSize() const
{
    return m_file_size;
}

} // namespace KIPIGoogleServicesPlugin
