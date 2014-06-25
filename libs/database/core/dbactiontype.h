/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-20
 * Description : DBActionType which wrappes other data types
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
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

#ifndef DBACTIONTYPE_H
#define DBACTIONTYPE_H

// Qt includes

#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
/**
 * The DBActionType is used by the databasecorebackend
 * to wrap another data object within an sql statement
 * and controls whether it should be used as field entry or as value
 * (prepared to an sql statement with positional binding).
 */
class DIGIKAM_EXPORT DBActionType
{
public:

    DBActionType();
    DBActionType(const DBActionType& actionType);
    ~DBActionType();

    static DBActionType value(const QVariant& value);
    static DBActionType fieldEntry(const QVariant& actionValue);

    /**
     * Returns the wrapped object.
     */
    QVariant getActionValue();

    /**
     * Sets the wrapped object.
     */
    void setActionValue(const QVariant& actionValue);

    /**
     * Returns true, if the entry is an value element.
     * Returns false, if the entry should be used as field entry.
     */
    bool isValue() const;

    /**
     * Sets the DBAction mode:
     * true, if the entry is an value element.
     * false, if the entry should be used as field entry.
     */
    void setValue(bool isValue);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DBActionType)

#endif // DBACTIONTYPE_H
