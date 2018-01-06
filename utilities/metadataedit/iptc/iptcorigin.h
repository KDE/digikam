/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-13
 * Description : IPTC origin settings page.
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

#ifndef IPTC_ORIGIN_H
#define IPTC_ORIGIN_H

// Qt includes

#include <QDateTime>
#include <QWidget>
#include <QByteArray>

namespace Digikam
{

class IPTCOrigin : public QWidget
{
    Q_OBJECT

public:

    explicit IPTCOrigin(QWidget* const parent);
    ~IPTCOrigin();

    void applyMetadata(QByteArray& exifData, QByteArray& iptcData);
    void readMetadata(QByteArray& iptcData);

    bool syncEXIFDateIsChecked() const;

    void setCheckedSyncEXIFDate(bool c);

    QDateTime getIPTCCreationDate() const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSetTodayCreated();
    void slotSetTodayDigitalized();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IPTC_ORIGIN_H
