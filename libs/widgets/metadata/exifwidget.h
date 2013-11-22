/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display Standard Exif metadata
 *
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EXIFWIDGET_H
#define EXIFWIDGET_H

// Qt includes

#include <QtCore/QString>

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit ExifWidget(QWidget* const parent, const char* name=0);
    ~ExifWidget();

    bool loadFromURL(const KUrl& url);

    QString getTagDescription(const QString& key);
    QString getTagTitle(const QString& key);

    QString getMetadataTitle();

protected Q_SLOTS:

    virtual void slotSaveMetadataToFile();

private:

    bool decodeMetadata();
    void buildView();

private:

    QStringList m_keysFilter;
};

}  // namespace Digikam

#endif /* EXIFWIDGET_H */
