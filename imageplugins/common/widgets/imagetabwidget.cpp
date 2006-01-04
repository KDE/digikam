/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : a tabulate image previews widget
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
 
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qframe.h>

// Local includes

#include "imagetabwidget.h"

namespace DigikamImagePlugins
{

ImageTabWidget::ImageTabWidget(QWidget *parent,
                               bool orgGuideVisible, bool targGuideVisible, 
                               int orgGuideMode, int targGuideMode)
              : QTabWidget(parent)
{
    QFrame *targetFrame = new QFrame(this);
    targetFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2 = new QVBoxLayout(targetFrame, 5, 0);
    m_previewTargetWidget = new Digikam::ImageGuideWidget(480, 320, targetFrame,
                                         targGuideVisible, targGuideMode,
                                         Qt::red, 1, false,
                                         Digikam::ImageGuideWidget::TargetPreviewImage);
                                         
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the target image."));
    l2->addWidget(m_previewTargetWidget, 0);
    addTab( targetFrame, i18n("Target") );
    
    // -------------------------------------------------------------
    
    QFrame *originalFrame = new QFrame(this);
    originalFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(originalFrame, 5, 0);
    m_previewOriginalWidget = new Digikam::ImageGuideWidget(480, 320, originalFrame,
                                           orgGuideVisible, orgGuideMode,
                                           Qt::red, 1, false,
                                           Digikam::ImageGuideWidget::OriginalImage);
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image."));
    l->addWidget(m_previewOriginalWidget, 0);
    addTab( originalFrame, i18n("Original") );
    
}      

ImageTabWidget::~ImageTabWidget()
{
}

}  // namespace DigikamImagePlugins

#include "imagetabwidget.moc"
