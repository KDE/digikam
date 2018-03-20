/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADV_PRINT_TASK_H
#define ADV_PRINT_TASK_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QPainter>
#include <QList>
#include <QRect>
#include <QImage>

// Local includes

#include "advprintsettings.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

namespace Digikam
{

class AdvPrintTask : public ActionJob
{
    Q_OBJECT

public:

    enum PrintMode
    {
        PREPAREPRINT = 0,
        PRINT,
        PREVIEW
    };

public:

    explicit AdvPrintTask(AdvPrintSettings* const settings,
                          PrintMode mode,
                          const QSize& size = QSize(),        // For PREVIEW stage.
                          int sizeIndex = 0);                 // For PREPAREPRINT stage.
    ~AdvPrintTask();

Q_SIGNALS:

    void signalMessage(const QString&, bool);
    void signalDone(bool);
    void signalPreview(const QImage&);

private:

    void run();

    void        preparePrint();
    void        printPhotos();
    QStringList printPhotosToFile();

    double getMaxDPI(const QList<AdvPrintPhoto*>& photos,
                     const QList<QRect*>& layouts,
                     int current);

    void printCaption(QPainter& p,
                      AdvPrintPhoto* const photo,
                      int captionW,
                      int captionH,
                      const QString& caption);

    bool paintOnePage(QPainter& p,
                      const QList<AdvPrintPhoto*>& photos,
                      const QList<QRect*>& layouts,
                      int& current,
                      bool cropDisabled,
                      bool useThumbnails = false);


private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_TASK_H
