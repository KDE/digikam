/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
 *
 * Copyright (C) 2010 by Marcel Wiesweg <user dot wiesweg at gmx dot de>
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

#ifndef DIMGABSTRACTHISTORYTEST_H
#define DIMGABSTRACTHISTORYTEST_H

// Qt includes

#include <QEventLoop>

// Local includes

#include "dimagehistory.h"
#include "editorcore.h"

using namespace Digikam;

QDebug operator<<(QDebug dbg, const HistoryImageId& id);

class DImgAbstractHistoryTest : public QObject
{
    Q_OBJECT

public:

    HistoryImageId id1() const;
    HistoryImageId id2() const;
    HistoryImageId id3() const;
    HistoryImageId id4() const;

    FilterAction action1() const;
    FilterAction action2() const;

    DImageHistory history1() const;

    void applyFilters1();
    void applyFilters2();
    void applyFilters3();
    void applyFilters4();

    QString tempFileName(const QString& purpose) const;
    QString tempFilePath(const QString& purpose) const;

    static QString imagePath();

public Q_SLOTS:

    virtual void slotImageLoaded(const QString&, bool);
    virtual void slotImageSaved(const QString&, bool);

protected:

    void initBaseTestCase();
    void cleanupBaseTestCase();

    QEventLoop m_loop;
    Digikam::EditorCore* m_im;
    QString m_tempFile;
};

#endif // DIMGABSTRACTHISTORYTEST_H
