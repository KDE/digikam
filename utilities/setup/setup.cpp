/* ============================================================
 * File  : setup.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qtabwidget.h>
#include <qapplication.h>
#include <klocale.h>

#include "setupcamera.h"
#include "setupgeneral.h"

#include "setup.h"

Setup::Setup(QWidget* parent, const char* name)
    : KDialogBase(parent, name, true, i18n("Configure"),
                  Ok|Cancel, Ok, true )
{
    setWFlags(Qt::WDestructiveClose);

    QTabWidget *tabWidget = new QTabWidget(this);
    setMainWidget(tabWidget);

    generalPage_ = new SetupGeneral(tabWidget);
    tabWidget->insertTab(generalPage_, i18n("Albums"));

    cameraPage_ = new SetupCamera(tabWidget);
    tabWidget->insertTab(cameraPage_, i18n("Cameras"));


    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    show();
    int W=Setup::width (), H=Setup::height();
    move(QApplication::desktop()->width ()/2-(W/2), QApplication::desktop()->height()/2-(H/2));
}

Setup::~Setup()
{
}

void Setup::slotOkClicked()
{
    cameraPage_->applySettings();
    generalPage_->applySettings();
    close();
}

