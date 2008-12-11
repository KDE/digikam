/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image filter class.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimgthreadedfilter.h"
#include "dimgthreadedfilter.moc"

// Qt includes.

#include <QObject>

// KDE includes.

#include <kdebug.h>


namespace Digikam
{

DImgThreadedFilter::DImgThreadedFilter(DImg *orgImage, QObject *parent,
                                       const QString& name)
                  : QThread()
{
    // remove meta data
    m_orgImage      = orgImage->copyImageData();
    m_parent        = parent;
    m_cancel        = false;

    m_name          = name;

    m_master        = 0;
    m_slave         = 0;
    m_progressBegin = 0;
    m_progressSpan  = 100;
}

DImgThreadedFilter::DImgThreadedFilter(DImgThreadedFilter *master, const DImg &orgImage,
                                       const DImg &destImage, int progressBegin, int progressEnd,
                                       const QString& name)
{
    m_orgImage      = orgImage;
    m_destImage     = destImage;
    m_parent        = 0;
    m_cancel        = false;

    m_name          = QString(name);

    m_master        = master;
    m_slave         = 0;
    m_progressBegin = progressBegin;
    m_progressSpan  = progressEnd - progressBegin;

    m_master->setSlave(this);
}

DImgThreadedFilter::~DImgThreadedFilter()
{
    cancelFilter();
    if (m_master)
        m_master->setSlave(0);
}

void DImgThreadedFilter::initFilter()
{
    m_destImage.reset();
    m_destImage = DImg(m_orgImage.width(), m_orgImage.height(),
                       m_orgImage.sixteenBit(), m_orgImage.hasAlpha());

    if (m_master)
        startFilterDirectly();
}

void DImgThreadedFilter::startFilter()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
        start();
    }
    else  // No image data
    {
        emit finished(false);
        kDebug(50003) << m_name << "::No valid image data !!! ..." << endl;
    }
}

void DImgThreadedFilter::startFilterDirectly()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
        emit started();

        filterImage();

        emit finished(!m_cancel);
    }
    else  // No image data
    {
        emit finished(false);
        kDebug(50003) << m_name << "::No valid image data !!! ..." << endl;
    }
}

void DImgThreadedFilter::run()
{
    startFilterDirectly();
}

void DImgThreadedFilter::cancelFilter()
{
    m_cancel = true;
    if (m_slave)
    {
        m_slave->m_cancel = true;
        // do not wait on slave, it is not running in its own separate thread!
        //m_slave->cleanupFilter();
    }
    wait();
    cleanupFilter();
}

void DImgThreadedFilter::postProgress(int progr)
{
    if (m_master)
    {
        progr = modulateProgress(progr);
        m_master->postProgress(progr);
    }
    else
    {
        emit progress(progr);
    }
}

void DImgThreadedFilter::setSlave(DImgThreadedFilter *slave)
{
    m_slave = slave;
}

int DImgThreadedFilter::modulateProgress(int progress)
{
    return m_progressBegin + (int)((double)progress * (double)m_progressSpan / 100.0);
}

}  // namespace Digikam
