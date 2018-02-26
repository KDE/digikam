/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-04
 * Description : a tool to create panorama by fusion of several images.
 *               This parser is based on pto file format described here:
 *               http://hugin.sourceforge.net/docs/nona/nona.txt, and
 *               on pto files produced by Hugin's tools.
 *
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PTO_PARSER_H
#define PTO_PARSER_H

// Qt includes

#include <QString>

// Local includes

#include "ptotype.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PTOFile
{
public:

    explicit PTOFile(const QString& huginVersion);
    ~PTOFile();

    bool     openFile(const QString& path);
    PTOType* getPTO();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PTO_PARSER_H
