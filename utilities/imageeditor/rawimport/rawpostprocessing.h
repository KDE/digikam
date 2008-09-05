/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-13-08
 * Description : Raw post processing corrections.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RAWPOSTPROCESSING_H
#define RAWPOSTPROCESSING_H

// Digikam includes.

#include "digikam_export.h"

// Local includes.

#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT RawPostProcessing : public DImgThreadedFilter
{

public:

    RawPostProcessing(DImg *orgImage, QObject *parent=0, const DRawDecoding& settings=DRawDecoding());

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    RawPostProcessing(DImgThreadedFilter *parentFilter, const DImg &orgImage, const DImg &destImage,
                      int progressBegin=0, int progressEnd=100, const DRawDecoding& settings=DRawDecoding());

    ~RawPostProcessing(){};

private:

    virtual void filterImage();
    void rawPostProcessing();

private:

    DRawDecoding m_customRawSettings;
};

}  // NameSpace Digikam

#endif /* RAWPOSTPROCESSING_H */
