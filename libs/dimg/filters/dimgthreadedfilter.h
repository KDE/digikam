/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image filter class.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIMGTHREADEDFILTER_H
#define DIMGTHREADEDFILTER_H

// Qt includes.

#include <QtCore/QEvent>
#include <QtCore/QThread>
#include <QtCore/QString>

// KDE includes.

#include <kapplication.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

class QObject;

namespace Digikam
{

class DIGIKAM_EXPORT DImgThreadedFilter : public QThread
{
    Q_OBJECT

public:

    /** Constructs a filter without argument.
        You need to call setOriginalImage(), setFilterName(), and startFilter() 
        to start the threaded computation.
        To run filter without to use multithreading, call startFilterDirectly().
    */
    DImgThreadedFilter(QObject *parent=0);

    /** Constructs a filter with all arguments (ready to use).
        You need to call startFilter() to start the threaded computation.
        To run filter without to use multithreading, call startFilterDirectly().
    */
    DImgThreadedFilter(DImg *orgImage, QObject *parent,
                       const QString& name = QString());

    ~DImgThreadedFilter();

    void setOriginalImage(const DImg& orgImage);
    void setFilterName(const QString& name);
    void setParent(QObject *parent);

    DImg getTargetImage()       { return m_destImage; };
    const QString &filterName() { return m_name; };

    /** Start the threaded computation */
    virtual void startFilter();
    /** Cancel the threaded computation. */
    virtual void cancelFilter();
    /** Start computation of this filter, directly in this thread. */
    virtual void startFilterDirectly();

Q_SIGNALS:

    /** This signal is emitted when image data is available and the computation has started. */
    void started();
    /** Emitted when progress info from the calculation is available */
    void progress(int progress);
    /** Emitted when the computation has completed.
        @param success True if computation finished without interruption on valid data
                       False if the thread was canceled, or no data is available.
    */
    void finished(bool success);

protected:

    /** Start filter operation before threaded method. Must be called by your constructor. */
    virtual void initFilter();

    /** List of threaded operations by filter. */
    virtual void run();

    /** Main image filter method. Override in subclass. */
    virtual void filterImage() = 0;

    /** Clean up filter data if necessary, called by stopComputation() method.
        Override in subclass.
     */
    virtual void cleanupFilter() {};

    /** Emit progress info */
    void postProgress(int progress);

protected:

    /**
      Support for chaining two filters as master and thread.

      Do not call startFilter() or startFilterDirectly() on this.
      The computation will be started from initFilter() which you must
      call from the derived class constructor.

      Constructor for slave mode:
      Constructs a new slave filter with the specified master.
      The filter will be executed in the current thread.
      orgImage and destImage will not be copied.
      progressBegin and progressEnd can indicate the progress span
      that the slave filter uses in the parent filter's progress.
      Any derived filter class that is publicly available to other filters
      should implement an additional constructor using this constructor.
      */
    DImgThreadedFilter(DImgThreadedFilter *master, const DImg &orgImage, const DImg &destImage,
                       int progressBegin=0, int progressEnd=100, const QString& name=QString());

    /** Inform the master that there is currently a slave. At destruction of the slave, call with slave=0. */
    void setSlave(DImgThreadedFilter *slave);

    /** This method modulates the progress value from the 0..100 span to the span of this slave.
        Called by postProgress if master is not null. */
    virtual int modulateProgress(int progress);

protected:

    /** Used to stop computation loop. */
    bool                m_cancel;

    /** The progress span that a slave filter uses in the parent filter's progress. */
    int                 m_progressBegin;
    int                 m_progressSpan;

    /** To post event from thread to parent. */
    QObject            *m_parent;

    /** Filter name.*/
    QString             m_name;

    /** Copy of original Image data. */
    DImg                m_orgImage;

    /** Output image data. */
    DImg                m_destImage;

    /** The current slave. Any filter might want to use another filter while processing. */
    DImgThreadedFilter *m_slave;

    /** The master of this slave filter. Progress info will be routed to this one. */
    DImgThreadedFilter *m_master;
};

}  // namespace Digikam

#endif /* DIMGTHREADEDFILTER_H */
