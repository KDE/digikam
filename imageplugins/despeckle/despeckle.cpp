/* ============================================================
 * File  : despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : Noise reduction image filter for ImageEditor
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

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qimage.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kimageeffect.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepreviewwidget.h>

// Local includes.

#include "despeckle.h"

namespace DigikamDespeckleFilterImagesPlugin
{

DespeckleDialog::DespeckleDialog(QWidget* parent)
               : KDialogBase(Plain, i18n("Noise reduction"), Help|Ok|Cancel,
                             Ok, parent, 0, true, true)
{
    m_parent = parent;
    
    // About data and help button.
    KAboutData* about = new KAboutData("despeckle", I18N_NOOP("Noise reduction"), "1.0",
                                       I18N_NOOP("A noise reduction filter plugin for Digikam."),
                                       KAboutData::License_GPL, "(c) 2004, Gilles Caulier", "",
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Noise reduction handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Noise reduction image preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------
    
    adjustSize();
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
}

DespeckleDialog::~DespeckleDialog()
{
}

void DespeckleDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("despeckle",
                                             "digikamimageplugins");
}

void DespeckleDialog::closeEvent(QCloseEvent *e)
{
    e->accept();    
}

void DespeckleDialog::slotEffect()
{
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    QImage newImage = KImageEffect::despeckle(image); 
    m_imagePreviewWidget->setPreviewImageData(newImage);
}

void DespeckleDialog::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();

    if (data) 
        {
        QImage image;
        image.create( w, h, 32 );
        image.setAlphaBuffer(true) ;
        memcpy(image.bits(), data, image.numBytes());

        QImage newImage = KImageEffect::despeckle(image); 
    
        memcpy(data, newImage.bits(), newImage.numBytes());
        iface.putOriginalData(data);
        delete [] data;
        }
    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

}  // NameSpace DigikamDespeckleFilterImagesPlugin

#include "despeckle.moc"
