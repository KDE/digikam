/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-31
 * Description : Auto-Color correction tool.
 * 
 * Copyright 2005 by  Gilles Caulier
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
 
#include <qframe.h>
#include <qvgroupbox.h>
#include <qcombobox.h> 
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// Digikam includes.

#include "imageiface.h"
#include "imagefilters.h"
#include "imagewidget.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_autocorrection.h"


ImageEffect_AutoCorrection::ImageEffect_AutoCorrection(QWidget* parent)
                          : KDialogBase(Plain, i18n("Auto Color Correction"),
                                        Help|Ok|Cancel, Ok,
                                        parent, 0, true, true),
                            m_parent(parent)
{
    setHelp("autocolorcorrectiontool.anchor", "digikam");

    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the auto-color correction tool preview"));
    l->addWidget(m_previewWidget, 0);
    topLayout->addWidget(frame);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Type:"), plainPage());
    m_typeCB = new QComboBox( false, plainPage() );
    m_typeCB->insertItem( previewEffectPic("autolevels"),      i18n("Auto Levels") );
    m_typeCB->insertItem( previewEffectPic("normalize"),       i18n("Normalize") );
    m_typeCB->insertItem( previewEffectPic("equalize"),        i18n("Equalize") );
    m_typeCB->insertItem( previewEffectPic("stretchcontrast"), i18n("Stretch Contrast") );
    m_typeCB->setCurrentText( i18n("Auto Levels") );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the auto-color correction tool to use:<p>"
                                    "<b>Auto Levels</b>: This option maximizes the tonal range in the Red, "
                                    "Green, and Blue channels. It search the image shadow and highlight "
                                    "limit values and adjust the Red, Green, and Blue channels "
                                    "to a full histogram range.<p>"
                                    "<b>Normalize</b>: this option scales brightness values across the active "
                                    "image so that the darkest point becomes black, and the "
                                    "brightest point becomes as bright as possible without "
                                    "altering its hue. This is often a \"magic fix\" for "
                                    "images that are dim or washed out.<p>"
                                    "<b>Equalize</b>: this option adjusts the brightness of colors across the "
                                    "active image so that the histogram for the value channel "
                                    "is as nearly as possible flat, that is, so that each possible "
                                    "brightness value appears at about the same number of pixels "
                                    "as each other value. Sometimes Equalize works wonderfully at "
                                    "enhancing the contrasts in an image. Other times it gives "
                                    "garbage. It is a very powerful operation, which can either work "
                                    "miracles on an image or destroy it.<p>"
                                    "<b>Stretch Contrast</b>: this option enhances the contrast and brightness "
                                    "of the RGB values of an image by stretching the lowest "
                                    "and highest values to their fullest range, adjusting "
                                    "everything in between."
                                    ));

    hlay->addWidget(label, 1);
    hlay->addWidget(m_typeCB, 5);

    resize(configDialogSize("Auto-Color Correction Dialog"));
    
    QTimer::singleShot(0, this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    connect(m_typeCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
    
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));                
}

ImageEffect_AutoCorrection::~ImageEffect_AutoCorrection()
{
    saveDialogSize("Auto-Color Correction Dialog");
}

void ImageEffect_AutoCorrection::closeEvent(QCloseEvent *e)
{
    delete m_previewWidget;
    e->accept();
}

QPixmap ImageEffect_AutoCorrection::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( QPixmap::QPixmap(KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png") );
}

void ImageEffect_AutoCorrection::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    Digikam::DImg image        = iface->getPreviewImage();

    autoCorrection(image, m_typeCB->currentItem());

    iface->putPreviewImage(image);

    m_previewWidget->update();
    kapp->restoreOverrideCursor();
}

void ImageEffect_AutoCorrection::slotOk()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg image = iface.getOriginalImage();

    if (!image.isNull())
    {
       int type = m_typeCB->currentItem();
       autoCorrection(image, type);
       QString name;
       
       switch (type)
       {
          case AutoLevelsCorrection:
             name = i18n("Auto Levels");
          break;

          case NormalizeCorrection:
             name = i18n("Normalize");
          break;

          case EqualizeCorrection:
             name = i18n("Equalize");
          break;

          case StretchContrastCorrection:
             name = i18n("Stretch Contrast");
          break;
       }
                                                  
       iface.putOriginalImage(name, image);
    }

    kapp->restoreOverrideCursor();
    accept();
}

void ImageEffect_AutoCorrection::autoCorrection(Digikam::DImg& image, int type)
{
    switch (type)
    {
       case AutoLevelsCorrection:
          Digikam::ImageFilters::autoLevelsCorrectionImage(image.bits(), image.width(), image.height(), image.sixteenBit());
          break;
       
       case NormalizeCorrection:
          Digikam::ImageFilters::normalizeImage(image.bits(), image.width(), image.height(), image.sixteenBit());
          break;
       
       case EqualizeCorrection:
          Digikam::ImageFilters::equalizeImage(image.bits(), image.width(), image.height(), image.sixteenBit());
          break;
       
       case StretchContrastCorrection:
          Digikam::ImageFilters::stretchContrastImage(image.bits(), image.width(), image.height(), image.sixteenBit());
          break;
    }
}

#include "imageeffect_autocorrection.moc"

