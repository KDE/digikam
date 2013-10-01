/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Thread actions task for image quality sorter.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEQUALITYTASK_H
#define IMAGEQUALITYTASK_H

// Qt includes

#include <QImage>
#include <QThread>

// KDE includes

#include <threadweaver/Job.h>

using namespace ThreadWeaver;

namespace Digikam
{

class ImageQualitySettings;

class ImageQualityTask : public Job
{
    Q_OBJECT

public:

    ImageQualityTask();
    ~ImageQualityTask();

    void setItem(const QString& path, const ImageQualitySettings& quality);

Q_SIGNALS:

    void signalFinished(const QImage&);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMAGEQUALITYTASK_H */
