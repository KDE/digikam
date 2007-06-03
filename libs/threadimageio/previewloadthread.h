/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef PREVIEW_LOAD_THREAD_H
#define PREVIEW_LOAD_THREAD_H

#include "managedloadsavethread.h"

namespace Digikam
{

class DIGIKAM_EXPORT PreviewLoadThread : public ManagedLoadSaveThread
{
    Q_OBJECT

public:

    PreviewLoadThread();

    /**
     * Load a preview that is optimized for fast loading.
     */
    void load(LoadingDescription description);
    /**
     * Load a preview with higher resolution, trading more quality
     * for less speed.
     * In the LoadingDescription container, provide "0" as maximum size.
     */
    void loadHighQuality(LoadingDescription description);

};

}   // namespace Digikam


#endif // SHARED_LOAD_SAVE_THREAD_H
