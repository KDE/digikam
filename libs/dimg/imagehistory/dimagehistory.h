/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class for manipulating modifications changeset for non-destruct. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QHash>
#include <QList>
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

        /**
         * A DImageHistory is a list of entries.
         *
         * Each entry has one action. The action can be null,
         * but it shall be null only if it is the action of the first
         * entry, with the "Original" as referred image,
         * representing the action of digitization.
         *
         * There can be zero, one or any number
         * of referred images per entry.
         * A referred image is a file in the state after the action is applied.
         */
        FilterAction          action;
        QList<HistoryImageId> referredImages;
    };

public:

    DImageHistory();
    DImageHistory(const DImageHistory& other);
    ~DImageHistory();

    DImageHistory& operator=(const DImageHistory& other);

    /**
     * A history is null if it is constructed with the default constructor
     */
    bool isNull() const;

    /**
     * A history is considered empty if there are no entries.
     */
    bool isEmpty() const;

    /**
     * A history is a valid history (telling something about the past),
     * if the history is not empty, and there is at least one
     * referred image other than the "Current" entry,
     * or there is a valid action.
     */
    bool isValid() const;

    /// Returns the number of entries
    int size() const;

    bool operator==(const DImageHistory& other) const;
    bool operator!=(const DImageHistory& other) const { return !operator==(other); }
    bool operator<(const DImageHistory& other) const;
    bool operator>(const DImageHistory& other) const;

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

    void appendReferredImage(const HistoryImageId& id);
    void insertReferredImage(int entryIndex, const HistoryImageId& id);

    /// Removes the last entry from the history
    void removeLast();

    /**
     * Access entries.
     * There are size() entries.
     */
    QList<DImageHistory::Entry>&       entries();
    const QList<DImageHistory::Entry>& entries() const;
    Entry&                             operator[](int i);
    const Entry&                       operator[](int i) const;

    /**
     * Access actions.
     *
     * There is one action per entry,
     * but the action may be null.
     */
    /// Returns if there is any non-null action
    bool hasActions() const;
    bool hasFilters() const { return hasActions(); }

    /// Returns the number of non-null actions
    int actionCount() const;

    /// Gets all actions which are not null
    QList<FilterAction> allActions() const;
    const FilterAction& action(int i) const;

    /**
     * Access referred images
     */
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

    /**
     * Edit referred images
     */

    /// Remove all referredImages, leaving the entries list untouched
    void clearReferredImages();

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

    /**
     * Change file path entries of the current referred image
     */
    void moveCurrentReferredImage(const QString& newPath, const QString& newFileName);

    /**
     * Serialize to and from XML.
     *
     * Note: The "Current" entry is skipped when writing to XML,
     * so make sure the file into the metadata of which you write the XML,
     * is the file marked as "Current" in this history.
     */
    QString toXml() const;
    static DImageHistory fromXml(const QString& xml);

public:

    // Set as public there because of PrivateSharedNull
    class Private;

private:

    QSharedDataPointer<Private> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DImageHistory)

#endif // DIMAGEHISTORY_H
