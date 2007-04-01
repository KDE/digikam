/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Marcel Wiesweg
 * Date   : 2007-03-22
 * Description : Database search query builder
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
 * ============================================================ */

#ifndef IMAGEQUERYBUILDER_H
#define IMAGEQUERYBUILDER_H

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageQueryBuilder
{
public:

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

    ImageQueryBuilder();
    QString buildQuery(const KURL& url) const;

protected:

    QString subQuery(enum SKey key, enum SOperator op, const QString& val) const;
    QString possibleDate(const QString& str, bool& exact) const;

    class RuleType
    {
    public:

        SKey      key;
        SOperator op;
        QString   val;
    };

    QString  m_longMonths[12];
    QString  m_shortMonths[12];
};

}

#endif

