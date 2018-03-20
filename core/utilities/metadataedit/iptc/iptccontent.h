/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : IPTC caption settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IPTC_CONTENT_H
#define IPTC_CONTENT_H

// Qt includes

#include <QWidget>
#include <QByteArray>

namespace Digikam
{

class IPTCContent : public QWidget
{
    Q_OBJECT

public:

    explicit IPTCContent(QWidget* const parent);
    ~IPTCContent();

    void applyMetadata(QByteArray& exifData, QByteArray& iptcData);
    void readMetadata(QByteArray& iptcData);

    bool syncJFIFCommentIsChecked() const;
    bool syncEXIFCommentIsChecked() const;

    void setCheckedSyncJFIFComment(bool c);
    void setCheckedSyncEXIFComment(bool c);

    QString getIPTCCaption() const;

Q_SIGNALS:

    void signalModified();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IPTC_CONTENT_H
