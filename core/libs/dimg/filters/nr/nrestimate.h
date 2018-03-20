/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Wavelets YCrCb Noise Reduction settings estimation by image content analys.
 *
 * Copyright (C) 2012-2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
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

#ifndef NRESTIMATE_H
#define NRESTIMATE_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "nrfilter.h"
#include "dimg.h"
#include "dimgthreadedanalyser.h"

namespace Digikam
{

class DIGIKAM_EXPORT NREstimate : public DImgThreadedAnalyser
{
public:

    /** Standard constructor with image container to parse
     */
    explicit NREstimate(DImg* const img, QObject* const parent=0);
    ~NREstimate();

    /** Perform estimate noise.
     */
    void startAnalyse();

    /** Return all Wavelets noise reduction settings computed by image analys.
     */
    NRContainer settings() const;

    /** To set image path where log files will be created to host computation algorithm results, for hacking purpose.
     *  If path is not set, no log files will be created.
     */
    void setLogFilesPath(const QString& path);

private:

    /** Internal method dedicated to convert DImg pixels from integer values to float values.
     *  These ones will by used internally by estimateNoise through OpenCV API.
     */
    void readImage() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* NRESTIMATE_H */
