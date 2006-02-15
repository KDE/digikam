/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-05
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
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

namespace Digikam
{

class ImageResizeDlgPriv
{
public:

    ImageResizeDlgPriv()
    {
        width          = 0;
        height         = 0;
        constrainCheck = 0;
        wInput         = 0;
        hInput         = 0;
        wpInput        = 0;
        hpInput        = 0;
    }

    int             *width;
    int             *height;
    int              prevW;
    int              prevH;
    
    double           prevWP;
    double           prevHP;

    QCheckBox       *constrainCheck;

    KIntSpinBox     *wInput;
    KIntSpinBox     *hInput;
    KDoubleSpinBox  *wpInput;
    KDoubleSpinBox  *hpInput;
};

ImageResizeDlg::ImageResizeDlg(QWidget *parent, int *width, int *height)
              : KDialogBase(Plain, i18n("Resize Image"), Help|Ok|Cancel, Ok,
                            parent, 0, true, true)
{
    d = new ImageResizeDlgPriv;
    setHelp("resizetool.anchor", "digikam");
    
    d->width  = width;
    d->height = height;

    d->prevW  = *d->width;
    d->prevH  = *d->height;
    d->prevWP = 100.0;
    d->prevHP = 100.0;
    
    QGridLayout *topLayout = new QGridLayout( plainPage(), 0, 3, 4, spacingHint());
    QLabel *label = 0;

    // -------------------------------------------------------------
    
    label    = new QLabel(i18n("Width:"), plainPage(), "w");
    d->wInput = new KIntSpinBox(1, 9999, 1, *d->width, 10, plainPage());
    d->wInput->setName("w");
    topLayout->addWidget(label, 0, 0);
    topLayout->addWidget(d->wInput, 0, 1);
    
    label    = new QLabel(i18n("Height:"), plainPage());
    d->hInput = new KIntSpinBox(1, 9999, 1, *d->height, 10, plainPage());
    d->hInput->setName("h");
    topLayout->addWidget(label, 0, 2);
    topLayout->addWidget(d->hInput, 0, 3);

    label     = new QLabel(i18n("Width (%):"), plainPage());
    d->wpInput = new KDoubleSpinBox(1, 999, 1, 100, 1, plainPage());
    d->wpInput->setName("wp");
    topLayout->addWidget(label, 1, 0);
    topLayout->addWidget(d->wpInput, 1, 1);

    label    = new QLabel(i18n("Height (%):"), plainPage(), "hp");
    d->hpInput = new KDoubleSpinBox(1, 999, 1, 100, 1, plainPage());
    d->hpInput->setName("hp");
    topLayout->addWidget(label, 1, 2);
    topLayout->addWidget(d->hpInput, 1, 3);

    d->constrainCheck = new QCheckBox(i18n("Maintain aspect ratio"),
                                     plainPage());
    topLayout->addMultiCellWidget(d->constrainCheck, 2, 2, 0, 3);

    d->constrainCheck->setChecked(true);

    // -------------------------------------------------------------
    
    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotChanged()));
            
    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotChanged()));
            
    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotChanged()));
            
    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotChanged()));
}

ImageResizeDlg::~ImageResizeDlg()
{
    delete d;
}

void ImageResizeDlg::slotOk()
{
    if (d->prevW != d->wInput->value() || d->prevH != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotChanged();
    
    *d->width  = d->prevW;
    *d->height = d->prevH;
    accept();
}

void ImageResizeDlg::slotChanged()
{
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);
    
    QString s(sender()->name());
    
    if (s == "w")
    {
        double val = d->wInput->value();
        double wp  = val/(double)(*d->width) * 100.0;
        d->wpInput->setValue(wp);

        if (d->constrainCheck->isChecked())
        {
            d->hpInput->setValue(wp);
            int h = (int)(wp*(*d->height)/100);
            d->hInput->setValue(h);
        }
    }
    else if (s == "h")
    {
        double val = d->hInput->value();
        double hp  = val/(double)(*d->height) * 100.0;
        d->hpInput->setValue(hp);

        if (d->constrainCheck->isChecked())
        {
            d->wpInput->setValue(hp);
            int w = (int)(hp*(*d->width)/100);
            d->wInput->setValue(w);
        }
    }
    else if (s == "wp")
    {
        double val = d->wpInput->value();
        int w      = (int)(val*(*d->width)/100);
        d->wInput->setValue(w);

        if (d->constrainCheck->isChecked())
        {
            d->hpInput->setValue(val);
            int h = (int)(val*(*d->height)/100);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hp")
    {
        double val = d->hpInput->value();
        int h      = (int)(val*(*d->height)/100);
        d->hInput->setValue(h);

        if (d->constrainCheck->isChecked())
        {
            d->wpInput->setValue(val);
            int w = (int)(val*(*d->width)/100);
            d->wInput->setValue(w);
        }
    }

    d->prevW = d->wInput->value();
    d->prevH = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();
    
    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

}  // namespace Digikam

#include "imageresizedlg.moc"
