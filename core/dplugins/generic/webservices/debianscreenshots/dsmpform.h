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

#ifndef DIGIKAM_DS_MPFORM_H
#define DIGIKAM_DS_MPFORM_H

// Qt includes

#include <QString>

namespace GenericDigikamDebianScreenshotsPlugin
{

class DSMPForm
{

public:

    explicit DSMPForm();
    ~DSMPForm();

    void finish();
    void reset();

    void addPair(const QString& name, const QString& value);
    bool addFile(const QString& fileName, const QString& path, const QString& fieldName = QString());

    QString    contentType() const;
    QByteArray formData()    const;
    QString    boundary()    const;

private:

    QByteArray m_buffer;
    QByteArray m_boundary;
};

} // namespace GenericDigikamDebianScreenshotsPlugin

#endif // DIGIKAM_DS_MPFORM_H
