/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-12
 * Description : Convenience object for grouping face database operations
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FACE_DATABASE_OPERATION_GROUP_H
#define FACE_DATABASE_OPERATION_GROUP_H

namespace Digikam
{

class FaceDbAccess;

/**
 * When you intend to execute a number of write operations to the database,
 * group them while holding a FaceDbOperationGroup.
 * For some database systems (SQLite), keeping a transaction across write operations
 * occurring in short time results in enormous speedup (800x).
 * For system that do not need this optimization, this class is a no-op.
 */
class FaceDbOperationGroup
{
public:

    /**
     * Retrieve a FaceDbAccess object each time when constructing and destructing.
     */
    FaceDbOperationGroup();

    /**
     * Use an existing FaceDbAccess object, which must live as long as this object exists.
     */
    FaceDbOperationGroup(FaceDbAccess* const access);

    ~FaceDbOperationGroup();

    /**
     * This will - if a transaction is held - commit the transaction and acquire a new one.
     * This may improve concurrent access.
     */
    void lift();

    void setMaximumTime(int msecs);

    /**
     * Resets to 0 the time used by allowLift()
     */
    void resetTime();

    /**
     * Allows to lift(). The transaction will be lifted if the time set by setMaximumTime()
     * has expired.
     */
    void allowLift();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FACE_DATABASE_OPERATION_GROUP_H
