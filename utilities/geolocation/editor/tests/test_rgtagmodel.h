/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-21
 * @brief  Test for RG tag model.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TEST_RGTAGMODEL_H
#define TEST_RGTAGMODEL_H

// Qt includes

#include <QtTest/QtTest>

// KDE includes

// local includes

class TestRGTagModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNoOp();
    void testModelEmpty();

    void testModel1();
    void testModel2();
    void testModel3();
    void testModelSpacerTags();
};

#endif /* TEST_RGTAGMODEL_H */

