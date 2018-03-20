/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : a db option key
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DBKEYSCOLLECTION_H
#define DBKEYSCOLLECTION_H

// Qt includes

#include <QMap>

// Local includes

#include "parsesettings.h"

namespace Digikam
{

typedef QMap<QString, QString> DbKeyIdsMap;

/**
 * @brief A class for managing / grouping database keys.
 *
 * This class manages database keys and provides methods to get the
 * appropriate value from the database.
 */
class DbKeysCollection
{
public:

    /**
     * Default constructor.
     *
     * @param name  collection name
     */
    explicit DbKeysCollection(const QString& name);
    virtual ~DbKeysCollection();

    /**
     * Get a value from the database.
     * @param key           the key representing the value in the database
     * @param settings      the %ParseSettings object holding all relevant information
     *                      about the image.
     * @return  the value of the given database key
     */
    QString getValue(const QString& key, ParseSettings& settings);

    /**
     * Get all IDs associated with this key collection.
     * @return a map of all associated ids and their description
     */
    DbKeyIdsMap ids() const;

    /**
     * Get the name of the %DbKeysCollection
     * @return the name of the collection
     */
    QString collectionName() const;

protected:

    /**
     * Abstract method for retrieving the value from the database for the given key.
     *
     * This method has to be implemented by all child classes. It is called by the
     * getValue() method.
     *
     * @param key           the key representing the value in the database
     * @param settings      the %ParseSettings object holding all relevant information
     *                      about the image.
     *
     * @return  the value of the given database key
     * @see DbKeysCollection::getValue()
     */
    virtual QString getDbValue(const QString& key, ParseSettings& settings) = 0;

    /**
     * Add an ID to the key collection.
     *
     * @param id            the id of the database key
     * @param description   a short description of the database key
     */
    void addId(const QString& id, const QString& description);

private:

    DbKeysCollection(const DbKeysCollection&);
    DbKeysCollection& operator=(const DbKeysCollection&);

private:

    DbKeyIdsMap idsMap;
    QString     name;
};

} // namespace Digikam

#endif /* DBKEYSCOLLECTION_H */
