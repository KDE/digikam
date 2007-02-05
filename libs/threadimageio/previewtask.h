/* ============================================================
 * Authors: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * Copyright 2006-2007 by Marcel Wiesweg, Gilles Caulier
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

#ifndef PREVIEW_TASK_H
#define PREVIEW_TASK_H

// Qt includes

#include <qimage.h>

// Local includes.

#include "loadsavetask.h"

namespace Digikam
{

class PreviewLoadedEvent : public NotifyEvent
{
public:

    PreviewLoadedEvent(const LoadingDescription &loadingDescription, const QImage &image)
        : m_loadingDescription(loadingDescription), m_image(image)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    LoadingDescription m_loadingDescription;
    QImage m_image;
};

//---------------------------------------------------------------------------------------------------

class PreviewLoadingTask : public SharedLoadingTask
{
public:

    PreviewLoadingTask(LoadSaveThread* thread, LoadingDescription description)
        : SharedLoadingTask(thread, description, LoadSaveThread::AccessModeRead, LoadingTaskStatusLoading)
        {}

    virtual void execute();

private:

    bool loadImagePreview(QImage& image, const QString& path);
    void exifRotate(const QString& filePath, QImage& thumb);
};


} // namespace Digikam

#endif // PREVIEW_TASK_H



