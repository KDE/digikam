/* ============================================================
 * File  : imageresizedlg.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-05
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <knuminput.h>

#include "imageresizedlg.h"

ImageResizeDlg::ImageResizeDlg(QWidget *parent, int *width, int *height)
    : KDialogBase(Plain, i18n("Resize Image"), Ok|Cancel, Ok,
                  parent, 0, true, true)
{
    m_width  = width;
    m_height = height;

    QGridLayout *topLayout =
        new QGridLayout( plainPage(), 0, 3, 4, spacingHint());

    QLabel      *label;

    label    = new QLabel(i18n("Width :"), plainPage());
    m_wInput = new KIntSpinBox(1, 9999, 1, *m_width, 10, plainPage()); 
    topLayout->addWidget(label, 0, 0);
    topLayout->addWidget(m_wInput, 0, 1);
    
    label    = new QLabel(i18n("Height :"), plainPage());
    m_hInput = new KIntSpinBox(1, 9999, 1, *m_height, 10, plainPage()); 
    topLayout->addWidget(label, 0, 2);
    topLayout->addWidget(m_hInput, 0, 3);

    label     = new QLabel(i18n("Width % :"), plainPage());
    m_wpInput = new KDoubleSpinBox(1, 999, 1, 100, 1, plainPage()); 
    topLayout->addWidget(label, 1, 0);
    topLayout->addWidget(m_wpInput, 1, 1);

    label    = new QLabel(i18n("Height % :"), plainPage());
    m_hpInput = new KDoubleSpinBox(1, 999, 1, 100, 1, plainPage()); 
    topLayout->addWidget(label, 1, 2);
    topLayout->addWidget(m_hpInput, 1, 3);

    m_constrainCheck = new QCheckBox(i18n("Maintain Aspect Ratio"),
                                     plainPage());
    topLayout->addMultiCellWidget(m_constrainCheck, 2, 2, 0, 3);

    m_constrainCheck->setChecked(true);

    connect(m_wInput, SIGNAL(valueChanged(int)),
            SLOT(slotWChanged(int)));
    connect(m_hInput, SIGNAL(valueChanged(int)),
            SLOT(slotHChanged(int)));
    connect(m_wpInput, SIGNAL(valueChanged(double)),
            SLOT(slotWPChanged(double)));
    connect(m_hpInput, SIGNAL(valueChanged(double)),
            SLOT(slotHPChanged(double)));
}

ImageResizeDlg::~ImageResizeDlg()
{
    
}

void ImageResizeDlg::slotOk()
{
    *m_width  = m_wInput->value();
    *m_height = m_hInput->value();
    accept();
}

void ImageResizeDlg::slotWChanged(int val)
{
    double wp = (double)val/(double)(*m_width) * 100.0;
    m_wpInput->setValue(wp);

    if (m_constrainCheck->isChecked()) 
        m_hpInput->setValue(wp);
}

void ImageResizeDlg::slotHChanged(int val)
{
    double hp = (double)val/(double)(*m_height) * 100;
    m_hpInput->setValue(hp);

    if (m_constrainCheck->isChecked())
        m_wpInput->setValue(hp);
}

void ImageResizeDlg::slotWPChanged(double val)
{
    int w = (int)(val*(*m_width)/100);
    m_wInput->setValue(w);
}

void ImageResizeDlg::slotHPChanged(double val)
{
    int h = (int)(val*(*m_height)/100);
    m_hInput->setValue(h);
}

#include "imageresizedlg.moc"
