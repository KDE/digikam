/* ============================================================
 * File  : imageeffect_border.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-20
 * Description : a Digikam image plugin for add a border  
 *               to an image.
 * 
 * Copyright 2005 by Gilles Caulier
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
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qtimer.h> 

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
    m_timer = 0;
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
    m_borderType->insertItem( i18n("Decorative Pine") );
    m_borderType->insertItem( i18n("Decorative Wood") );
    m_borderType->insertItem( i18n("Decorative Paper") );
    m_borderType->insertItem( i18n("Decorative Parque") );
    m_borderType->insertItem( i18n("Decorative Ice") );
    m_borderType->insertItem( i18n("Decorative Leaf") );
    m_borderType->insertItem( i18n("Decorative Marble") );
    m_borderType->insertItem( i18n("Decorative Rain") );

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
            this, SLOT(slotTimer()));            

    connect(m_foregroundColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorForegroundChanged(const QColor &)));            

    connect(m_backgroundColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorBackgroundChanged(const QColor &)));            
}

ImageEffect_Border::~ImageEffect_Border()
{
    if (m_timer)
       delete m_timer;
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

void ImageEffect_Border::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
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

       case 3: // Decorative Pine.
       case 4: // Decorative Wood.
       case 5: // Decorative Paper.
       case 6: // Decorative Parque.
       case 7: // Decorative Ice.
       case 8: // Decorative Leaf.
       case 9: // Decorative Marble.
       case 10:// Decorative Rain.
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

       case 3: // Decorative Pine.
       case 4: // Decorative Wood.
       case 5: // Decorative Paper.
       case 6: // Decorative Parque.
       case 7: // Decorative Ice.
       case 8: // Decorative Leaf.
       case 9: // Decorative Marble.
       case 10:// Decorative Rain.
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

       case 3: // Decorative Pine.
       case 4: // Decorative Wood.
       case 5: // Decorative Paper.
       case 6: // Decorative Parque.
       case 7: // Decorative Ice.
       case 8: // Decorative Leaf.
       case 9: // Decorative Marble.
       case 10:// Decorative Rain.
          m_foregroundColorButton->setEnabled(false);
          m_backgroundColorButton->setEnabled(false);
          m_labelForeground->setEnabled(false);
          m_labelBackground->setEnabled(false);
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

       case 3: // Decorative Pine.
       case 4: // Decorative Wood.
       case 5: // Decorative Paper.
       case 6: // Decorative Parque.
       case 7: // Decorative Ice.
       case 8: // Decorative Leaf.
       case 9: // Decorative Marble.
       case 10:// Decorative Rain.
          setCursor( KCursor::waitCursor() );
          pattern(src, dest, borderWidth);
          setCursor( KCursor::arrowCursor() );        
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

       case 3: // Decorative Pine.
       case 4: // Decorative Wood.
       case 5: // Decorative Paper.
       case 6: // Decorative Parque.
       case 7: // Decorative Ice.
       case 8: // Decorative Leaf.
       case 9: // Decorative Marble.
       case 10:// Decorative Rain.
          pattern(src, dest, m_borderWidth->value());
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
          output[x] = topColor.rgb();
        
       for(;x < dest.width(); ++x)
          output[x] = btmColor.rgb();
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
          output[x] = topColor.rgb();
          
       for(; x < dest.width(); ++x)
          output[x] = btmColor.rgb();
       }
       
    KImageEffect::blendOnLower(borderWidth, borderWidth, src, dest);
}

void ImageEffect_Border::pattern(QImage &src, QImage &dest, int borderWidth)
{
    QString pattern;
    
    switch (m_borderType->currentItem())
       {
       case 3: // Decorative Pine.
          pattern = "tree-pattern";
          break;
          
       case 4: // Decorative Wood.
          pattern = "wood-pattern";
          break;
       
       case 5: // Decorative Paper.
          pattern = "paper-pattern";
          break;
       
       case 6: // Decorative Parque.
          pattern = "parque-pattern";
          break;
       
       case 7: // Decorative Ice.
          pattern = "ice-pattern";
          break;
       
       case 8: // Decorative Leaf.
          pattern = "leaf-pattern";
          break;

       case 9: // Decorative Marble.
          pattern = "marble-pattern";
          break;
       
       case 10:// Decorative Rain.
          pattern = "rain-pattern";
          break;

       }
    
    QPixmap patternPixmap(m_previewWidget->imageIface()->originalWidth() + borderWidth*2,
                          m_previewWidget->imageIface()->originalHeight() + borderWidth*2);
    
    KGlobal::dirs()->addResourceType(pattern.ascii(), KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    QString path = KGlobal::dirs()->findResourceDir(pattern.ascii(), pattern + ".png");
    
    QPainter p(&patternPixmap);
    p.fillRect( 0, 0, patternPixmap.width(), patternPixmap.height(),
                QBrush::QBrush(QColor::QColor(Qt::black),
                QPixmap::QPixmap(path + pattern + ".png")) );
    p.end();
    
    dest = patternPixmap.convertToImage().scale( src.width() + borderWidth*2,
                                                 src.height() + borderWidth*2 );
    KImageEffect::blendOnLower(borderWidth, borderWidth, src, dest);
}

}  // NameSpace DigikamBorderImagesPlugin

#include "imageeffect_border.moc"
