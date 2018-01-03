/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image filter class.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QObject>
#include <QDateTime>
#include <QThreadPool>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DImgThreadedFilter::DImgThreadedFilter(QObject* const parent, const QString& name)
    : DynamicThread(parent)
{
    setOriginalImage(DImg());
    setFilterName(name);
    m_version      = 1;
    m_wasCancelled = false;

    initMaster();
}

DImgThreadedFilter::DImgThreadedFilter(DImg* const orgImage, QObject* const parent,
                                       const QString& name)
    : DynamicThread(parent)
{
    // remove meta data
    setOriginalImage(orgImage->copyImageData());
    setFilterName(name);
    m_version      = 1;
    m_wasCancelled = false;

    initMaster();
}

DImgThreadedFilter::DImgThreadedFilter(DImgThreadedFilter* const master, const DImg& orgImage,
                                       const DImg& destImage, int progressBegin, int progressEnd,
                                       const QString& name)
{
    setFilterName(name);
    setOriginalImage(orgImage);
    m_destImage    = destImage;
    m_version      = 1;
    m_wasCancelled = false;

    initSlave(master, progressBegin, progressEnd);
}

DImgThreadedFilter::~DImgThreadedFilter()
{
    cancelFilter();

    if (m_master)
    {
        m_master->setSlave(0);
    }
}

void DImgThreadedFilter::initSlave(DImgThreadedFilter* const master, int progressBegin, int progressEnd)
{
    m_master          = master;
    m_slave           = 0;
    m_progressBegin   = progressBegin;
    m_progressSpan    = progressEnd - progressBegin;
    m_progressCurrent = 0;

    if (m_master)
    {
        m_master->setSlave(this);
    }
}

void DImgThreadedFilter::initMaster()
{
    m_master          = 0;
    m_slave           = 0;
    m_progressBegin   = 0;
    m_progressSpan    = 100;
    m_progressCurrent = 0;
}

void DImgThreadedFilter::setupFilter(const DImg& orgImage)
{
    setOriginalImage(orgImage);
    // some filters may require that initFilter is called
    initFilter();
}

void DImgThreadedFilter::setupAndStartDirectly(const DImg& orgImage, DImgThreadedFilter* const master,
                                               int progressBegin, int progressEnd)
{
    initSlave(master, progressBegin, progressEnd);
    setupFilter(orgImage);
}

void DImgThreadedFilter::setOriginalImage(const DImg& orgImage)
{
    m_orgImage = orgImage;
}

void DImgThreadedFilter::setFilterName(const QString& name)
{
    m_name = QString(name);
}

QList<int> DImgThreadedFilter::supportedVersions() const
{
    return QList<int>() << 1;
}

void DImgThreadedFilter::setFilterVersion(int version)
{
    if (supportedVersions().contains(version))
    {
        m_version = version;
    }
}

int DImgThreadedFilter::filterVersion() const
{
    return m_version;
}

void DImgThreadedFilter::initFilter()
{
    prepareDestImage();

    if (m_master)
    {
        startFilterDirectly();
    }
}

void DImgThreadedFilter::prepareDestImage()
{
    m_destImage.reset();

    if (!m_orgImage.isNull())
    {
        m_destImage = DImg(m_orgImage.width(), m_orgImage.height(),
                           m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    }
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
        qCDebug(DIGIKAM_DIMG_LOG) << m_name << "::No valid image data !!! ...";
    }
}

void DImgThreadedFilter::startFilterDirectly()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
        emit started();

        m_wasCancelled = false;

        try
        {
            QDateTime now = QDateTime::currentDateTime();
            filterImage();
            //qCDebug(DIGIKAM_DIMG_LOG) << m_name << ":: excecution time : " << now.msecsTo(QDateTime::currentDateTime()) << " ms";
        }
        catch (std::bad_alloc& ex)
        {
            //TODO: User notification
            qCCritical(DIGIKAM_DIMG_LOG) << "Caught out-of-memory exception! Aborting operation" << ex.what();
            emit finished(false);
            return;
        }

        emit finished(!m_wasCancelled);
    }
    else  // No image data
    {
        emit finished(false);
        qCDebug(DIGIKAM_DIMG_LOG) << m_name << "::No valid image data !!! ...";
    }
}

void DImgThreadedFilter::run()
{
    startFilterDirectly();
}

void DImgThreadedFilter::cancelFilter()
{
    if (isRunning())
    {
        m_wasCancelled = true;
    }

    stop();

    if (m_slave)
    {
        m_slave->stop();
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
    else if (m_progressCurrent != progr)
    {
        emit progress(progr);
        m_progressCurrent = progr;
    }
}

void DImgThreadedFilter::setSlave(DImgThreadedFilter* const slave)
{
    m_slave = slave;
}

int DImgThreadedFilter::modulateProgress(int progress)
{
    return m_progressBegin + (int)((double)progress * (double)m_progressSpan / 100.0);
}

bool DImgThreadedFilter::parametersSuccessfullyRead() const
{
    return true;
}

QString DImgThreadedFilter::readParametersError(const FilterAction&) const
{
    return QString();
}

QList<int> DImgThreadedFilter::multithreadedSteps(int stop, int start) const
{
    uint  nbCore = QThreadPool::globalInstance()->maxThreadCount();
    float step   = ((float)stop - (float)start) / (float)nbCore;
    QList<int> vals;

    vals << start;

    for (uint i = 1 ; i < nbCore ; ++i)
        vals << vals.last() + step;

    vals << stop;

    return vals;
}

}  // namespace Digikam
