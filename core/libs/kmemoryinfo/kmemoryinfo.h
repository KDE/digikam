/*
   Copyright 2010 Pino Toscano <pino at kde dot org>
   Copyright 2011 Marcel Wiesweg <marcel dot wiesweg at gmx dot de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DIGIKAM_KMEMORYINFO_H
#define DIGIKAM_KMEMORYINFO_H

// Qt includes

#include <QSharedDataPointer>

// Local includes

#include "digikam_export.h"

class QDateTime;

namespace Digikam
{

class DIGIKAM_EXPORT KMemoryInfo
{

public:

    /**
     * A detail of memory.
     */
    enum MemoryDetail
    {
        TotalRam        = 1 << 0,
        AvailableRam    = 1 << 1,
        TotalSwap       = 1 << 10,
        AvailableSwap   = 1 << 11,
        AvailableMemory = AvailableRam | AvailableSwap
    };
    Q_DECLARE_FLAGS(MemoryDetails, MemoryDetail)

    /**
     * Constructs a memory information object which has no updated information.
     * @see update()
     */
    explicit KMemoryInfo();

    /**
     * Copy constructor.
     */
    KMemoryInfo(const KMemoryInfo& info);

    /**
     * Destructor.
     */
    ~KMemoryInfo();

    KMemoryInfo& operator=(const KMemoryInfo& info);

    /**
     * Returns status if last update was successful and the data is valid.
     * -1 : not valid : unsupported platform
     *  0 : not valid : parse failure from supported platform
     *  1 : valid     : parse done with sucess from supported platform
     */
    int isValid() const;

    /**
     * Returns a KMemoryInfo object already updated to the current memory situation.
     */
    static KMemoryInfo currentInfo();

    /**
     * Request an update of the system memory.
     * @returns whether the update was successful :
     * -1 : unsupported platform
     *  0 : parse failure from supported platform
     *  1 : parse done with sucess from supported platform
     */
    int update();

    /**
     * Returns the specified memory @p details, as it was read by the last
     * update().
     * If you combine the flags, they will either be added, or, when nonsensical, the larger
     * of two values is returned (if you request TotalRam and FreeRam, you get TotalRam).
     * @returns the value of the specified detail if available, or -1 if any requested detail
     *          detail was not requested or not available in the last update()
     */
    qint64 bytes(MemoryDetails detail)     const;
    double kilobytes(MemoryDetails detail) const;
    double megabytes(MemoryDetails detail) const;

    /**
     * Returns the timestamp of the last update, or a null one if the current
     * memory information was never updated.
     */
    QDateTime lastUpdate() const;

public:

    // Defined as public due to use by external parts
    class KMemoryInfoData;

private:

    QSharedDataPointer<KMemoryInfoData> d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::KMemoryInfo::MemoryDetails)

#endif // DIGIKAM_KMEMORYINFO_H
