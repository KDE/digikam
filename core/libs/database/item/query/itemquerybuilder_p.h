/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *               Internal containers.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ITEM_QUERY_BUILDER_P_H
#define DIGIKAM_ITEM_QUERY_BUILDER_P_H

#include "itemquerybuilder.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QDir>
#include <QMap>
#include <QRectF>
#include <QUrl>
#include <QLocale>
#include <QUrlQuery>

// Local includes

#include "metaengine.h"
#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "fieldquerybuilder.h"

namespace Digikam
{

class Q_DECL_HIDDEN RuleTypeForConversion
{
public:

    RuleTypeForConversion();

    QString             key;
    SearchXml::Relation op;
    QString             val;
};

// -------------------------------------------------------------------------

enum SKey
{
    ALBUM = 0,
    ALBUMNAME,
    ALBUMCAPTION,
    ALBUMCOLLECTION,
    TAG,
    TAGNAME,
    IMAGENAME,
    IMAGECAPTION,
    IMAGEDATE,
    KEYWORD,
    RATING
};

enum SOperator
{
    EQ = 0,
    NE,
    LT,
    GT,
    LIKE,
    NLIKE,
    LTE,
    GTE
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN RuleType
{
public:

    SKey      key;
    SOperator op;
    QString   val;
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN SubQueryBuilder
{
public:

    QString build(enum SKey key,
                  enum SOperator op,
                  const QString& passedVal,
                  QList<QVariant>* boundValues) const;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_QUERY_BUILDER_P_H
