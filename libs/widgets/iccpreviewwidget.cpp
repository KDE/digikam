/* ============================================================
 * Author: Francisco J. Cruz <fj.cruz@supercable.es>
 * Date  : 2006-01-12
 * Description :a widget to display ICC profiles metada in file
 * dialog preview.
 * 
 * Copyright 2006 by Francisco J. Cruz
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

#include <config.h>

#include "iccpreviewwidget.h"

// Qt includes

#include <qlayout.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qdir.h>

// KDE includes

#include <klocale.h>
#include <kdialog.h>

// Others

#include LCMS_HEADER

namespace Digikam {

ICCPreviewWidget::ICCPreviewWidget(QWidget *parent)
                    : KPreviewWidgetBase( parent )
{
    //TODO
    QVBoxLayout *layout = new QVBoxLayout(parent, 0,  KDialog::spacingHint());
    
    QVGroupBox *metaData = new QVGroupBox(parent);
    
    QHGroupBox *name = new QHGroupBox(metaData);
    name->setFrameStyle(QFrame::NoFrame);
    QLabel *label1 = new QLabel(i18n("Name: "), name);
    m_name = new QLabel(0, name);

    QHGroupBox *description = new QHGroupBox(metaData);
    description->setFrameStyle(QFrame::NoFrame);
    QLabel *label2 = new QLabel(i18n("Description: "), description);
    m_description = new QLabel(0, description);

    QHGroupBox *colorSpace = new QHGroupBox(metaData);
    colorSpace->setFrameStyle(QFrame::NoFrame);
    QLabel *label3 = new QLabel(i18n("Color space: "), colorSpace);
    m_colorSpace = new QLabel(0, colorSpace);

    QHGroupBox *deviceClass = new QHGroupBox(metaData);
    deviceClass->setFrameStyle(QFrame::NoFrame);
    QLabel *label4 = new QLabel(i18n("Device class: "), deviceClass);
    m_deviceClass = new QLabel(0, deviceClass);
    
    layout->addWidget(metaData);
}


ICCPreviewWidget::~ICCPreviewWidget()
{
}

void ICCPreviewWidget::showPreview( const KURL &url)
{
    //TODO
}

void ICCPreviewWidget::clearPreview()
{
    //TODO
}

void ICCPreviewWidget::getICCData( const KURL &url)
{
    //TODO
    cmsHPROFILE tmpProfile=0;
    tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(url.path()), "r");
}

}

#include "iccpreviewwidget.moc"
