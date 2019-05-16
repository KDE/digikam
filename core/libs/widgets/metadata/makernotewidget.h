/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display non standard Exif metadata
 *               used by camera makers
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_MAKER_NOTE_WIDGET_H
#define DIGIKAM_MAKER_NOTE_WIDGET_H

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MakerNoteWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit MakerNoteWidget(QWidget* const parent, const QString& name=QString());
    ~MakerNoteWidget();

    bool loadFromURL(const QUrl& url) override;

    QString getTagDescription(const QString& key) override;
    QString getTagTitle(const QString& key) override;

    QString getMetadataTitle() override;

protected Q_SLOTS:

    virtual void slotSaveMetadataToFile() override;

private:

    bool decodeMetadata() override;
    void buildView() override;

private:

    QStringList m_keysFilter;
};

} // namespace Digikam

#endif // DIGIKAM_MAKER_NOTE_WIDGET_H
