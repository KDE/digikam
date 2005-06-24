/* ============================================================
 * File  : imageeffect_lensdistortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : a digiKam image plugin for to reduce spherical
 *               aberration provide by lens on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Original Distortion Correction algorithm copyrighted 
 * 2001-2003 David Hodson <hodsond@acm.org>
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
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qframe.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qtimer.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_lensdistortion.h"

namespace DigikamLensDistortionImagesPlugin
{

PixelAccess::PixelAccess(uint *data, int Width, int Height)
{
    m_width       = PixelAccessWidth;
    m_height      = PixelAccessHeight;
    m_depth       = 4;
    m_imageWidth  = Width;
    m_imageHeight = Height;
    m_srcPR       = data;
     
    m_image.create( m_imageWidth, m_imageHeight, 32 );
    memcpy(m_image.bits(), m_srcPR, m_image.numBytes());
    
    for ( int i = 0 ; i < PixelAccessRegions ; ++i ) 
       {
       m_buffer[i] = new uchar[m_height * m_width * m_depth];
       
       m_region = m_image.copy(0, 0, m_width, m_height);
       memcpy(m_buffer[i], m_region.bits(), m_region.numBytes());
       
       m_tileMinX[i] = 1;
       m_tileMaxX[i] = m_width - 2;
       m_tileMinY[i] = 1;
       m_tileMaxY[i] = m_height - 2;
       }
}

PixelAccess::~PixelAccess()
{
    for( int i = 0 ; i < PixelAccessRegions ; ++i ) 
       delete [] m_buffer[i];
}

uchar* PixelAccess::pixelAccessAddress(int i, int j)
{
    return m_buffer[0] + m_depth * (m_width * (j + 1 - m_tileMinY[0]) + (i + 1 - m_tileMinX[0]));
}

// Swap region[n] with region[0].
void PixelAccess::pixelAccessSelectRegion(int n)
{
    uchar* temp;
    int    a, b, c, d;
    int    i;

    temp = m_buffer[n];
    a    = m_tileMinX[n];
    b    = m_tileMaxX[n];
    c    = m_tileMinY[n];
    d    = m_tileMaxY[n];

    for( i = n ; i > 0 ; --i) 
       {
       m_buffer[i]   = m_buffer[i-1];
       m_tileMinX[i] = m_tileMinX[i-1];
       m_tileMaxX[i] = m_tileMaxX[i-1];
       m_tileMinY[i] = m_tileMinY[i-1];
       m_tileMaxY[i] = m_tileMaxY[i-1];
       }

    m_buffer[0]   = temp;
    m_tileMinX[0] = a;
    m_tileMaxX[0] = b;
    m_tileMinY[0] = c;
    m_tileMaxY[0] = d;
}

// Buffer[0] is cleared, should start at [i, j], fill rows that overlap image.
void PixelAccess::pixelAccessDoEdge(int i, int j)
{
    int    lineStart, lineEnd;
    int    rowStart, rowEnd;
    int    lineWidth;
    uchar* line;

    lineStart = i;
    if (lineStart < 0) lineStart = 0;
    lineEnd = i + m_width;
    if (lineEnd > m_imageWidth) lineEnd = m_imageWidth;
    lineWidth = lineEnd - lineStart;

    if( lineStart >= lineEnd ) 
       return;

    rowStart = j;
    if (rowStart < 0) rowStart = 0;
    rowEnd = j + m_height;
    if (rowEnd > m_imageHeight) rowEnd = m_imageHeight;

    for( int y = rowStart ; y < rowEnd ; ++y ) 
       {
       line = pixelAccessAddress(lineStart, y);

       m_region = m_image.copy(lineStart, y, lineWidth, 1);
       memcpy(line, m_region.bits(), m_region.numBytes());
       }
}

// Moves buffer[0] so that [x, y] is inside it.
void PixelAccess::pixelAccessReposition(int xInt, int yInt)
{
    int newStartX = xInt - PixelAccessXOffset;
    int newStartY = yInt - PixelAccessYOffset;

    m_tileMinX[0] = newStartX + 1;
    m_tileMaxX[0] = newStartX + m_width - 2;
    m_tileMinY[0] = newStartY + 1;
    m_tileMaxY[0] = newStartY + m_height - 2;

    if ( (newStartX < 0) || ((newStartX + m_width) >= m_imageWidth) ||
         (newStartY < 0) || ((newStartY + m_height) >= m_imageHeight) ) 
       {
       // some data is off edge of image 

       memset(m_buffer[0], 0, m_width * m_height * m_depth);

       if ( ((newStartX + m_width) < 0) || (newStartX >= m_imageWidth) ||
            ((newStartY + m_height) < 0) || (newStartY >= m_imageHeight) ) 
          {
          // totally outside, just leave it. 
          } 
       else 
          {
          pixelAccessDoEdge(newStartX, newStartY);
          }
       } 
    else 
       {
       m_region = m_image.copy(newStartX, newStartY, m_width, m_height);
       memcpy(m_buffer[0], m_region.bits(), m_region.numBytes());
       }
}

void PixelAccess::pixelAccessGetCubic(double srcX, double srcY, double brighten, uchar* dst, int dstDepth)
{
    int     xInt, yInt;
    double  dx, dy;
    uchar  *corner;

    xInt = (int)floor(srcX);
    dx   = srcX - xInt;
    yInt = (int)floor(srcY);
    dy   = srcY - yInt;

    // We need 4x4 pixels, xInt-1 to xInt+2 horz, yInt-1 to yInt+2 vert 
    // they're probably in the last place we looked... 
    
    if ((xInt >= m_tileMinX[0]) && (xInt < m_tileMaxX[0]) &&
        (yInt >= m_tileMinY[0]) && (yInt < m_tileMaxY[0]) ) 
      {
      corner = pixelAccessAddress(xInt - 1, yInt - 1);
      cubicInterpolate(corner, m_depth * m_width, m_depth, dst, dstDepth, dx, dy, brighten);
      return;
      }

    // Or maybe it was a while back... 
    
    for ( int i = 1 ; i < PixelAccessRegions ; ++i) 
       {
       if ((xInt >= m_tileMinX[i]) && (xInt < m_tileMaxX[i]) &&
           (yInt >= m_tileMinY[i]) && (yInt < m_tileMaxY[i]) ) 
          {
          // Check here first next time 
          
          pixelAccessSelectRegion(i);
          corner = pixelAccessAddress(xInt - 1, yInt - 1);
          cubicInterpolate(corner, m_depth * m_width, m_depth, dst, dstDepth, dx, dy, brighten);
          return;
          }
       }

    // Nope, recycle an old region.
    
    pixelAccessSelectRegion(PixelAccessRegions - 1);
    pixelAccessReposition(xInt, yInt);
    
    corner = pixelAccessAddress(xInt - 1, yInt - 1);
    cubicInterpolate(corner, m_depth * m_width, m_depth, dst, dstDepth, dx, dy, brighten);
}

/*
 * Catmull-Rom cubic interpolation
 *
 * equally spaced points p0, p1, p2, p3
 * interpolate 0 <= u < 1 between p1 and p2
 *
 * (1 u u^2 u^3) (  0.0  1.0  0.0  0.0 ) (p0)
 *               ( -0.5  0.0  0.5  0.0 ) (p1)
 *               (  1.0 -2.5  2.0 -0.5 ) (p2)
 *               ( -0.5  1.5 -1.5  0.5 ) (p3)
 *
 */
void PixelAccess::cubicInterpolate(uchar* src, int rowStride, int srcDepth, uchar* dst, 
                                   int dstDepth, double dx, double dy, double brighten)
{
    float um1, u, up1, up2;
    float vm1, v, vp1, vp2;
    int   c;
    float verts[4 * MAX_PIXEL_DEPTH];

    um1 = ((-0.5 * dx + 1.0) * dx - 0.5) * dx;
    u   = (1.5 * dx - 2.5) * dx * dx + 1.0;
    up1 = ((-1.5 * dx + 2.0) * dx + 0.5) * dx;
    up2 = (0.5 * dx - 0.5) * dx * dx;

    vm1 = ((-0.5 * dy + 1.0) * dy - 0.5) * dy;
    v   = (1.5 * dy - 2.5) * dy * dy + 1.0;
    vp1 = ((-1.5 * dy + 2.0) * dy + 0.5) * dy;
    vp2 = (0.5 * dy - 0.5) * dy * dy;

    // Note: if dstDepth < srcDepth, we calculate unneeded pixels here 
    // later - select or create index array.
  
    for (c = 0 ; c < 4 * srcDepth ; ++c) 
       {
       verts[c] = vm1 * src[c] + v * src[c+rowStride] + vp1 * src[c+rowStride*2] + vp2 * src[c+rowStride*3];
       }
  
    for (c = 0 ; c < dstDepth ; ++c) 
       {
       float result;
       result = um1 * verts[c] + u * verts[c+srcDepth] + up1 * verts[c+srcDepth*2] + up2 * verts[c+srcDepth*3];
       result *= brighten;
       
       if (result < 0.0) 
          {
          dst[c] = 0;
          }
       else if (result > 255.0) 
          {
          dst[c] = 255;
          } 
       else 
          {
          dst[c] = (uint)result;
          }
       }
}


///////////////////////////////////////////////////////////////////////////////////////////////

ImageEffect_LensDistortion::ImageEffect_LensDistortion(QWidget* parent)
                          : KDialogBase(Plain, i18n("Lens Distortion Correction"),
                                        Help|User1|Ok|Cancel, Ok,
                                        parent, 0, true, true, i18n("&Reset Values")),
                            m_parent(parent)
{
    QString whatsThis;
    
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to the default values.") );
    m_cancel = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Lens Distortion Correction"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to reduce spherical aberration caused "
                                                 "by a lens to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("David Hodson", I18N_NOOP("Lens distortion correction algorithm."),
                     "hodsond at acm dot org");
                     
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Lens Distortion Correction Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 1, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Lens Distortion Correction"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 2);

    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );

    // -------------------------------------------------------------
        
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* l = new QHBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageGuideWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the spherical aberration correction. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be draw "
                                           "to guide you in adjusting the lens distortion correction. "
                                           "Press the left mouse button to freeze the dashed "
                                           "line's position."));
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 1);
    
    // -------------------------------------------------------------
                                                  
    QGroupBox *gbox2 = new QGroupBox(i18n("Settings"), plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 5, 2, 20, spacingHint());

    m_maskPreviewLabel = new QLabel( gbox2 );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QWhatsThis::add( m_maskPreviewLabel, i18n("<p>You can see here a thumbnail preview of the distortion correction "
                                              "applied to a cross pattern.") );
    gridBox2->addMultiCellWidget(m_maskPreviewLabel, 0, 0, 0, 2);
        
    QLabel *label1 = new QLabel(i18n("Main:"), gbox2);
    m_mainSlider = new QSlider(-1000, 1000, 1, 0, Qt::Horizontal, gbox2, "m_mainSlider");
    m_mainSlider->setTickmarks(QSlider::Below);
    m_mainSlider->setTickInterval(200);
    m_mainSlider->setTracking ( false );
    m_mainSpinBox = new QSpinBox(-1000, 1000, 1, gbox2, "m_mainSpinBox");
    m_mainSpinBox->setValue(0);
        
    whatsThis = i18n("<p>This value controls the amount of distortion. Negative values correct lens barrel "
                     "distortion, while positive values correct lens pincushion distortion.");
    
    QWhatsThis::add( m_mainSpinBox, whatsThis);
    QWhatsThis::add( m_mainSlider, whatsThis);
    
    gridBox2->addMultiCellWidget(label1, 1, 1, 0, 0);
    gridBox2->addMultiCellWidget(m_mainSlider, 1, 1, 1, 1);
    gridBox2->addMultiCellWidget(m_mainSpinBox, 1, 1, 2, 2);
    
    QLabel *label2 = new QLabel(i18n("Edge:"), gbox2);
    m_edgeSlider = new QSlider(-1000, 1000, 1, 0, Qt::Horizontal, gbox2, "m_edgeSlider");
    m_edgeSlider->setTickmarks(QSlider::Below);
    m_edgeSlider->setTickInterval(200);
    m_edgeSlider->setTracking ( false );  
    m_edgeSpinBox = new QSpinBox(-1000, 1000, 1, gbox2, "m_edgeSpinBox");
    m_edgeSpinBox->setValue(0);
        
    whatsThis = i18n("<p>This value controls in the same manner as the Main control, but has more effect "
                     "at the edges of the image than at the center.");
    
    QWhatsThis::add( m_edgeSpinBox, whatsThis);
    QWhatsThis::add( m_edgeSlider, whatsThis);                     
    
    gridBox2->addMultiCellWidget(label2, 2, 2, 0, 0);
    gridBox2->addMultiCellWidget(m_edgeSlider, 2, 2, 1, 1);
    gridBox2->addMultiCellWidget(m_edgeSpinBox, 2, 2, 2, 2);
    
    QLabel *label3 = new QLabel(i18n("Zoom:"), gbox2);
    m_rescaleSlider = new QSlider(-1000, 1000, 1, 0, Qt::Horizontal, gbox2, "m_rescaleSlider");
    m_rescaleSlider->setTickmarks(QSlider::Below);
    m_rescaleSlider->setTickInterval(200);
    m_rescaleSlider->setTracking ( false );  
    m_rescaleSpinBox = new QSpinBox(-1000, 1000, 1, gbox2, "m_rescaleSpinBox");
    m_rescaleSpinBox->setValue(0);
    
    whatsThis = i18n("<p>This value rescales the overall image size.");
    QWhatsThis::add( m_rescaleSpinBox, whatsThis);
    QWhatsThis::add( m_rescaleSlider, whatsThis);                     
    
    gridBox2->addMultiCellWidget(label3, 3, 3, 0, 0);
    gridBox2->addMultiCellWidget(m_rescaleSlider, 3, 3, 1, 1);
    gridBox2->addMultiCellWidget(m_rescaleSpinBox, 3, 3, 2, 2);

    QLabel *label4 = new QLabel(i18n("Brighten:"), gbox2);
    m_brightenSlider = new QSlider(-1000, 1000, 1, 0, Qt::Horizontal, gbox2, "m_brightenSlider");
    m_brightenSlider->setTickmarks(QSlider::Below);
    m_brightenSlider->setTickInterval(200);
    m_brightenSlider->setTracking ( false );  
    
    m_brightenSpinBox = new QSpinBox(-1000, 1000, 1, gbox2, "m_brightenSpinBox");
    m_brightenSpinBox->setValue(0);
    
    whatsThis = i18n("<p>This value adjust the brightness in image corners.");
    QWhatsThis::add( m_brightenSpinBox, whatsThis);
    QWhatsThis::add( m_brightenSlider, whatsThis);                     
    
    gridBox2->addMultiCellWidget(label4, 4, 4, 0, 0);
    gridBox2->addMultiCellWidget(m_brightenSlider, 4, 4, 1, 1);
    gridBox2->addMultiCellWidget(m_brightenSpinBox, 4, 4, 2, 2);
    
    m_progressBar = new KProgress(100, gbox2, "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    gridBox2->addMultiCellWidget(m_progressBar, 5, 5, 0, 2);

    topLayout->addMultiCellWidget(gbox2, 1, 1, 2, 2);
    
    // -------------------------------------------------------------

    adjustSize();
    disableResize();  

    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_mainSlider, SIGNAL(valueChanged(int)),
            m_mainSpinBox, SLOT(setValue(int)));
    connect(m_mainSpinBox, SIGNAL(valueChanged(int)),
            m_mainSlider, SLOT(setValue(int)));            
    connect(m_mainSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));            
            
    connect(m_edgeSlider, SIGNAL(valueChanged(int)),
            m_edgeSpinBox, SLOT(setValue(int)));
    connect(m_edgeSpinBox, SIGNAL(valueChanged(int)),
            m_edgeSlider, SLOT(setValue(int)));   
    connect(m_edgeSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     

    connect(m_rescaleSlider, SIGNAL(valueChanged(int)),
            m_rescaleSpinBox, SLOT(setValue(int)));
    connect(m_rescaleSpinBox, SIGNAL(valueChanged(int)),
            m_rescaleSlider, SLOT(setValue(int)));   
    connect(m_rescaleSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     
    
    connect(m_brightenSlider, SIGNAL(valueChanged(int)),
            m_brightenSpinBox, SLOT(setValue(int)));
    connect(m_brightenSpinBox, SIGNAL(valueChanged(int)),
            m_brightenSlider, SLOT(setValue(int)));   
    connect(m_brightenSpinBox, SIGNAL(valueChanged (int)),
            this, SLOT(slotEffect()));     
}

ImageEffect_LensDistortion::~ImageEffect_LensDistortion()
{
}

void ImageEffect_LensDistortion::slotHelp()
{
    KApplication::kApplication()->invokeHelp("lensdistortion",
                                             "digikamimageplugins");
}

void ImageEffect_LensDistortion::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_LensDistortion::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_LensDistortion::slotUser1()
{
    m_mainSlider->blockSignals(true);
    m_mainSpinBox->blockSignals(true);
    m_edgeSlider->blockSignals(true);
    m_edgeSpinBox->blockSignals(true);
    m_rescaleSlider->blockSignals(true);
    m_rescaleSpinBox->blockSignals(true);
    m_brightenSlider->blockSignals(true);
    m_brightenSpinBox->blockSignals(true);
      
    m_mainSlider->setValue(0);
    m_mainSpinBox->setValue(0);
    m_edgeSlider->setValue(0);
    m_edgeSpinBox->setValue(0);
    m_rescaleSlider->setValue(0);
    m_rescaleSpinBox->setValue(0);
    m_brightenSlider->setValue(0);
    m_brightenSpinBox->setValue(0);
    
    m_mainSlider->blockSignals(false);
    m_mainSpinBox->blockSignals(false);
    m_edgeSlider->blockSignals(false);
    m_edgeSpinBox->blockSignals(false);
    m_rescaleSlider->blockSignals(false);
    m_rescaleSpinBox->blockSignals(false);
    m_brightenSlider->blockSignals(false);
    m_brightenSpinBox->blockSignals(false);

    slotEffect();
} 

void ImageEffect_LensDistortion::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    // All data from the image
    uint* data  = iface->getPreviewData();
    int w       = iface->previewWidth();
    int h       = iface->previewHeight();
    double m    = (double)(m_mainSlider->value() /10.0);
    double e    = (double)(m_edgeSlider->value() / 10.0);
    double r    = (double)(m_rescaleSlider->value() /10.0);
    double b    = (double)(m_brightenSlider->value() /10.0);

    m_progressBar->setValue(0); 

    // Calc preview.    
    QImage preview(120, 120, 32);
    memset(preview.bits(), 255, preview.numBytes());
    QPixmap pix (preview);
    QPainter pt(&pix);
    pt.setPen( QPen::QPen(Qt::black, 1) ); 
    pt.fillRect( 0, 0, pix.width(), pix.height(), QBrush::QBrush(Qt::black, Qt::CrossPattern) );
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    QImage preview2(pix.convertToImage());
    wideangle((uint*)preview2.bits(), preview2.width(), preview2.height(), m, e, r, b, 0, 0, false);
    m_maskPreviewLabel->setPixmap(QPixmap::QPixmap(preview2));

    // Calc image transformations.    
    wideangle(data, w, h, m, e, r, b, 0, 0);
    
    iface->putPreviewData(data);
           
    delete [] data;
    m_progressBar->setValue(0); 
    m_previewWidget->update();
}

void ImageEffect_LensDistortion::slotOk()
{
    m_mainSlider->setEnabled(false);
    m_mainSpinBox->setEnabled(false);
    m_edgeSlider->setEnabled(false);
    m_edgeSpinBox->setEnabled(false);
    m_rescaleSlider->setEnabled(false);
    m_rescaleSpinBox->setEnabled(false);
    m_brightenSlider->setEnabled(false);
    m_brightenSpinBox->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    double m    = (double)(m_mainSlider->value() /10.0);
    double e    = (double)(m_edgeSlider->value() / 10.0);
    double r    = (double)(m_rescaleSlider->value() /10.0);
    double b    = (double)(m_brightenSlider->value() /10.0);

    m_progressBar->setValue(0); 
        
    if (data) 
       {
       wideangle(data, w, h, m, e, r, b, 0, 0);
       if ( !m_cancel ) iface->putOriginalData(i18n("Lens Distortion"), data);
       }
    
    delete [] data;    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

void ImageEffect_LensDistortion::wideangle(uint *data, int Width, int Height, double main, 
                                           double edge, double rescale, double brighten,
                                           int centre_x, int centre_y, bool progress)
{
    m_normallise_radius_sq = 4.0 / (Width * Width + Height * Height);
    m_centre_x             = Width * (100.0 + centre_x) / 200.0;
    m_centre_y             = Height * (100.0 + centre_y) / 200.0;
    m_mult_sq              = main / 200.0;
    m_mult_qd              = edge / 200.0;
    m_rescale              = pow(2.0, - rescale / 100.0);
    m_brighten             = - brighten / 10.0;
    
    PixelAccess *pa = new PixelAccess(data, Width, Height);

    /*
     * start at image (i, j), increment by (step, step)
     * output goes to dst, which is w x h x d in size
     * NB: d <= image.bpp
     */
   
    // We working on the full image.
    int    i = 0, j = 0, dstWidth = Width, dstHeight = Height, dstDepth = 4;
    uchar* dst = (uchar*)data;
    int    step = 1;
  
    int    dstI, dstJ;
    int    iLimit, jLimit;
    double srcX, srcY, mag;

    iLimit = i + dstWidth * step;
    jLimit = j + dstHeight * step;

    for (dstJ = j ; !m_cancel && (dstJ < jLimit) ; dstJ += step) 
       {
       for (dstI = i ; !m_cancel && (dstI < iLimit) ; dstI += step) 
          {
          // Get source Coordinates.
          double radius_sq;
          double off_x;
          double off_y;
          double radius_mult;

          off_x       = dstI - m_centre_x;
          off_y       = dstJ - m_centre_y;
          radius_sq   = (off_x * off_x) + (off_y * off_y);

          radius_sq  *= m_normallise_radius_sq;

          radius_mult = radius_sq * m_mult_sq + radius_sq * radius_sq * m_mult_qd;
          mag         = radius_mult;
          radius_mult = m_rescale * (1.0 + radius_mult);

          srcX        = m_centre_x + radius_mult * off_x;
          srcY        = m_centre_y + radius_mult * off_y;

          brighten = 1.0 + mag * m_brighten;
          pa->pixelAccessGetCubic(srcX, srcY, brighten, dst, dstDepth);
          dst += dstDepth;
          }
     
       // Update progress bar in dialog.
       
       if (progress)
          {
          m_progressBar->setValue((int) (((double)dstJ * 100.0) / jLimit));
          kapp->processEvents(); 
          }
       }
       
    delete pa;
}

}  // NameSpace DigikamLensDistortionImagesPlugin

#include "imageeffect_lensdistortion.moc"
