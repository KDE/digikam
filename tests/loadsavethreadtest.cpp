/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2012-10-23
 * @brief  a command line tool to test DImg image loader
 *
 * @author Copyright (C) 2012 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "loadsavethreadtest.moc"

// Qt includes

#include <QFileInfo>
#include <QDebug>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"

using namespace KDcrawIface;

LoadSaveThreadTest::LoadSaveThreadTest(const QString& filePath)
    : KApplication()
{
    qRegisterMetaType<LoadingDescription>("LoadingDescription");
    qRegisterMetaType<DImg>("DImg");

    m_thread = new LoadSaveThread;

    connect( m_thread, SIGNAL(signalImageLoaded(LoadingDescription, DImg)),
             this, SLOT(slotImageLoaded(LoadingDescription, DImg)) );

    connect( m_thread, SIGNAL(signalImageSaved(QString, bool)),
             this, SLOT(slotImageSaved(QString, bool)) );

    connect( m_thread, SIGNAL(signalLoadingProgress(LoadingDescription, float)),
             this, SLOT(slotLoadingProgress(LoadingDescription, float)) );

    connect( m_thread, SIGNAL(signalSavingProgress(QString, float)),
             this, SLOT(slotSavingProgress(QString, float)) );

    RawDecodingSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = RawDecodingSettings::AHD;

    LoadingDescription desc(filePath, DRawDecoding(settings));

    m_thread->load(desc);
}

void LoadSaveThreadTest::slotImageLoaded(const LoadingDescription& desc, const DImg& img)
{
    QFileInfo fi(desc.filePath);
    qDebug() << "Image " << fi.baseName() << " loaded";

    QString outFilePath(fi.baseName() + QString(".out.png"));
    DImg image = img;
    m_thread->save(image, outFilePath, "PNG");
}

void LoadSaveThreadTest::slotImageSaved(const QString& filePath, bool b)
{
    QFileInfo fi(filePath);
    qDebug() << fi.baseName() << " saved : " << (b ? "ok" : "pb");
}

void LoadSaveThreadTest::slotLoadingProgress(const LoadingDescription& desc, float p)
{
    QFileInfo fi(desc.filePath);
    qDebug() << "Loading " << fi.baseName() << " : " << p << " %";
}

void LoadSaveThreadTest::slotSavingProgress(const QString& filePath, float p)
{
    QFileInfo fi(filePath);
    qDebug() << "Saving " << fi.baseName() << " : " << p << " %";
}
