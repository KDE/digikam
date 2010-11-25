/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class that holds list of applied filters to image
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

#ifndef FILTERACTION_H
#define FILTERACTION_H

// Qt includes

#include <QVariant>
#include <QHash>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT FilterAction
{
public:

    enum Category
    {
        // Do not change existing values, they are written to XML in files!
        /** When given the set of stored parameters and the original data,
         *  an identical result will be produced. */
        ReproducibleFilter = 0,
        /**
         *  The operation is documented and a number of parameters may be known,
         *  but the identical result cannot be reproduced.
         *  It may be possible to produce a sufficiently similar result.
         */
        ComplexFilter = 1,
        /** The source images are known, a textual description may be added,
         *  but there is no way to automatically replay
         */
        DocumentedHistory = 2,

        CategoryFirst = ReproducibleFilter,
        CategoryLast  = DocumentedHistory
    };

public:

    FilterAction();
    FilterAction(const QString& identifier, int version, Category category = ReproducibleFilter);

    bool isNull() const;

    bool operator==(const FilterAction& other) const;

    Category category() const;

    /** Returns a technical identifier for the filter used to produce this action.
     *  Can include a namespace. Example: digikam:charcoal */
    QString  identifier() const;

    /** Returns the version (>= 1) of the filter used to produce this action.
     *  When a filter / plugin is found by the identifier, it can decide
     *  by the version if it supports this action and which parameters it expects.
     */
    int      version() const;

    /** Returns a description / comment for this action.
     *  In the case of DocumentedHistory, this may be the most useful value.
     */
    QString  description() const;
    void setDescription(const QString& description);

    QString displayableName() const;
    void setDisplayableName(const QString& displayableName);

    bool hasParameter(const QString& key) const;

    const QVariant parameter(const QString& key) const;
    QVariant& parameter(const QString& key);

    template <typename T>
    T parameter(const QString& key) const
    {
        return parameter(key).value<T>();
    }

    /// Sets parameter, removing all other values for the same key
    void setParameter(const QString& key, const QVariant& value);
    /// Adds a parameter, possibly keeping existing parameters with the same key.
    void addParameter(const QString& key, const QVariant& value);
    /// Removes all parameters for key
    void removeParameters(const QString& key);
    /// Clear all parameters
    void clearParameters();
    /// Adds a set of parameters
    void addSetOfParameters(const QHash<QString, QVariant>& values);

    const QHash<QString,QVariant>& parameters() const;
    QHash<QString, QVariant>& parameters();

    bool isEmpty() const;

protected:

    Category                 m_category;
    QString                  m_identifier;
    int                      m_version;
    QString                  m_description;
    QString                  m_displayableName;
    QHash<QString, QVariant> m_params;
};

} // namespace Digikam

#endif // FILTERACTION_H
