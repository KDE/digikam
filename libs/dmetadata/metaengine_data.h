/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-11-15
 * @brief  Exiv2 library interface
 *
 * @author Copyright (C) 2009-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2009-2012 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef META_ENGINE_DATA_H
#define META_ENGINE_DATA_H

// Qt includes

#include <QSharedDataPointer>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetaEngineData
{
public:

    MetaEngineData();
    MetaEngineData(const MetaEngineData&);
    ~MetaEngineData();

    MetaEngineData& operator=(const MetaEngineData&);

public:

    // Declared as public due to use in MetaEngine::Private class
    class Private;

private:

    QSharedDataPointer<Private> d;

    friend class MetaEngine;
};

} // namespace Digikam

#endif /* META_ENGINE_DATA_H */
