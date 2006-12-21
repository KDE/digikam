/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-21-12
 * Description : a view to embed the image preview widget.
 * 
 * Copyright 2006 Gilles Caulier
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

#include <qwidget.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>

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
        buttonsArea        = 0;
        backButton         = 0;
        editButton         = 0;
        imagePreviewWidget = 0;
    }

    QPushButton        *backButton;
    QPushButton        *editButton;

    QWidget            *buttonsArea;

    ImagePreviewWidget *imagePreviewWidget;
};
    
ImagePreviewView::ImagePreviewView(QWidget *parent)
                : QVBox(parent)
{
    d = new ImagePreviewViewPriv;
    d->imagePreviewWidget = new ImagePreviewWidget(this);
    d->buttonsArea        = new QWidget(this);
    QHBoxLayout *hlay     = new QHBoxLayout(d->buttonsArea);
    d->backButton         = new QPushButton(i18n("Back to Album"), d->buttonsArea);
    d->editButton         = new QPushButton(i18n("Edit..."), d->buttonsArea);
    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    setMargin(0);
    setLineWidth(1);

    hlay->setMargin(KDialogBase::marginHint());
    hlay->addStretch(1);
    hlay->addWidget(d->backButton);
    hlay->addStretch(10);
    hlay->addWidget(d->editButton);
    hlay->addStretch(1);

    // ----------------------------------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));  

    connect(d->backButton, SIGNAL( clicked() ),
            this, SIGNAL( backToAlbumSignal() ) );
             
    connect(d->editButton, SIGNAL( clicked() ),
            this, SIGNAL( editImageSignal() ) );          
             
    connect(d->imagePreviewWidget, SIGNAL( previewStarted() ),
            this, SLOT( slotPreviewStarted() ) );          
    
    connect(d->imagePreviewWidget, SIGNAL( previewComplete() ),
            this, SLOT( slotPreviewComplete() ) );          
    
    connect(d->imagePreviewWidget, SIGNAL( previewFailed() ),
            this, SLOT( slotPreviewFailed() ) );    
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
    d->buttonsArea->setPaletteBackgroundColor(ThemeEngine::instance()->baseColor());
}

void ImagePreviewView::slotPreviewStarted()
{
    d->backButton->setEnabled(false);
    d->editButton->setEnabled(false);
}

void ImagePreviewView::slotPreviewComplete()
{
    d->backButton->setEnabled(true);
    d->editButton->setEnabled(true);
}

void ImagePreviewView::slotPreviewFailed()
{
    d->backButton->setEnabled(true);
    d->editButton->setEnabled(false);
}

}  // NameSpace Digikam

