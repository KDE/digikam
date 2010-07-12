/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-11
 * Description : script interface for digiKam
 *
 * Copyright (C) 2010 by Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "scriptiface.h"
#include "ui_scriptiface.h"

namespace Digikam
{

scriptiface::scriptiface()
           : KDialog(),
             ui(new Ui::scriptiface)
{
    ui->setupUi(this);
}

scriptiface::~scriptiface()
{
    delete ui;
}

void scriptiface::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
             ui->retranslateUi(this);
             break;
        default:
             break;
    }
}

} // namespace DigiKam
