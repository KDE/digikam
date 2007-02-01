/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-21-12
 * Description : a embeded view to show the image preview widget.
 * 
 * Copyright 2006-2007 Gilles Caulier
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

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "themeengine.h"
#include "imagepreviewview.h"
#include "imagepreviewview.moc"

namespace Digikam
{

class ImagePreviewViewPriv
{
public:

    ImagePreviewViewPriv()
    {
        imagePreviewWidget = 0;
    }

    ImagePreviewWidget *imagePreviewWidget;
};
    
ImagePreviewView::ImagePreviewView(QWidget *parent)
                : QVBox(parent)
{
    d = new ImagePreviewViewPriv;
    d->imagePreviewWidget = new ImagePreviewWidget(this);

    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain); 
    setMargin(0); 
    setLineWidth(1); 

    // ----------------------------------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));  

    connect(d->imagePreviewWidget, SIGNAL( previewComplete() ),
            this, SIGNAL( previewLoadedSignal() ) );          
    
    connect(d->imagePreviewWidget, SIGNAL( previewFailed() ),
            this, SIGNAL( previewLoadedSignal() ) );    
}

ImagePreviewView::~ImagePreviewView()
{
    delete d;
}

ImagePreviewWidget* ImagePreviewView::imagePreviewWidget()
{
    return d->imagePreviewWidget;
}

void ImagePreviewView::slotThemeChanged()
{
    setPaletteBackgroundColor(ThemeEngine::instance()->baseColor());
}

}  // NameSpace Digikam

