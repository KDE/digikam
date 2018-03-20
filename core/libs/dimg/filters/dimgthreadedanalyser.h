/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-11-13
 * Description : threaded image analys class.
 *               this class is dedicated to run algorithm in a separated thread
 *               over an image to process analys. No image data are changed.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIMGTHREADEDANALYSER_H
#define DIMGTHREADEDANALYSER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgThreadedAnalyser : public DImgThreadedFilter
{

public:

    /** Constructs a filter without argument.
     *  You need to call setupFilter() and startFilter()
     *  to start the threaded computation.
     *  To run filter without to use multithreading, call startFilterDirectly().
     */
    explicit DImgThreadedAnalyser(QObject* const parent=0, const QString& name = QString());

    /** Constructs an image ananlyser with all arguments (ready to use).
     *  The given original image will be copied.
     *  You need to call startFilter() to start the threaded computation.
     *  To run analyser without to use multithreading, call startFilterDirectly().
     */
    DImgThreadedAnalyser(DImg* const orgImage, QObject* const parent=0,
                         const QString& name = QString());

    ~DImgThreadedAnalyser();

private:

    // NOTE: Versionning is not supported in this class

    FilterAction filterAction()
    {
        // return null object
        return FilterAction();
    };

    void readParameters(const FilterAction&)
    {
         // Do nothing.
    };

    QString filterIdentifier() const
    {
        // return null object
        return QString();
    };

    QList<int> supportedVersions() const
    {
        // return null object
        return QList<int>();
    };

    void prepareDestImage()
    {
        // No destination image is required here.
    };

    void filterImage()
    {
        startAnalyse();
    };

protected:

    /** Main image analys method. Override in subclass.
     */
    virtual void startAnalyse() = 0;
};

}  // namespace Digikam

#endif /* DIMGTHREADEDANALYSER_H */
