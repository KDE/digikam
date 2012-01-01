/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-13
 * Description : a test for applying FilterActions
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimagefilteractiontest.moc"

// Qt includes

#include <QPixmap>
#include <QLabel>
#include <QDialog>
#include <QHBoxLayout>

// KDE includes

#include <qtest_kde.h>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "dimg.h"
#include "dimagehistory.h"
#include "drawdecoding.h"
#include "filteractionfilter.h"

using namespace Digikam;

QTEST_KDEMAIN(DImageFilterActionTest, /*No*/GUI)
//QTEST_MAIN(DImageFilterActionTest)

void DImageFilterActionTest::testDRawDecoding()
{
    DRawDecoding params;

    params.rawPrm.sixteenBitsImage = true;
    params.rawPrm.autoBrightness  = true;
    params.rawPrm.whiteBalance = KDcrawIface::RawDecodingSettings::AERA;
    params.rawPrm.RGBInterpolate4Colors = true;
    params.rawPrm.RAWQuality = KDcrawIface::RawDecodingSettings::AMAZE;
    params.rawPrm.NRType = KDcrawIface::RawDecodingSettings::WAVELETSNR;
    params.rawPrm.outputColorSpace = KDcrawIface::RawDecodingSettings::ADOBERGB;

    FilterAction action;
    params.writeToFilterAction(action);

    qDebug() << action.parameters();

    DRawDecoding params2 = DRawDecoding::fromFilterAction(action);
    QVERIFY(params == params2);
}

void DImageFilterActionTest::testActions()
{
    QStringList files = QDir(imagePath()).entryList(QDir::Files);
    files.removeOne(originalImage());

    DImg original(imagePath() + originalImage());
    QVERIFY(!original.isNull());

    foreach(const QString& fileName, files)
    {
        DImg ref(imagePath() + fileName);
        QVERIFY(!ref.isNull());
        DImageHistory history = ref.getImageHistory();

        FilterActionFilter filter;
        filter.setFilterActions(history.allActions());
        QVERIFY(filter.isReproducible() || filter.isComplexAction());

        filter.setupFilter(original.copy());
        filter.startFilterDirectly();
        qDebug() << filter.filterActions().size();

        DImg img = filter.getTargetImage();

        QVERIFY(ref.size() == img.size());

        bool isEqual = true;
        DImg diff(ref.width(), ref.height(), ref.sixteenBit());
        diff.fill(DColor(Qt::black));

        for (uint x=0; x<ref.width(); ++x)
        {
            for (uint y=0; y<ref.height(); ++y)
            {
                DColor cref = ref.getPixelColor(x,y);
                DColor cres = img.getPixelColor(x,y);

                if (cref.red() != cres.red() || cref.green() != cres.green() || cref.blue() != cres.blue())
                {
                    //qDebug() << x << y;
                    diff.setPixelColor(x,y, DColor(Qt::white));
                    isEqual = false;
                }
            }
        }

        if (!isEqual)
        {
            showDiff(original, ref, img, diff);
        }

        QVERIFY(isEqual);
    }
}

void DImageFilterActionTest::showDiff(const Digikam::DImg& orig, const Digikam::DImg& ref,
                                      const Digikam::DImg& result, const DImg& diff)
{
    QDialog d;
    QLabel l1, l2, l3, l4;
    l1.setPixmap(orig.convertToPixmap());
    l2.setPixmap(ref.convertToPixmap());
    l3.setPixmap(result.convertToPixmap());
    l4.setPixmap(diff.convertToPixmap());
    QHBoxLayout layout(&d);
    layout.addWidget(&l1);
    layout.addWidget(&l2);
    layout.addWidget(&l3);
    layout.addWidget(&l4);
    d.setLayout(&layout);
    d.exec();
}

void DImageFilterActionTest::initTestCase()
{
    // initialize kexiv2 before doing any multitasking
    KExiv2Iface::KExiv2::initializeExiv2();
}

void DImageFilterActionTest::cleanupTestCase()
{
    // clean up the kexiv2 memory:
    KExiv2Iface::KExiv2::cleanupExiv2();
}

QString DImageFilterActionTest::originalImage()
{
    // picture taken by me, downscaled to 100x66 and metadata stripped off
    return "DSC00636.JPG";
}

QString DImageFilterActionTest::imagePath()
{
    return QString(KDESRCDIR) + "filteractiontestimages/";
}

