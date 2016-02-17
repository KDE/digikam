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

#ifndef CALTEMPLATE_H
#define CALTEMPLATE_H

// Qt includes

#include <QWidget>

// Local include

#include "ui_caltemplate.h"

namespace Digikam
{

class CalMonthWidget;

class CalTemplate : public QWidget
{
    Q_OBJECT

public:

    CalTemplate(const QList<QUrl>& urlList, QWidget* const parent);
    ~CalTemplate();

private Q_SLOTS:

    void yearChanged(int year);
    void monthChanged(int);

private:

    Ui::CalTemplate       m_ui;
    QVector<CalMonthWidget*> m_wVector;
};

}  // Namespace Digikam

#endif // CALTEMPLATE_H
