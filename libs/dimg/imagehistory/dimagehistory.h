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
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "filteraction.h"
#include "historyimageid.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImageHistory
{
public:

    class Entry
    {
    public:

        FilterAction          action;
        QList<HistoryImageId> referredImages;
    };

public:

    DImageHistory();
    DImageHistory(const DImageHistory& other);
    ~DImageHistory();

    DImageHistory& operator=(const DImageHistory& other);

    bool isNull() const;
    bool isEmpty() const;
    bool hasFilters() const;

    int size() const;

    bool operator==(const DImageHistory& other) const;

    /**
     * Appends a new filter action to the history.
     */
    DImageHistory& operator<<(const FilterAction& action);
    /**
     * Appends a new referred image, representing the current state
     * of the history.
     * If you add an id of type Current, adjustReferredImages() will be called.
     */
    DImageHistory& operator<<(const HistoryImageId& imageId);

    /// Removes the last entry from the history
    void removeLast();
    /// Remove all referredImages, leaving the entries list untouched
    void clearReferredImages();

    bool operator<(const DImageHistory& other);
    bool operator>(const DImageHistory& other);

    QList<DImageHistory::Entry>& entries();
    const QList<DImageHistory::Entry>& entries() const;

    /// Gets all actions which are not null
    QList<FilterAction> allActions() const;
    int actionCount() const;
    /// Returns if there is any not-null action
    bool hasActions() const;

    const FilterAction& action(int i) const;
    QList<HistoryImageId>& referredImages(int i);
    const QList<HistoryImageId>& referredImages(int i) const;

    QList<HistoryImageId> allReferredImages() const;
    HistoryImageId currentReferredImage() const;
    HistoryImageId originalReferredImage() const;
    QList<HistoryImageId> referredImagesOfType(HistoryImageId::Type type) const;
    bool hasReferredImages() const;
    bool hasReferredImageOfType(HistoryImageId::Type type) const;
    bool hasCurrentReferredImage() const;
    bool hasOriginalReferredImage() const;

    void setOriginalFileName(const QString& fileName);
    QString originalFileName();

    void setOriginalFilePath(const QString& filePath);
    QString originalFilePath();

    QString originalUUID() const;

    QString toXml() const;
    static DImageHistory fromXml(const QString& xml);

    /**
     * Adjusts the type of a Current HistoryImageId:
     * If it is the first entry, it becomes Original,
     * if it is in an intermediate entry, it becomes Intermediate,
     * if in the last entry, it stays current.
     */
    void adjustReferredImages();

    /// Changes the UUID of the current (last added current) referred image
    void adjustCurrentUuid(const QString& uuid);

    /**
     * Remove file path entries pointing to the given absolute path
     * from any referred images. This is useful when said file
     * is about to be overwritten.
     * All other HistoryImageId fields remain unchanged, no HistoryImageId is removed.
     * path: directory path, without filename.
     */
    void purgePathFromReferredImages(const QString& path, const QString& fileName);

public:

    // Set as public there because of ImageHistoryPrivSharedNull
    class ImageHistoryPriv;

private:

    QSharedDataPointer<ImageHistoryPriv> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DImageHistory)

#endif // DIMAGEHISTORY_H
