/* ============================================================
 * File  : imageeffect_border.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-20
 * Description : a Digikam image plugin for add a border  
 *               around an image.
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
#include <qpen.h>
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
                                       I18N_NOOP("A digiKam image plugin to add a border around an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
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
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 2, 2 , marginHint(), spacingHint());
    
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
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    QWhatsThis::add( m_previewWidget, i18n("This is the preview of the border added to the image."));
    l->addWidget(m_previewWidget, 0);
    topLayout->addMultiCellWidget(frame, 1, 1, 0, 0);
    topLayout->setRowStretch(1, 10);
    topLayout->setColStretch(0, 10);
        
    // -------------------------------------------------------------
    
    QVBoxLayout *vLayout2 = new QVBoxLayout( topLayout, spacingHint());                                                  
    QGroupBox *gbox2 = new QGroupBox(i18n("Settings"), plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 3, 2, 20, spacingHint());
                                                  
    QLabel *label1 = new QLabel(i18n("Type:"), gbox2);
    m_borderType = new QComboBox( false, gbox2 );
    m_borderType->insertItem( i18n("Solid") );
    // Niepce is Real name. This is the first guy in the world to have built a camera.
    m_borderType->insertItem( "Niepce" );     
    m_borderType->insertItem( i18n("Beveled") );
    m_borderType->insertItem( i18n("Decorative Pine") );
    m_borderType->insertItem( i18n("Decorative Wood") );
    m_borderType->insertItem( i18n("Decorative Paper") );
    m_borderType->insertItem( i18n("Decorative Parque") );
    m_borderType->insertItem( i18n("Decorative Ice") );
    m_borderType->insertItem( i18n("Decorative Leaf") );
    m_borderType->insertItem( i18n("Decorative Marble") );
    m_borderType->insertItem( i18n("Decorative Rain") );
    m_borderType->insertItem( i18n("Decorative Craters") );
    m_borderType->insertItem( i18n("Decorative Dried") );
    m_borderType->insertItem( i18n("Decorative Pink") );
    m_borderType->insertItem( i18n("Decorative Stone") );
    m_borderType->insertItem( i18n("Decorative Chalk") );
    m_borderType->insertItem( i18n("Decorative Granite") );
    m_borderType->insertItem( i18n("Decorative Rock") );
    m_borderType->insertItem( i18n("Decorative Wall") );
    QWhatsThis::add( m_borderType, i18n("<p>Select here the border type to add around the image."));
    gridBox2->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridBox2->addMultiCellWidget(m_borderType, 0, 0, 1, 1);
    
    QLabel *label2 = new QLabel(i18n("Width:"), gbox2);
    m_borderWidth = new KIntNumInput(gbox2);
    QWhatsThis::add( m_borderWidth, i18n("<p>Set here the border width in pixels to add around the image."));
    gridBox2->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridBox2->addMultiCellWidget(m_borderWidth, 1, 1, 1, 1);
            
    m_labelForeground = new QLabel(gbox2);
    m_firstColorButton = new KColorButton( QColor::QColor( 192, 192, 192 ), gbox2 );
    m_labelBackground = new QLabel(gbox2);
    m_secondColorButton = new KColorButton( QColor::QColor( 128, 128, 128 ), gbox2 );
    gridBox2->addMultiCellWidget(m_labelForeground, 2, 2, 0, 0);
    gridBox2->addMultiCellWidget(m_firstColorButton, 2, 2, 1, 1);
    gridBox2->addMultiCellWidget(m_labelBackground, 3, 3, 0, 0);
    gridBox2->addMultiCellWidget(m_secondColorButton, 3, 3, 1, 1);    

    vLayout2->addWidget(gbox2);
    vLayout2->addStretch();
    topLayout->addMultiCellLayout(vLayout2, 1, 1, 1, 1);            
    
    // -------------------------------------------------------------

    Digikam::ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();
    
    if (w > h) 
       m_borderWidth->setRange(1, h/2, 1, true);       
    else 
       m_borderWidth->setRange(1, w/2, 1, true);

    readSettings();
    resize(configDialogSize("Add Border Tool Dialog"));    
            
    // -------------------------------------------------------------
    
    connect(m_borderType, SIGNAL(activated(int)),
            this, SLOT(slotBorderTypeChanged(int)));
            
    connect(m_borderWidth, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            

    connect(m_firstColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorForegroundChanged(const QColor &)));            

    connect(m_secondColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorBackgroundChanged(const QColor &))); 
    
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));                                  
}

ImageEffect_Border::~ImageEffect_Border()
{
    saveDialogSize("Add Border Tool Dialog");
        
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Border::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Add Border Tool Settings");
    
    m_borderType->setCurrentItem( config->readNumEntry("Border Type", SolidBorder) );
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
    m_decorativeFirstColor = config->readColorEntry("Decorative First Color", black);; 
    m_decorativeSecondColor = config->readColorEntry("Decorative Second Color", black);
    
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
    config->writeEntry( "Decorative First Color", m_decorativeFirstColor );
    config->writeEntry( "Decorative Second Color", m_decorativeSecondColor );
    
    config->sync();
}

void ImageEffect_Border::slotHelp()
{
    KApplication::kApplication()->invokeHelp("border", "digikamimageplugins");
}

void ImageEffect_Border::closeEvent(QCloseEvent *e)
{
    e->accept();    
}

void ImageEffect_Border::slotUser1()
{
    m_borderType->blockSignals(true);
    m_borderWidth->blockSignals(true);
    m_firstColorButton->blockSignals(true);
    m_secondColorButton->blockSignals(true);
        
    m_borderType->setCurrentItem(SolidBorder); 
    m_borderWidth->setValue(100);
    m_solidColor = QColor::QColor( 0, 0, 0 );
    slotBorderTypeChanged(SolidBorder);

    m_borderType->blockSignals(false);
    m_borderWidth->blockSignals(false);
    m_firstColorButton->blockSignals(false);
    m_secondColorButton->blockSignals(false);
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
       case SolidBorder:
          m_solidColor = color;
          break;
       
       case NiepceBorder:
          m_niepceBorderColor = color;
          break;

       case BeveledBorder:
          m_bevelUpperLeftColor = color;
          break;
          
       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          m_decorativeFirstColor = color;
          break;
       }
       
    slotEffect();       
}

void ImageEffect_Border::slotColorBackgroundChanged(const QColor &color)
{
    switch (m_borderType->currentItem())
       {
       case SolidBorder:
          m_solidColor = color;
          break;
       
       case NiepceBorder:
          m_niepceLineColor = color;
          break;

       case BeveledBorder:
          m_bevelLowerRightColor = color;
          break;

       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          m_decorativeSecondColor = color;
          break;          
       }
       
    slotEffect();       
}

void ImageEffect_Border::slotBorderTypeChanged(int borderType)
{
    m_labelForeground->setText(i18n("First:"));
    m_labelBackground->setText(i18n("Second:"));
    QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the foreground color of the border."));
    QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the Background color of the border."));
    m_firstColorButton->setEnabled(true);
    m_secondColorButton->setEnabled(true);
    m_labelForeground->setEnabled(true);
    m_labelBackground->setEnabled(true);
    m_borderWidth->setEnabled(true);
          
    switch (borderType)
       {
       case SolidBorder:
          m_firstColorButton->setColor( m_solidColor );
          m_secondColorButton->setEnabled(false);
          m_labelBackground->setEnabled(false);
          break;
       
       case NiepceBorder:
          QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the color of the main border."));
          QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the color of the line."));
          m_firstColorButton->setColor( m_niepceBorderColor );
          m_secondColorButton->setColor( m_niepceLineColor );
          break;

       case BeveledBorder:
          QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the color of the upper left area."));
          QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the color of the lower right area."));
          m_firstColorButton->setColor( m_bevelUpperLeftColor );
          m_secondColorButton->setColor( m_bevelLowerRightColor );
          break;

       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the color of the first line."));
          QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the color of the second line."));
          m_firstColorButton->setColor( m_decorativeFirstColor );
          m_secondColorButton->setColor( m_decorativeSecondColor );
          break;
       }

    slotEffect();
}

void ImageEffect_Border::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data = iface->getPreviewData();
    int w      = iface->previewWidth();
    int h      = iface->previewHeight();
    
    int borderType = m_borderType->currentItem();
    float ratio = (float)w/(float)iface->originalWidth();
    int borderWidth = (int)((float)m_borderWidth->value()*ratio);
    
    QImage src, dest, tmp;
    src.create( w, h, 32 );
    memcpy(src.bits(), data, src.numBytes());

    switch (borderType)
       {
       case SolidBorder:
          solid(src, tmp, m_solidColor, borderWidth);
          break;
       
       case NiepceBorder:
          niepce(src, tmp, m_niepceBorderColor, borderWidth, 
                 m_niepceLineColor, 3);
          break;

       case BeveledBorder:
          bevel(src, tmp, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, borderWidth);
          break;

       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          pattern( src, tmp, borderWidth, m_decorativeFirstColor, m_decorativeSecondColor,
                 (int)(20.0*ratio), (int)(20.0*ratio) );
          break;
       }
    
    tmp = tmp.smoothScale(w, h, QImage::ScaleMin);
    dest.create( w, h, 32 );
    dest.fill(m_previewWidget->colorGroup().background().rgb());
    bitBlt( &dest, (w-tmp.width())/2, (h-tmp.height())/2, &tmp, 0, 0, tmp.width(), tmp.height());
    iface->putPreviewData((uint*)dest.bits());
    delete [] data;
    m_previewWidget->update();
    kapp->restoreOverrideCursor();
}

void ImageEffect_Border::slotOk()
{
    accept();
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    
    int borderType = m_borderType->currentItem();
    
    QImage src, dest;
    src.create( w, h, 32 );
    memcpy(src.bits(), data, src.numBytes());

    switch (borderType)
       {
       case SolidBorder:
          solid(src, dest, m_solidColor, m_borderWidth->value());
          break;
       
       case NiepceBorder:
          niepce(src, dest, m_niepceBorderColor, m_borderWidth->value(), 
                 m_niepceLineColor, 10);
          break;

       case BeveledBorder:
          bevel(src, dest, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, m_borderWidth->value());
          break;

       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          pattern(src, dest, m_borderWidth->value(), m_decorativeFirstColor, m_decorativeSecondColor, 20, 20);
          break;
       }

    iface->putOriginalData(i18n("Add Border"), (uint*)dest.bits(), dest.width(), dest.height());
       
    delete [] data;    
    
    writeSettings();
    kapp->restoreOverrideCursor();    
}

void ImageEffect_Border::solid(QImage &src, QImage &dest, const QColor &fg, int borderWidth)
{
    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    dest.fill(fg.rgb());
       
    bitBlt( &dest, borderWidth, borderWidth, &src, 0, 0, src.width(), src.height());
}

void ImageEffect_Border::niepce(QImage &src, QImage &dest, const QColor &fg, int borderWidth, 
                                const QColor &bg, int lineWidth)
{
    QImage tmp;
    solid(src, tmp, bg, lineWidth);
    solid(tmp, dest, fg, borderWidth);
}

void ImageEffect_Border::bevel(QImage &src, QImage &dest, const QColor &topColor, 
                               const QColor &btmColor, int borderWidth)
{
    unsigned int *output;
    int x, y;
    int wc;
    
    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    
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
       
    bitBlt( &dest, borderWidth, borderWidth, &src, 0, 0, src.width(), src.height());
}

void ImageEffect_Border::pattern(QImage &src, QImage &dest, int borderWidth,
                                 const QColor &firstColor, const QColor &secondColor, 
                                 int firstWidth, int secondWidth)
{
    QString pattern;
    
    switch (m_borderType->currentItem())
       {
       
       case PineBorder:
          pattern = "pine-pattern";
          break;
          
       case WoodBorder: 
          pattern = "wood-pattern";
          break;
       
       case PaperBorder: 
          pattern = "paper-pattern";
          break;
       
       case ParqueBorder: 
          pattern = "parque-pattern";
          break;
       
       case IceBorder:
          pattern = "ice-pattern";
          break;
       
       case LeafBorder: 
          pattern = "leaf-pattern";
          break;

       case MarbleBorder:
          pattern = "marble-pattern";
          break;
       
       case RainBorder:
          pattern = "rain-pattern";
          break;
       
       case CratersBorder:
          pattern = "craters-pattern";
          break;
       
       case DriedBorder:
          pattern = "dried-pattern";
          break;
       
       case PinkBorder:
          pattern = "pink-pattern";
          break;
       
       case StoneBorder:
          pattern = "stone-pattern";
          break;
       
       case ChalkBorder:
          pattern = "chalk-pattern";
          break;
       
       case GraniteBorder:
          pattern = "granit-pattern";
          break;

       case RockBorder:
          pattern = "rock-pattern";
          break;
       
       case WallBorder:
          pattern = "wall-pattern";
          break;
       }
    
    QPixmap patternPixmap(m_previewWidget->imageIface()->originalWidth() + m_borderWidth->value()*2,
                          m_previewWidget->imageIface()->originalHeight() + m_borderWidth->value()*2);
    
    KGlobal::dirs()->addResourceType(pattern.ascii(), KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    QString path = KGlobal::dirs()->findResourceDir(pattern.ascii(), pattern + ".png");
    
    // Pattern tile.
    QPainter p(&patternPixmap);
    p.fillRect( 0, 0, patternPixmap.width(), patternPixmap.height(),
                QBrush::QBrush(Qt::black,
                QPixmap::QPixmap(path + pattern + ".png")) );
    p.end();
    
    // First line around the pattern tile.
    QImage tmp2 = patternPixmap.convertToImage().smoothScale( src.width() + borderWidth*2, 
                                                              src.height() + borderWidth*2 );
    
    solid(tmp2, dest, firstColor, firstWidth);                                                 
    
    // Second line around original image.
    QImage tmp;
    solid(src, tmp, secondColor, secondWidth);                                                 
    
    // Copy original image.                                                 
    bitBlt( &dest, borderWidth, borderWidth, 
            &tmp, 0, 0, tmp.width(), tmp.height());
}

}  // NameSpace DigikamBorderImagesPlugin

#include "imageeffect_border.moc"
