/* ============================================================
 * File  : imageeffect_border.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-20
 * Description : a Digikam image plugin for add a border  
 *               to an image.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Some code are inspired of border Algorithms from Pixie Plus.
 * Copyright (C) 1999-2001 Daniel M. Duley <mosfet@kde.org>
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

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcombobox.h>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kcolorbutton.h>
#include <kimageeffect.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_border.h"


namespace DigikamBorderImagesPlugin
{

ImageEffect_Border::ImageEffect_Border(QWidget* parent)
                  : KDialogBase(Plain, i18n("Add Border"),
                                Help|User1|Ok|Cancel, Ok,
                                parent, 0, true, true, i18n("&Reset Values")),
                    m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all parameters to the default values.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Add Border"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to add a border to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Add Border Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 5, 4 , marginHint(), spacingHint());
    
    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Add Border to Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 3);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the border added to the image.") );
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 3);
    
    // -------------------------------------------------------------
                                                  
    QLabel *label1 = new QLabel(i18n("Border Type:"), plainPage());
    
    m_borderType = new QComboBox( false, plainPage() );
    m_borderType->insertItem( i18n("Solid") );
    m_borderType->insertItem( i18n("Niepce") );
    m_borderType->insertItem( i18n("Beveled") );
    m_borderType->insertItem( i18n("Liquid") );
    m_borderType->insertItem( i18n("Round Corners") );
    QWhatsThis::add( m_borderType, i18n("<p>Select here the border type to add around the image."));
    
    topLayout->addMultiCellWidget(label1, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(m_borderType, 2, 2, 1, 1);
    QLabel *label2 = new QLabel(i18n("Width:"), plainPage());
    m_borderWidth = new KIntNumInput(plainPage());

    QWhatsThis::add( m_borderWidth, i18n("<p>Set here the border width in pixels to add around the image."));
    
    topLayout->addMultiCellWidget(label2, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(m_borderWidth, 3, 3, 1, 3);
        
    m_labelForeground = new QLabel(plainPage());
    m_foregroundColorButton = new KColorButton( QColor::QColor( 192, 192, 192 ), plainPage() );
    m_labelBackground = new QLabel(plainPage());
    m_backgroundColorButton = new KColorButton( QColor::QColor( 128, 128, 128 ), plainPage() );

    topLayout->addMultiCellWidget(m_labelForeground, 4, 4, 0, 0);
    topLayout->addMultiCellWidget(m_foregroundColorButton, 4, 4, 1, 1);
    topLayout->addMultiCellWidget(m_labelBackground, 4, 4, 2, 2);
    topLayout->addMultiCellWidget(m_backgroundColorButton, 4, 4, 3, 3);
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize();  
    readSettings();
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    int w = iface->originalWidth();
    int h = iface->originalHeight();
    
    if (w > h) 
       m_borderWidth->setRange(1, h/2, 1, true);       
    else 
       m_borderWidth->setRange(1, w/2, 1, true);
        
    // -------------------------------------------------------------
    
    connect(m_borderType, SIGNAL(activated(int)),
            this, SLOT(slotBorderTypeChanged(int)));
            
    connect(m_borderWidth, SIGNAL(valueChanged(int)),
            this, SLOT(slotEffect()));            

    connect(m_foregroundColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorForegroundChanged(const QColor &)));            

    connect(m_backgroundColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorBackgroundChanged(const QColor &)));            
}

ImageEffect_Border::~ImageEffect_Border()
{
}

void ImageEffect_Border::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Add Border Tool Settings");
    
    m_borderType->setCurrentItem( config->readNumEntry("Border Type", 0) );
    m_borderWidth->setValue( config->readNumEntry("Border Width", 100) );
    
    QColor *black = new QColor( 0, 0, 0 );
    QColor *white = new QColor( 255, 255, 255 );
    QColor *gray1 = new QColor( 192, 192, 192 );
    QColor *gray2 = new QColor( 128, 128, 128 );
    
    m_solidColor = config->readColorEntry("Solid Color", black);
    m_niepceBorderColor = config->readColorEntry("Niepce Border Color", white);
    m_niepceLineColor = config->readColorEntry("Niepce Line Color", black);
    m_bevelUpperLeftColor = config->readColorEntry("Bevel Upper Left Color", gray1);
    m_bevelLowerRightColor = config->readColorEntry("Bevel Lower Right Color", gray2);
    m_liquidBackgroundColor = config->readColorEntry("Liquid Background Color", gray1);
    m_liquidForegroundColor = config->readColorEntry("Liquid Foreground Color", gray2);
    m_roundCornerBackgroundColor = config->readColorEntry("Round Corner Background Color", gray1);
    
    delete black;
    delete white;
    delete gray1;
    delete gray2;
              
    slotBorderTypeChanged(m_borderType->currentItem());
}
    
void ImageEffect_Border::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Add Border Tool Settings");
    
    config->writeEntry( "Border Width", m_borderWidth->value() );
    config->writeEntry( "Border Type", m_borderType->currentItem() );
    
    config->writeEntry( "Solid Color", m_solidColor );
    config->writeEntry( "Niepce Border Color", m_niepceBorderColor );
    config->writeEntry( "Niepce Line Color", m_niepceLineColor );
    config->writeEntry( "Bevel Upper Left Color", m_bevelUpperLeftColor );
    config->writeEntry( "Bevel Lower Right Color", m_bevelLowerRightColor );
    config->writeEntry( "Liquid Background Color", m_liquidBackgroundColor );
    config->writeEntry( "Liquid Foreground Color", m_liquidForegroundColor );
    config->writeEntry( "Round Corner Background Color", m_roundCornerBackgroundColor );
    
    config->sync();
}

void ImageEffect_Border::slotHelp()
{
    KApplication::kApplication()->invokeHelp("borders",
                                             "digikamimageplugins");
}

void ImageEffect_Border::closeEvent(QCloseEvent *e)
{
    e->accept();    
}

void ImageEffect_Border::slotUser1()
{
    m_borderType->blockSignals(true);
    m_borderWidth->blockSignals(true);
    m_foregroundColorButton->blockSignals(true);
    m_backgroundColorButton->blockSignals(true);
        
    m_borderType->setCurrentItem(0);    // Solid.
    m_borderWidth->setValue(100);
    m_solidColor = QColor::QColor( 0, 0, 0 );
    slotBorderTypeChanged(0);

    m_borderType->blockSignals(false);
    m_borderWidth->blockSignals(false);
    m_foregroundColorButton->blockSignals(false);
    m_backgroundColorButton->blockSignals(false);
} 

void ImageEffect_Border::slotColorForegroundChanged(const QColor &color)
{
    switch (m_borderType->currentItem())
       {
       case 0: // Solid.
          m_solidColor = color;
          break;
       
       case 1: // Niepce.
          m_niepceBorderColor = color;
          break;

       case 2: // Beveled.
          m_bevelUpperLeftColor = color;
          break;

       case 3: // Liquid.
          m_liquidForegroundColor = color;
          break;

       case 4: // Round Corners.
          break;
       }
       
    slotEffect();       
}

void ImageEffect_Border::slotColorBackgroundChanged(const QColor &color)
{
    switch (m_borderType->currentItem())
       {
       case 0: // Solid.
          m_solidColor = color;
          break;
       
       case 1: // Niepce.
          m_niepceLineColor = color;
          break;

       case 2: // Beveled.
          m_bevelLowerRightColor = color;
          break;

       case 3: // Liquid.
          m_liquidBackgroundColor = color;
          break;

       case 4: // Round Corners.
          m_roundCornerBackgroundColor = color;
          break;
       }
       
    slotEffect();       
}

void ImageEffect_Border::slotBorderTypeChanged(int borderType)
{
    m_labelForeground->setText(i18n("Foreground:"));
    m_labelBackground->setText(i18n("Background:"));    
    QWhatsThis::add( m_foregroundColorButton, i18n("<p>Set here the foreground color of the border."));
    QWhatsThis::add( m_backgroundColorButton, i18n("<p>Set here the foreground color of the border."));
    m_foregroundColorButton->setEnabled(true);
    m_backgroundColorButton->setEnabled(true);
    m_labelForeground->setEnabled(true);
    m_labelBackground->setEnabled(true);
    m_borderWidth->setEnabled(true);
          
    switch (borderType)
       {
       case 0: // Solid.
          m_foregroundColorButton->setColor( m_solidColor );
          m_backgroundColorButton->setEnabled(false);
          m_labelBackground->setEnabled(false);
          m_borderWidth->setEnabled(true);
          break;
       
       case 1: // Niepce.
          m_labelForeground->setText(i18n("Main:"));
          m_labelBackground->setText(i18n("Line:"));
          QWhatsThis::add( m_foregroundColorButton, i18n("<p>Set here the color of the main border."));
          QWhatsThis::add( m_backgroundColorButton, i18n("<p>Set here the color of the line."));
          m_foregroundColorButton->setColor( m_niepceBorderColor );
          m_backgroundColorButton->setColor( m_niepceLineColor );
          break;

       case 2: // Beveled.
          m_labelForeground->setText(i18n("Upper Left:"));
          m_labelBackground->setText(i18n("Lower Right:"));
          QWhatsThis::add( m_foregroundColorButton, i18n("<p>Set here the color of the upper left area."));
          QWhatsThis::add( m_backgroundColorButton, i18n("<p>Set here the color of the lower right area."));
          m_foregroundColorButton->setColor( m_bevelUpperLeftColor );
          m_backgroundColorButton->setColor( m_bevelLowerRightColor );
          break;

       case 3: // Liquid.
          m_foregroundColorButton->setColor( m_liquidForegroundColor );
          m_backgroundColorButton->setColor( m_liquidBackgroundColor );
          break;

       case 4: // Round Corners.
          m_foregroundColorButton->setEnabled(false);
          m_backgroundColorButton->setColor( m_roundCornerBackgroundColor );
          m_labelForeground->setEnabled(false);
          m_borderWidth->setEnabled(false);
          break;
       }

    slotEffect();
}

void ImageEffect_Border::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data = iface->getPreviewData();
    int w      = iface->previewWidth();
    int h      = iface->previewHeight();
    
    int borderType = m_borderType->currentItem();
    float ratio = (float)w/(float)iface->originalWidth();
    int borderWidth = (int)((float)m_borderWidth->value()*ratio);
    
    QImage src, dest;
    src.create( w, h, 32 );
    src.setAlphaBuffer(true);
    memcpy(src.bits(), data, src.numBytes());

    switch (borderType)
       {
       case 0: // Solid.
          solid(src, dest, m_solidColor, borderWidth);
          break;
       
       case 1: // Niepce.
          niepce(src, dest, m_niepceBorderColor, borderWidth, 
                 m_niepceLineColor, 3);
          break;

       case 2: // Beveled.
          bevel(src, dest, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, borderWidth);
          break;

       case 3: // Liquid.
          liquid(src, dest, m_liquidForegroundColor, 
                 m_liquidBackgroundColor, borderWidth);
          break;

       case 4: // Round Corners.
          roundCorner(src, dest, m_roundCornerBackgroundColor);
          break;
       }

    iface->putPreviewData((uint*)dest.scale(w, h).bits());
    
    delete [] data;
    m_previewWidget->update();
}

void ImageEffect_Border::slotOk()
{
    accept();
    m_parent->setCursor( KCursor::waitCursor() );
    
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    
    int borderType = m_borderType->currentItem();
    
    QImage src, dest;
    src.create( w, h, 32 );
    src.setAlphaBuffer(true);
    memcpy(src.bits(), data, src.numBytes());

    switch (borderType)
       {
       case 0: // Solid.
          solid(src, dest, m_solidColor, m_borderWidth->value());
          break;
       
       case 1: // Niepce.
          niepce(src, dest, m_niepceBorderColor, m_borderWidth->value(), 
                 m_niepceLineColor, 10);
          break;

       case 2: // Beveled.
          bevel(src, dest, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, m_borderWidth->value());
          break;

       case 3: // Liquid.
          liquid(src, dest, m_liquidForegroundColor, 
                 m_liquidBackgroundColor, m_borderWidth->value());
          break;

       case 4: // Round Corners.
          roundCorner(src, dest, m_roundCornerBackgroundColor);
          break;
       }

    iface->putOriginalData((uint*)dest.bits(), dest.width(), dest.height());
       
    delete [] data;    
    
    writeSettings();

    m_parent->setCursor( KCursor::arrowCursor() );        
}

void ImageEffect_Border::solid(QImage &src, QImage &dest, const QColor &fg, int borderWidth)
{
    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    dest.setAlphaBuffer(true);
    dest.fill(fg.rgb());
       
    KImageEffect::blendOnLower(borderWidth, borderWidth, src, dest);
}

void ImageEffect_Border::niepce(QImage &src, QImage &dest, const QColor &fg, int borderWidth, const QColor &bg, int lineWidth)
{
    unsigned int *output;
    int x, y;
    
    dest.reset();
    dest.create(src.width() + borderWidth*2 + lineWidth*2, src.height() + borderWidth*2 + lineWidth*2, 32);
    dest.setAlphaBuffer(true);
    dest.fill(fg.rgb());
    
    // Copy original image.
                         
    KImageEffect::blendOnLower(borderWidth + lineWidth, borderWidth + lineWidth, src, dest);

    // Drawing fine line.
    // top

    for(y = borderWidth; y < borderWidth+lineWidth; ++y)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x = borderWidth; x < dest.width()-borderWidth; ++x)
          output[x] = bg.rgb();
       }
      
    // left and right
    
    for(y = borderWidth; y < dest.height()-borderWidth; ++y)
       {
       output = (unsigned int *)dest.scanLine(y);
        
       for(x = borderWidth; x < borderWidth+lineWidth; ++x)
            output[x] = bg.rgb();
            
       for(x = dest.width()-borderWidth-lineWidth; x < dest.width()-borderWidth; ++x)
            output[x] = bg.rgb();
       }
    
    // bottom
    
    for(y = dest.height()-borderWidth-lineWidth; y < dest.height()-borderWidth; ++y)
       {
       output = (unsigned int *)dest.scanLine(y);
        
       for(x = borderWidth; x < dest.width()-borderWidth; ++x)
          output[x] = bg.rgb();
       }        
}

void ImageEffect_Border::liquid(QImage &src, QImage &dest, const QColor &fg,
                                const QColor &bg, int borderWidth)
{
    unsigned int *output;
    int x, y;

    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    dest.setAlphaBuffer(true);
    
    QColor darkColor   = fg.dark(130);
    QColor lightColor1 = fg.light(110);
    QColor lightColor2 = fg.light(115);

    dest.fill(fg.rgb());

    // top line
    
    y=0;
    output = (unsigned int *)dest.scanLine(y);
    
    for(x=2; x < dest.width()-2; ++x)
       {
       output[x] = darkColor.rgb();
       }
    
    ++y;
    
    // second line
    
    output = (unsigned int *)dest.scanLine(y);
    output[1] = darkColor.rgb();
    output[dest.width()-2] = darkColor.rgb();
    ++y;
    
    // sides
    
    for(; y < dest.height()-2; ++y)
       {
       output = (unsigned int *)dest.scanLine(y);
       output[0] = darkColor.rgb();
       output[dest.width()-1] = darkColor.rgb();
       }
       
    // second to last line
    
    output = (unsigned int *)dest.scanLine(y);
    output[1] = darkColor.rgb();
    output[dest.width()-2] = darkColor.rgb();
    ++y;
    
    // last line
    
    output = (unsigned int *)dest.scanLine(y);
    
    for(x=2; x < dest.width()-2; ++x)
       {
       output[x] = darkColor.rgb();
       }

    // top fill
    
    y = 1;
    output = (unsigned int *)dest.scanLine(y);
    
    for(x=2; x < dest.width()-2; ++x)
        output[x] = lightColor1.rgb();
    
    ++y;
    output = (unsigned int *)dest.scanLine(y);
    
    for(x=1; x < dest.width()-1; ++x)
        output[x] = lightColor1.rgb();
    
    // bottom fill
    
    y = dest.height()-3;
    output = (unsigned int *)dest.scanLine(y);
    
    for(x=1; x < dest.width()-1; ++x)
        output[x] = lightColor2.rgb();
    
    ++y;
    output = (unsigned int *)dest.scanLine(y);
    
    for(x=2; x < dest.width()-2; ++x)
        output[x] = lightColor2.rgb();

    // fuzzy corners
    
    int red, green, blue;
    QColor blendColor(darkColor);
    red = (blendColor.red() >> 1) + (bg.red() >> 1);
    green = (blendColor.green() >> 1) + (bg.green() >> 1);
    blue = (blendColor.blue() >> 1) + (bg.blue() >> 1);
    blendColor.setRgb(red, green, blue);
    
    // top line
    
    y = 0;
    output = (unsigned int *)dest.scanLine(y);
    output[1] = blendColor.rgb();
    output[dest.width()-2] = blendColor.rgb();
    output[0] = bg.rgb();
    output[dest.width()-1] = bg.rgb();
    ++y;
    
    // second line
    
    output = (unsigned int *)dest.scanLine(y);
    output[0] = blendColor.rgb();
    output[dest.width()-1] = blendColor.rgb();
    
    // second to last line
    
    y = dest.height()-2;
    output = (unsigned int *)dest.scanLine(y);
    output[0] = blendColor.rgb();
    output[dest.width()-1] = blendColor.rgb();
    ++y;
    
    // last line
    
    output = (unsigned int *)dest.scanLine(y);
    output[1] = blendColor.rgb();
    output[dest.width()-2] = blendColor.rgb();
    output[0] = bg.rgb();
    output[dest.width()-1] = bg.rgb();

    // place image
    
    KImageEffect::blendOnLower(borderWidth, borderWidth, src, dest);
}

void ImageEffect_Border::bevel(QImage &src, QImage &dest, const QColor &topColor, 
                               const QColor &btmColor, int borderWidth)
{
    unsigned int *output;
    int x, y;
    int wc;
    
    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    dest.setAlphaBuffer(true);
    
    // top
    
    for(y=0, wc = dest.width()-1; y < borderWidth; ++y, --wc)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x=0; x < wc; ++x)
          {
          output[x] = topColor.rgb();
          }
        
       for(;x < dest.width(); ++x)
          {
          output[x] = btmColor.rgb();
          }
       }
       
    // left and right
    
    for(; y < dest.height()-borderWidth; ++y)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x=0; x < borderWidth; ++x)
          output[x] = topColor.rgb();
       
       for(x = dest.width()-1; x > dest.width()-borderWidth-1; --x)
          output[x] = btmColor.rgb();
       }
       
    // bottom
    
    for(wc = borderWidth; y < dest.height(); ++y, --wc)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x=0; x < wc; ++x)
          {
          output[x] = topColor.rgb();
          }
          
       for(; x < dest.width(); ++x)
          {
          output[x] = btmColor.rgb();
          }
       }
       
    KImageEffect::blendOnLower(borderWidth, borderWidth, src, dest);
}

void ImageEffect_Border::roundCorner(QImage &src, QImage &dest, const QColor &bg)
{
    int total, current;
    unsigned int *data;
    QColor c;
    int h, s, v;
    int r, g, b;
    int bgH, bgS, bgV;
    int alpha;
    QString path;
    QImage img;
    
    // round corners adds 1 pixel on top and left, 8 on bottom and right
    
    dest.reset();
    //dest.create(src.width()+9, src.height()+9, 32);
    dest.create(src.width(), src.height(), 32);
    dest.setAlphaBuffer(true);
    dest.fill(bg.rgb());

    // first do the shadowing
    
    KGlobal::dirs()->addResourceType("roundcorner-shadow", 
                                     KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    path = KGlobal::dirs()->findResourceDir("roundcorner-shadow", "roundcorner-shadow.png");
    img.load(path + "roundcorner-shadow.png");
    img.detach();
    
    if(img.depth() < 32)
       img = img.convertDepth(32);
    
    // convert shadow alpha layer to background color
    
    data = (unsigned int *)img.bits();
    total = img.width()*img.height();
    bg.hsv(&bgH, &bgS, &bgV);
    
    for(current=0; current<total; ++current)
       {
       alpha = qAlpha(data[current]);
       c.setRgb(data[current]);
        
       if(alpha == 0)
          {
          c = bg;
          alpha = 255;
          }
       else if(alpha != 255)
          {
          float srcPercent = ((float)alpha)/255.0;
          float destPercent = 1.0-srcPercent;
          r = (int)((srcPercent*c.red()) + (destPercent*bg.red()));
          g = (int)((srcPercent*c.green()) + (destPercent*bg.green()));
          b = (int)((srcPercent*c.blue()) + (destPercent*bg.blue()));
          c.setRgb(r, g, b);
          alpha = 255;
          }
       else
          {
          c.hsv(&h, &s, &v);
          c.setHsv(bgH, bgS, v);
          }
       data[current] = qRgba(c.red(), c.green(), c.blue(), alpha);
       }
       
    // tile shadow to destination

    // top left corner
    tileImage(dest, 0, 0, 14, 14, 
              img, 0, 0, 14, 14);
    
    // top right corner
    tileImage(dest, dest.width()-13, 0, 14, 14,
              img, img.width()-13, 0, 14, 14);
               
    // bottom left corner
    tileImage(dest, 0, dest.height()-13, 14, 14,
              img, 0, img.height()-13, 14, 14);
               
    // bottom right corner
    tileImage(dest, dest.width()-13, dest.height()-13, 14, 14,
              img, img.width()-13, img.height()-13, 14, 14);
               
    // top tile
    tileImage(dest, 13, 0, dest.width()-25, 14,
              img, 14, 0, 4, 14);
               
    // bottom tile
    tileImage(dest, 13, dest.height()-13, dest.width()-25, 14,
              img, 14, img.height()-13, 4, 14);
               
    // left tile
    tileImage(dest, 0, 13, 14, dest.height()-25,
              img, 0, 14, 14, 4);
               
    // right tile
    tileImage(dest, dest.width()-13, 13, 14, dest.height()-25,
              img, img.width()-13, 14, 14, 4);

    // done with shadow, calculate image mask and alpha layer
    
    KGlobal::dirs()->addResourceType("roundcorner-picfill", 
                                     KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    path = KGlobal::dirs()->findResourceDir("roundcorner-picfill", "roundcorner-picfill.png");
    img.load(path + "roundcorner-picfill.png");
    img.detach();

    if(img.depth() < 32)
       img = img.convertDepth(32);
        
    QImage blendedSrc = src;
    blendedSrc.detach();

    // top left corner
    copyImageSecondaryAlpha(blendedSrc, 0, 0, 14, 14, 
                            img, 0, 0, 14, 14);
    
    // top right corner
    copyImageSecondaryAlpha(blendedSrc, blendedSrc.width()-13, 0, 14, 14,
                            img, img.width()-13, 0, 14, 14);
    
    // bottom left corner
    copyImageSecondaryAlpha(blendedSrc, 0, blendedSrc.height()-13, 14, 14,
                            img, 0, img.height()-13, 14, 14);
    
    // bottom right corner
    copyImageSecondaryAlpha(blendedSrc, blendedSrc.width()-13, blendedSrc.height()-13, 14, 14,
                            img, img.width()-13, img.height()-13, 14, 14);
    
    // top tile
    copyImageSecondaryAlpha(blendedSrc, 13, 0, blendedSrc.width()-25, 14,
                            img, 14, 0, 2, 14);
    
    // bottom tile
    copyImageSecondaryAlpha(blendedSrc, 13, blendedSrc.height()-13, blendedSrc.width()-25, 14,
                            img, 14, img.height()-13, 2, 14);
    
    // left tile
    copyImageSecondaryAlpha(blendedSrc, 0, 13, 14, blendedSrc.height()-25,
                            img, 0, 14, 14, 2);
    
    // right tile
    copyImageSecondaryAlpha(blendedSrc, blendedSrc.width()-13, 13, 14, blendedSrc.height()-25,
                            img, img.width()-13, 14, 14, 2);

    KImageEffect::blendOnLower(0, 0, blendedSrc, dest);
}

void ImageEffect_Border::copyImageSecondaryAlpha(QImage &dest, int dx, int dy, int dw, int dh,
                                                 QImage &src, int sx, int sy, int sw, int sh)
{
    unsigned int *srcData, *destData;
    int orig_sx = sx;
    int orig_sy = sy;
    int orig_dx = dx;
    //int orig_dy = dy;
    int sx2 = sx+sw-1;
    int sy2 = sy+sh-1;
    int dx2 = dx+dw-1;
    int dy2 = dy+dh-1;

    int r, g, b, alpha;

    for(;dy < dy2; ++sy, ++dy)
       {
       if(sy > sy2)
            sy = orig_sy;
        
       srcData = (unsigned int *)src.scanLine(sy);
       destData = (unsigned int *)dest.scanLine(dy);
       
       for(sx=orig_sx, dx=orig_dx; dx < dx2; ++sx, ++dx)
          {
          if(sx > sx2)
                sx = orig_sx;
           
          r = qRed(destData[dx]);
          g = qGreen(destData[dx]);
          b = qBlue(destData[dx]);
          alpha = qAlpha(srcData[sx]);
          destData[dx] = qRgba(r, g, b, alpha);
          }
       }
}

void ImageEffect_Border::tileImage(QImage &dest, int dx, int dy, int dw, int dh, 
                                   QImage &src, int sx, int sy, int sw, int sh)
{
    unsigned int *srcData, *destData;
    int orig_sx = sx;
    int orig_sy = sy;
    int orig_dx = dx;
    //int orig_dy = dy;
    int sx2 = sx+sw-1;
    int sy2 = sy+sh-1;
    int dx2 = dx+dw-1;
    int dy2 = dy+dh-1;

    for(;dy < dy2; ++sy, ++dy)
       {
       if(sy > sy2)
          sy = orig_sy;
        
       srcData = (unsigned int *)src.scanLine(sy);
       destData = (unsigned int *)dest.scanLine(dy);
        
       for(sx=orig_sx, dx=orig_dx; dx < dx2; ++sx, ++dx)
          {
          if(sx > sx2)
             sx = orig_sx;
             
          destData[dx] = srcData[sx];
          }
       }
}

}  // NameSpace DigikamBorderImagesPlugin

#include "imageeffect_border.moc"
