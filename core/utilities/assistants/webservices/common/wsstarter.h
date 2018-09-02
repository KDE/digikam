/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-09-02
 * Description : Start Web Service methods.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef DIGIKAM_WS_STARTER_H
#define DIGIKAM_WS_STARTER_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DInfoInterface;

class DIGIKAM_EXPORT WSStarter : public QObject
{
    Q_OBJECT

public:

    static WSStarter* instance();

    static void cleanUp();

    static void exportFlickr(DInfoInterface* const iface, QWidget* const parent);

private:

    explicit WSStarter();
    ~WSStarter();

    void toFlickr(DInfoInterface* const iface, QWidget* const parent);

private:

    class Private;
    Private* const d;

    friend class WSStarterCreator;
};

} // namespace Digikam

#endif // DIGIKAM_WS_STARTER_H
