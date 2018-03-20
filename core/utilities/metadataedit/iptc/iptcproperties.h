/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-10
 * Description : IPTC workflow status properties settings page.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IPTC_PROPERTIES_H
#define IPTC_PROPERTIES_H

// Qt includes

#include <QWidget>
#include <QByteArray>

namespace Digikam
{

class IPTCProperties : public QWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QString> CountryCodeMap;

public:

    explicit IPTCProperties(QWidget* const parent);
    ~IPTCProperties();

    void applyMetadata(QByteArray& iptcData);
    void readMetadata(QByteArray& iptcData);

    static CountryCodeMap countryCodeMap();

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSetTodayReleased();
    void slotSetTodayExpired();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IPTC_PROPERTIES_H
