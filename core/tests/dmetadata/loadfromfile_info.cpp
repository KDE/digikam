/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : a command line tool to load main info from file
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QDebug>
#include <QApplication>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

int main (int argc, char** argv)
{
    QApplication app(argc, argv);

    if (argc != 2)
    {
        qDebug() << "loadfromfile_info - test to load metadata from file and print main info tags";
        qDebug() << "Usage: <file>";
        return -1;
    }

    QString filePath = QString::fromLocal8Bit(argv[1]);

    DMetadata meta;
    meta.load(filePath);

    qDebug() << "--- Comments   -------------------------------";
    qDebug() << meta.getImageComments();
    qDebug() << "--- Titles     -------------------------------";
    qDebug() << meta.getImageTitles();
    qDebug() << "--- IPTC info  -------------------------------";
    qDebug() << meta.getCreatorContactInfo();
    qDebug() << meta.getIptcCoreLocation();
    qDebug() << meta.getIptcCoreSubjects();
    qDebug() << "--- Media info -------------------------------";
    qDebug() << meta.getPhotographInformation();
    qDebug() << meta.getVideoInformation();
    qDebug() << "--- XMP info   -------------------------------";
    qDebug() << meta.getXmpKeywords();
    qDebug() << meta.getXmpSubjects();
    qDebug() << meta.getXmpSubCategories();

    return 0;
}
