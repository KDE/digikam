/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * Copyright 2005-2006 by Marcel Wiesweg, Gilles Caulier
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

// Local includes.

#include "previewloadthread.h"

namespace Digikam
{

PreviewLoadThread::PreviewLoadThread()
{
    connect(this, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg&)),
            this, SLOT(slotTranslateLoadedSignal(const LoadingDescription &, const DImg&)));
}

void PreviewLoadThread::load(LoadingDescription description)
{
    ManagedLoadSaveThread::loadPreview(description);
}

void PreviewLoadThread::slotTranslateLoadedSignal(const LoadingDescription &loadingDescription, const DImg& img)
{
    /*
        So what's this?
        Usually, there is a PreviewTask which will send a signalPreviewLoaded.
        However, in one case, a signalImageLoaded will be sent instead:
        If the task is waiting on a normal loading task in another thread which is currently
        loading the same image.
        This task will then send a LoadedEvent to all its listeners, which will trigger the
        signalImageLoaded, which is intercepted and translated here.
    */
    DImg copy(img);
    emit signalPreviewLoaded(loadingDescription, copy.copyQImage());
}

}   // namespace Digikam

