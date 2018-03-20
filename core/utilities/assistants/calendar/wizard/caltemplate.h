/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : template selection for calendar.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAL_TEMPLATE_H
#define CAL_TEMPLATE_H

// Qt includes

#include <QWidget>
#include <QList>
#include <QUrl>

// Local include

#include "ui_caltemplate.h"

namespace Digikam
{

class CalTemplate : public QWidget
{
    Q_OBJECT

public:

    explicit CalTemplate(const QList<QUrl>& urlList, QWidget* const parent);
    ~CalTemplate();

private Q_SLOTS:

    void yearChanged(int year);
    void monthChanged(int);

private:

    class Private;
    Private* const d;
};

} // Namespace Digikam

#endif // CAL_TEMPLATE_H
