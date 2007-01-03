/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * Copyright 2006 by Marcel Wiesweg
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

    void load(LoadingDescription description);

signals:

    // This signal is emitted when the loading process has finished.
    // If the process failed, img is null.
    // The returned image is read-only. A copy needs to be made if changes are needed.
    void signalPreviewLoaded(const LoadingDescription &loadingDescription, const QImage &image);

public:

    void previewLoaded(const LoadingDescription &loadingDescription, const QImage &image)
            { emit signalPreviewLoaded(loadingDescription, image); };

protected slots:

    void slotTranslateLoadedSignal(const LoadingDescription &loadingDescription, const DImg& img);

};

}   // namespace Digikam


#endif // SHARED_LOAD_SAVE_THREAD_H
