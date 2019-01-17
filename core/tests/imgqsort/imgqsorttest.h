/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to detect image quality level
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMGQSORT_TEST_H
#define DIGIKAM_IMGQSORT_TEST_H

// Qt includes

#include <QObject>
#include <QDir>
#include <QMultiMap>
#include <QString>

class ImgQSortTest : public QObject
{
    Q_OBJECT

private:

    QDir imageDir() const;

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void testParseTestImagesForExposureDetection();
    void testParseTestImagesForNoiseDetection();

    void testParseTestImagesForBlurDetection();
    void testParseTestImagesForCompressionDetection();
};

#endif // DIGIKAM_IMGQSORT_TEST_H
