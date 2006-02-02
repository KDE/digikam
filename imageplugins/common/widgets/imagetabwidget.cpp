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

class ImageTabWidgetPriv
{
public:

    ImageTabWidgetPriv()
    {
        previewOriginalWidget = 0;
        previewTargetWidget   = 0;
    }

    ~ImageTabWidgetPriv()
    {
        delete previewOriginalWidget;
        delete previewTargetWidget;
    }
    
    Digikam::ImageGuideWidget *previewOriginalWidget;
    Digikam::ImageGuideWidget *previewTargetWidget;
};

ImageTabWidget::ImageTabWidget(QWidget *parent,
                               bool orgGuideVisible, bool targGuideVisible, 
                               int orgGuideMode, int targGuideMode)
              : QTabWidget(parent)
{
    d = new ImageTabWidgetPriv;
    QFrame *targetFrame = new QFrame(this);
    targetFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l2 = new QVBoxLayout(targetFrame, 5, 0);
    d->previewTargetWidget = new Digikam::ImageGuideWidget(480, 320, targetFrame,
                                          targGuideVisible, targGuideMode,
                                          Qt::red, 1, false);
                                         
    QWhatsThis::add( d->previewTargetWidget, i18n("<p>You can see here the target image."));
    l2->addWidget(d->previewTargetWidget, 0);
    addTab( targetFrame, i18n("Target") );
    
    // -------------------------------------------------------------
    
    QFrame *originalFrame = new QFrame(this);
    originalFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(originalFrame, 5, 0);
    d->previewOriginalWidget = new Digikam::ImageGuideWidget(480, 320, originalFrame,
                                            orgGuideVisible, orgGuideMode,
                                            Qt::red, 1, false);
    QWhatsThis::add( d->previewOriginalWidget, i18n("<p>You can see here the original image."));
    l->addWidget(d->previewOriginalWidget, 0);
    addTab( originalFrame, i18n("Original") );
}      

ImageTabWidget::~ImageTabWidget()
{
    delete d;
}

Digikam::ImageGuideWidget *ImageTabWidget::previewOriginal(void)
{
    return d->previewOriginalWidget;
}

Digikam::ImageGuideWidget *ImageTabWidget::previewTarget(void)
{
    return d->previewTargetWidget;
};
    
}  // namespace DigikamImagePlugins

#include "imagetabwidget.moc"
