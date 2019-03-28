/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy thread.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2019      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_FC_THREAD_H
#define DIGIKAM_FC_THREAD_H

// Qt includes

#include <QMap>
#include <QUrl>

// Local includes

#include "actionthreadbase.h"

using namespace Digikam;

namespace DigikamGenericFileCopyPlugin
{

class FCThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit FCThread(QObject* const parent);
    ~FCThread();

    void setItemsUrlList(const QList<QUrl>& itemsList,
                         const QUrl& dstUrl, bool overwrite);
    void cancel();

Q_SIGNALS:

    void signalUrlProcessed(const QUrl& from, const QUrl& to);
    void signalCancelTask();
};

}  // namespace DigikamGenericFileCopyPlugin

#endif // DIGIKAM_FC_THREAD_H
