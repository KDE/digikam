/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-05
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

// Qt includes.
 
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "imageresizedlg.h"

ImageResizeDlg::ImageResizeDlg(QWidget *parent, int *width, int *height)
              : KDialogBase(Plain, i18n("Resize Image"), Help|Ok|Cancel, Ok,
                            parent, 0, true, true)
{
    setHelp("resizetool.anchor", "digikam");
    
    m_width  = width;
    m_height = height;

    m_prevW  = *m_width;
    m_prevH  = *m_height;
    m_prevWP = 100.0;
    m_prevHP = 100.0;
    
    QGridLayout *topLayout =
        new QGridLayout( plainPage(), 0, 3, 4, spacingHint());

    QLabel      *label;

    label    = new QLabel(i18n("Width:"), plainPage(), "w");
    m_wInput = new KIntSpinBox(1, 9999, 1, *m_width, 10, plainPage()); 
    m_wInput->setName("w");
    topLayout->addWidget(label, 0, 0);
    topLayout->addWidget(m_wInput, 0, 1);
    
    label    = new QLabel(i18n("Height:"), plainPage());
    m_hInput = new KIntSpinBox(1, 9999, 1, *m_height, 10, plainPage());
    m_hInput->setName("h");
    topLayout->addWidget(label, 0, 2);
    topLayout->addWidget(m_hInput, 0, 3);

    label     = new QLabel(i18n("Width (%):"), plainPage());
    m_wpInput = new KDoubleSpinBox(1, 999, 1, 100, 1, plainPage()); 
    m_wpInput->setName("wp");
    topLayout->addWidget(label, 1, 0);
    topLayout->addWidget(m_wpInput, 1, 1);

    label    = new QLabel(i18n("Height (%):"), plainPage(), "hp");
    m_hpInput = new KDoubleSpinBox(1, 999, 1, 100, 1, plainPage()); 
    m_hpInput->setName("hp");
    topLayout->addWidget(label, 1, 2);
    topLayout->addWidget(m_hpInput, 1, 3);

    m_constrainCheck = new QCheckBox(i18n("Maintain aspect ratio"),
                                     plainPage());
    topLayout->addMultiCellWidget(m_constrainCheck, 2, 2, 0, 3);

    m_constrainCheck->setChecked(true);


    connect(m_wInput, SIGNAL(valueChanged(int)),
            SLOT(slotChanged()));
    connect(m_hInput, SIGNAL(valueChanged(int)),
            SLOT(slotChanged()));
    connect(m_wpInput, SIGNAL(valueChanged(double)),
            SLOT(slotChanged()));
    connect(m_hpInput, SIGNAL(valueChanged(double)),
            SLOT(slotChanged()));
}

ImageResizeDlg::~ImageResizeDlg()
{
}

void ImageResizeDlg::slotOk()
{
    if (m_prevW != m_wInput->value() || m_prevH != m_hInput->value() ||
        m_prevWP != m_wpInput->value() || m_prevHP != m_hpInput->value())
        slotChanged();
    
    *m_width  = m_prevW;
    *m_height = m_prevH;
    accept();
}

void ImageResizeDlg::slotChanged()
{
    m_wInput->blockSignals(true);
    m_hInput->blockSignals(true);
    m_wpInput->blockSignals(true);
    m_hpInput->blockSignals(true);
    
    QString s(sender()->name());
    
    if (s == "w")
    {
        double val = m_wInput->value();
        double wp  = val/(double)(*m_width) * 100.0;
        m_wpInput->setValue(wp);

        if (m_constrainCheck->isChecked())
        {
            m_hpInput->setValue(wp);
            int h = (int)(wp*(*m_height)/100);
            m_hInput->setValue(h);
        }
    }
    else if (s == "h")
    {
        double val = m_hInput->value();
        double hp  = val/(double)(*m_height) * 100.0;
        m_hpInput->setValue(hp);

        if (m_constrainCheck->isChecked())
        {
            m_wpInput->setValue(hp);
            int w = (int)(hp*(*m_width)/100);
            m_wInput->setValue(w);
        }
    }
    else if (s == "wp")
    {
        double val = m_wpInput->value();
        int w      = (int)(val*(*m_width)/100);
        m_wInput->setValue(w);

        if (m_constrainCheck->isChecked())
        {
            m_hpInput->setValue(val);
            int h = (int)(val*(*m_height)/100);
            m_hInput->setValue(h);
        }
    }
    else if (s == "hp")
    {
        double val = m_hpInput->value();
        int h      = (int)(val*(*m_height)/100);
        m_hInput->setValue(h);

        if (m_constrainCheck->isChecked())
        {
            m_wpInput->setValue(val);
            int w = (int)(val*(*m_width)/100);
            m_wInput->setValue(w);
        }
    }

    m_prevW = m_wInput->value();
    m_prevH = m_hInput->value();
    m_prevWP = m_wpInput->value();
    m_prevHP = m_hpInput->value();
    
    m_wInput->blockSignals(false);
    m_hInput->blockSignals(false);
    m_wpInput->blockSignals(false);
    m_hpInput->blockSignals(false);
}

#include "imageresizedlg.moc"
