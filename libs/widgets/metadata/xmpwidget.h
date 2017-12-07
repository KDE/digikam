/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-19
 * Description : A widget to display XMP metadata
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef XMPWIDGET_H
#define XMPWIDGET_H

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT XmpWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit XmpWidget(QWidget* const parent, const QString& name=QString());
    ~XmpWidget();

    bool loadFromURL(const QUrl& url);

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

} // namespace Digikam

#endif // IPTCWIDGET_H
