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

#ifndef MPFORM_GDRIVE_H
#define MPFORM_GDRIVE_H

// Qt includes

#include <QByteArray>
#include <QString>

namespace KIPIGoogleServicesPlugin
{

class MPForm_GDrive
{
public:

    MPForm_GDrive();
    ~MPForm_GDrive();

    void finish();
    void reset();

    void addPair(const QString& name, const QString& description, const QString& mimetype, const QString& id);
    bool addFile(const QString& path);

    QString contentType() const;
    QByteArray formData() const;
    QString boundary()    const;
    QString getFileSize() const;

private:

    QByteArray m_buffer;
    QByteArray m_boundary;
    QString    m_file_size;
};

} // namespace KIPIGoogleServicesPlugin

#endif // MPFORM_GDRIVE_H
