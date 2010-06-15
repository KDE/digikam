/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class for manipulating modifications changeset for non-destruct. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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


#ifndef DIMAGEHISTORY_H
#define DIMAGEHISTORY_H

//Qt includes

#include <QtCore/QXmlStreamWriter>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QHash>
#include <QtCore/QList>

#include <QSharedDataPointer>
#include <QString>
#include <QVariant>

// KDE includes

// Local includes

#include "digikam_export.h"
#include "filteraction.h"
#include "historyimageid.h"

namespace Digikam
{
class ImageHistoryPriv;
  
class DIGIKAM_EXPORT DImageHistory
{
public:

    class Entry
    {
    public:

        FilterAction action;
        HistoryImageId referredImages;
    };

    DImageHistory();
    DImageHistory(const DImageHistory& other);
    ~DImageHistory();

    DImageHistory &operator=(const DImageHistory& other);

    bool isNull() const;
    bool isEmpty() const;

    int size() const;

    DImageHistory &operator<<(const FilterAction& action);
    DImageHistory &operator<<(const HistoryImageId& imageId);
    DImageHistory &operator<<(const Entry& entry);

    QList<DImageHistory::Entry> &entries();
    const QList<DImageHistory::Entry> &entries() const;

    const FilterAction &action(int i) const;
    const HistoryImageId &referredImages(int i) const;

    QString toXml() const;
    static DImageHistory fromXml(const QString& xml);

private:

    QSharedDataPointer<ImageHistoryPriv> d;  
};

}

#endif // DIMAGEHISTORY_H
