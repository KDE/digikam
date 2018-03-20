/* ===============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-01
 * Description : Dialog to allow a custom page layout
 *
 * Copyright (C) 2010-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 * ============================================================== */

#ifndef ADV_PRINT_CUSTOM_DLG_H
#define ADV_PRINT_CUSTOM_DLG_H

// Qt includes

#include <QDialog>
#include <QWidget>

// Local includes

#include "ui_advprintcustomlayout.h"

namespace Digikam
{

class AdvPrintCustomLayoutDlg : public QDialog,
                                public Ui::AdvPrintCustomLayout
{
    Q_OBJECT

public:

    explicit AdvPrintCustomLayoutDlg(QWidget* const parent = 0);
    ~AdvPrintCustomLayoutDlg();

    void readSettings();
    void saveSettings();

private:

    enum CustomChoice
    {
        PHOTO_GRID              = 1,
        FIT_AS_MANY_AS_POSSIBLE = 2
    };
};

} // namespace Digikam

#endif // ADV_PRINT_CUSTOM_DLG_H
