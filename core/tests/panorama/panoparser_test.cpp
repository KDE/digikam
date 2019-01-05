/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-24
 * Description : a test program for the pto parser
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

// Local includes

#include "ptofile.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    if (argc != 2)
        return 1;

    QString ptoFile(QString::fromLocal8Bit(argv[1]));

    PTOFile file(QLatin1String("2014.0"));
    file.openFile(ptoFile);

    PTOType* const ptoData = file.getPTO();
    delete ptoData;

    return 0;
}
