/* ============================================================
 * File  : imagerotatedlg.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-16
 * Description : a dialog for free resizing image operations.
 * 
 * Copyright 2004 by Gilles Caulier
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

// Qt includes. 
 
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "imagerotatedlg.h"

ImageRotateDlg::ImageRotateDlg(QWidget *parent, double *angle)
              : KDialogBase(Plain, i18n("Rotate Image"), Help|Ok|Cancel, Ok,
                            parent, 0, true, true)
{
    setHelp("imageeditor.anchor", "digikam");
    
    m_angle = angle;

    QGridLayout *topLayout =
        new QGridLayout( plainPage(), 0, 3, 4, spacingHint());

    QLabel *label = new QLabel(i18n("Angle :"), plainPage());
    m_angleInput = new KDoubleNumInput(plainPage()); 
    m_angleInput->setPrecision(1);
    m_angleInput->setRange(-180.0, 180.0, 0.1, true);
    m_angleInput->setValue(0.0);
    QWhatsThis::add( m_angleInput, 
                     i18n("<p>An angle in degrees by which to rotate the image."));
    topLayout->addWidget(label, 0, 0);
    topLayout->addWidget(m_angleInput, 0, 1);
}

ImageRotateDlg::~ImageRotateDlg()
{
}

void ImageRotateDlg::slotOk()
{
    *m_angle = m_angleInput->value();
    accept();
}

#include "imagerotatedlg.moc"
