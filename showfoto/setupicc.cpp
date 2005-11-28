/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
 *         F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-11-24
 * Description : ICC profils setup tab.
 * 
 * Copyright 2005 by Gilles Caulier and F.J. Cruz
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

// QT includes.

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// // Local includes.

#include "setupicc.h"


SetupICC::SetupICC(QWidget* parent )
        : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QVBoxLayout *layout = new QVBoxLayout( this, 0, KDialog::spacingHint());

   // --------------------------------------------------------

   // TODO

   // --------------------------------------------------------

   layout->addStretch();

   readSettings();
   adjustSize();

   mainLayout->addWidget(this);
}

SetupICC::~SetupICC()
{
}

void SetupICC::applySettings()
{
    // TODO
}

void SetupICC::readSettings()
{
    // TODO
}

#include "setupicc.moc"
