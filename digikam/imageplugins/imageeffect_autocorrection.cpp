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

// Digikam includes.

#include <imageiface.h>
#include <imagefilters.h>
#include <imagewidget.h>

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
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of auto-color correction tool"));

    l->addWidget(m_previewWidget, 0);
    topLayout->addWidget(frame);
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Type:"), plainPage());
    m_typeCB = new QComboBox( false, plainPage() );
    m_typeCB->insertItem( previewEffectPic("autolevels"),      i18n("Auto Levels") );
    m_typeCB->insertItem( previewEffectPic("normalize"),       i18n("Normalize") );
    m_typeCB->insertItem( previewEffectPic("equalize"),        i18n("Equalize") );
    m_typeCB->insertItem( previewEffectPic("stretchcontrast"), i18n("Stretch Contrast") );
    m_typeCB->setCurrentText( i18n("Auto Levels") );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the auto-color correction tool to use:<p>"
                                    "<b>Auto Levels</b>: this option enhances the contrast and brightness "
                                    "of the RGB values of an image by stretching the lowest "
                                    "and highest values to their fullest range, adjusting "
                                    "everything in between.<p>"
                                    "<b>Normalize</b>: this option scales brightness values across the active "
                                    "image so that the darkest point becomes black, and the "
                                    "brightest point becomes as bright as possible without "
                                    "altering its hue. This is often a \"magic fix\" for "
                                    "images that are dim or washed out.<p>"
                                    "<b>Equalize</b>: this option adjusts the brightness of colors across the "
                                    "active image so that the histogram for the Value channel "
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

    adjustSize();
    
    QTimer::singleShot(0, this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    connect(m_typeCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
}

ImageEffect_AutoCorrection::~ImageEffect_AutoCorrection()
{
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
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint * data = iface->getPreviewData();
    int w       = iface->previewWidth();
    int h       = iface->previewHeight();

    int type = m_typeCB->currentItem();

    autoCorrection(data, w, h, type);

    iface->putPreviewData(data);

    delete [] data;

    m_previewWidget->update();
    m_parent->setCursor( KCursor::arrowCursor() );
}

void ImageEffect_AutoCorrection::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();

    if (data) 
       {
       int type = m_typeCB->currentItem();

       autoCorrection(data, w, h, type);

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
                                                  
       iface->putOriginalData(name, data);

       delete [] data;
       }

    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

void ImageEffect_AutoCorrection::autoCorrection(uint *data, int w, int h, int type)
{
    switch (type)
       {
       case AutoLevelsCorrection:
          Digikam::ImageFilters::autoLevelsCorrectionImage(data, w, h);
          break;
       
       case NormalizeCorrection:
          Digikam::ImageFilters::normalizeImage(data, w, h);
          break;
       
       case EqualizeCorrection:
          Digikam::ImageFilters::equalizeImage(data, w, h);
          break;
       
       case StretchContrastCorrection:
          Digikam::ImageFilters::stretchContrastImage(data, w, h);
          break;
       }
}

#include "imageeffect_autocorrection.moc"

