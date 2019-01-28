/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ADV_PRINT_THREAD_H
#define DIGIKAM_ADV_PRINT_THREAD_H

// Local includes

#include "advprintsettings.h"
#include "advprinttask.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class DIGIKAM_EXPORT AdvPrintThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit AdvPrintThread(QObject* const parent);
    ~AdvPrintThread();

    void preparePrint(AdvPrintSettings* const settings, int sizeIndex);
    void print(AdvPrintSettings* const settings);
    void preview(AdvPrintSettings* const settings, const QSize& size);

Q_SIGNALS:

    void signalProgress(int);
    void signalDone(bool);
    void signalMessage(const QString&, bool);
    void signalPreview(const QImage&);
};

} // namespace DigikamGenericPrintCreatorPlugin

#endif // DIGIKAM_ADV_PRINT_THREAD_H
