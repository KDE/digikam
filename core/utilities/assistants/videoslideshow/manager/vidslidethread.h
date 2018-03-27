/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef VID_SLIDE_THREAD_H
#define VID_SLIDE_THREAD_H

// Local includes

#include "vidslidesettings.h"
#include "vidslidetask.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VidSlideThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit VidSlideThread(QObject* const parent);
    ~VidSlideThread();

    void processStream(VidSlideSettings* const settings);

Q_SIGNALS:

    void signalProgress(int);
    void signalDone(bool);
    void signalMessage(const QString&, bool);
};

} // namespace Digikam

#endif // VID_SLIDE_THREAD_H
