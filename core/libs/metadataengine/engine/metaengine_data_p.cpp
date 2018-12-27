/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Internal private data container.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "metaengine_data_p.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

void MetaEngineData::Private::clear()
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        imageComments.clear();
        exifMetadata.clear();
        iptcMetadata.clear();
#ifdef _XMP_SUPPORT_
        xmpMetadata.clear();
#endif
    }
    catch( Exiv2::Error& e )
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot clear data container using Exiv2"
                                           << " (Error #" << e.code() << ": "
                                           << std::string(e.what()).c_str();
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

}

} // namespace Digikam
