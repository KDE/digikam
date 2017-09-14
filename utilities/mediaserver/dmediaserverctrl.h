/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server control widget.
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMEDIA_SERVER_CTRL_H
#define DMEDIA_SERVER_CTRL_H

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMediaServerCtrl : public QWidget
{
    Q_OBJECT

public:

    explicit DMediaServerCtrl(QWidget* const parent = 0);
    ~DMediaServerCtrl();

    void updateServerStatus();

Q_SIGNALS:
    
    void signalStartMediaServer();

private Q_SLOTS:

    void slotToggleMediaServer();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DMEDIA_SERVER_CTRL_H
