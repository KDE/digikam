/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : Apply tag paths to item with DMetadata.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_APPLY_TAGS_TEST_H
#define DIGIKAM_APPLY_TAGS_TEST_H

// Qt includes

#include <QObject>
#include <QString>

class ApplyTagsTest : public QObject
{
    Q_OBJECT

private:

    void applyTags(const QString& file, const QStringList& tags);

private Q_SLOTS:

    void initTestCase();
    void testApplyTagsToMetadata();
    void cleanupTestCase();
};

#endif // DIGIKAM_APPLY_TAGS_TEST_H
