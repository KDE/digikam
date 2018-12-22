/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database - private containers.
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Mohamed_Anwer  <m_dot_anwer at gmx dot com>
 * Copyright (C) 2018      by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#ifndef DIGIKAM_ITEM_LISTER_P_H
#define DIGIKAM_ITEM_LISTER_P_H

#include "itemlister.h"

// C++ includes

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <limits>

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QRegExp>
#include <QDir>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "itemquerybuilder.h"
#include "dmetadata.h"
#include "haariface.h"
#include "dbenginesqlquery.h"
#include "tagscache.h"
#include "itemtagpair.h"
#include "dbjobsthread.h"
#include "dbjobinfo.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"

namespace Digikam
{

/**
 * Used by QSet
 */
inline uint qHash(const ItemListerRecord& key)
{
    return key.imageID;
}

class Q_DECL_HIDDEN ItemLister::Private
{

public:

    explicit Private()
    {
        recursive               = true;
        listOnlyAvailableImages = true;
        allowExtraValues        = false;
    }

    /*
     * The binary field for file size is only 32 bit.
     * If the value fits, we pass it. If it does not, we pass -1,
     * and the receiver shall get the full number itself
     */
    int toInt32BitSafe(const QList<QVariant>::const_iterator& it)
    {
        qlonglong v = (*it).toLongLong();

        if (v > std::numeric_limits<int>::max() || v < 0)
        {
            return -1;
        }

        return (int)v;
    }

public:

    bool recursive;
    bool listOnlyAvailableImages;
    bool allowExtraValues;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_LISTER_P_H
