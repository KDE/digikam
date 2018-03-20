/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Multithreaded loader for thumbnails
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAIL_TASK_H
#define THUMBNAIL_TASK_H

// Qt includes

#include <QImage>

// Local includes

#include "loadsavetask.h"

namespace Digikam
{

class ThumbnailCreator;

class ThumbnailLoadingTask : public SharedLoadingTask
{
public:

    ThumbnailLoadingTask(LoadSaveThread* const thread, const LoadingDescription& description);

    virtual void execute();
    virtual void setResult(const LoadingDescription& loadingDescription, const QImage& qimage);
    virtual void postProcess();

private:

    virtual void setResult(const LoadingDescription&, const DImg&) {};
    void setupCreator();

private:

    QImage            m_qimage;
    ThumbnailCreator* m_creator;
};

} // namespace Digikam

#endif // PREVIEW_TASK_H
