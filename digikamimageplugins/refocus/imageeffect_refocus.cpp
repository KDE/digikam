/* ============================================================
 * File  : imageeffect_refocus.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-29
 * Description : a digiKam image editor plugin to refocus 
 *               an image.
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
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qrect.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "matrix.h"
#include "version.h"
#include "imageeffect_refocus.h"

#define MAX_MATRIX_SIZE 25

namespace DigikamRefocusImagesPlugin
{

ImageEffect_Refocus::ImageEffect_Refocus(QWidget* parent)
                   : KDialogBase(Plain, i18n("Photograph Refocus"),
                                 Help|User1|User2|User3|Ok|Cancel, Ok,
                                 parent, 0, true, true,
                                 i18n("&Reset Values"),
                                 i18n("&Load..."),
                                 i18n("&Save As...")),
                     m_parent(parent)
{
    m_timer = 0;
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User2, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User3, i18n("<p>Save all filter parameters to settings text file.") );    
    m_cancel = false;
    m_dirty  = false;   
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Photograph Refocus"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to refocus a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Ernst Lippe", I18N_NOOP("FIR Wiener deconvolution filter algorithm"), 
                     "ernstl@users.sourceforge.net");
                     
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Photograph Refocus Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Refocus a Photograph"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, i18n("Preview"), plainPage(), true);
    hlay1->addWidget(m_imagePreviewWidget);
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    QGridLayout* grid = new QGridLayout( topLayout, 3 , 4, spacingHint());
    
    QLabel *label2 = new QLabel(i18n("Circular Sharpness:"), plainPage());
    m_radius = new KDoubleNumInput(plainPage());
    m_radius->setPrecision(2);
    m_radius->setRange(0.0, 5.0, 0.001, true);
    QWhatsThis::add( m_radius, i18n("<p>This is the Radius of the Circular convolution. It is the most important "
                                    "parameter for using the plugin. For most images the default value of 0.9 "
                                    "should give good results. Select a higher value when your image is very blurred."));
    
    grid->addMultiCellWidget(label2, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_radius, 0, 0, 1, 1);
    
        
    
    QLabel *label4 = new QLabel(i18n("Correlation:"), plainPage());
    m_correlation = new KDoubleNumInput(plainPage());
    m_correlation->setPrecision(3);
    m_correlation->setRange(0.0, 1.0, 0.001, true);
    QWhatsThis::add( m_correlation, i18n("<p>Increasing the Correlation may help reducing artifacts. The correlation can "
                                         "range  from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                         "Using a high value for the Correlation will reduce the sharpening effect of the "
                                         "plug-in."));

    grid->addMultiCellWidget(label4, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_correlation, 1, 1, 1, 1);
    
    QLabel *label5 = new QLabel(i18n("Noise Filter:"), plainPage());
    m_noise = new KDoubleNumInput(plainPage());
    m_noise->setPrecision(3);
    m_noise->setRange(0.0, 1.0, 0.001, true);
    QWhatsThis::add( m_noise, i18n("<p>Increasing the Noise Filter parameter may help reducing artifacts. The Noise Filter "
                                   "can range from 0-1 but values higher than 0.1 are rarely helpful. When the Noise Filter "
                                   "value is too low, e.g. 0.0 the image quality will be horrible. A useful value is 0.01. "
                                   "Using a high value for the Noise Filter will reduce the sharpening "
                                   "effect of the plug-in."));

    grid->addMultiCellWidget(label5, 2, 2, 0, 0);
    grid->addMultiCellWidget(m_noise, 2, 2, 1, 1);
    
    QLabel *label3 = new QLabel(i18n("Gaussian Sharpness:"), plainPage());
    m_gauss = new KDoubleNumInput(plainPage());
    m_gauss->setPrecision(2);
    m_gauss->setRange(0.0, 1.0, 0.001, true);
    QWhatsThis::add( m_gauss, i18n("<p>This is the Sharpness for the Gaussian convolution. Use this parameter when your "
                                   "blurring is of Gaussian type. In most cases you should set this parameter to 0, because "
                                   "it causes nasty artifacts. When you use non-zero values you will probably have to "
                                   "increase the Correlation and/or Noise Filter parameters, too."));

    grid->addMultiCellWidget(label3, 0, 0, 3, 3);
    grid->addMultiCellWidget(m_gauss, 0, 0, 4, 4);

    
    QLabel *label1 = new QLabel(i18n("Matrix Size:"), plainPage());
    m_matrixSize = new KIntNumInput(plainPage());
    m_matrixSize->setRange(0, MAX_MATRIX_SIZE, 1, true);  
    QWhatsThis::add( m_matrixSize, i18n("<p>This parameter determines the size of the transformation matrix. "
                                        "Increasing the Matrix Width may give better results, especially when you have "
                                        "chosen large values for Circular or Gaussian Sharpness."));

    grid->addMultiCellWidget(label1, 1, 1, 3, 3);
    grid->addMultiCellWidget(m_matrixSize, 1, 1, 4, 4);
    
    // -------------------------------------------------------------
    
    adjustSize();
    disableResize(); 
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_matrixSize, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));                        
    
    connect(m_radius, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_gauss, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_correlation, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    connect(m_noise, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));                        

    // -------------------------------------------------------------
    
    // Image creation with dummy borders (mosaic mode). It needs to do it before to apply deconvolution filter 
    // on original image border pixels including on matrix size area. This way limit artefacts on image border.
    
    Digikam::ImageIface iface(0, 0);
        
    uint*  data = iface.getOriginalData();
    int    w    = iface.originalWidth();
    int    h    = iface.originalHeight();
    
    m_img.create( w + 4*MAX_MATRIX_SIZE, h + 4*MAX_MATRIX_SIZE, 32 );
    
    QImage tmp;
    QImage org(w, h, 32);
    memcpy(org.bits(), data, org.numBytes());            
    bitBlt(&m_img, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, &org, 0, 0, w, h);
    
    // Create dummy top border
    tmp = org.copy(0, 0, w, 2*MAX_MATRIX_SIZE).mirror(false, true);
    bitBlt(&m_img, 2*MAX_MATRIX_SIZE, 0, &tmp, 0, 0, w, 2*MAX_MATRIX_SIZE);
    
    // Create dummy bottom border
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, w, 2*MAX_MATRIX_SIZE).mirror(false, true);
    bitBlt(&m_img, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE+h, &tmp, 0, 0, w, 2*MAX_MATRIX_SIZE);

    // Create dummy right border
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, h).mirror(true, false);
    bitBlt(&m_img, 0, 2*MAX_MATRIX_SIZE, &tmp, 0, 0, 2*MAX_MATRIX_SIZE, h);
    
    // Create dummy left border
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, h).mirror(true, false);
    bitBlt(&m_img, w+2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, &tmp, 0, 0, 2*MAX_MATRIX_SIZE, h);
    
    // Create dummy top/left corner
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE).mirror(true, true);
    bitBlt(&m_img, 0, 0, &tmp, 0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);

    // Create dummy top/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE).mirror(true, true);
    bitBlt(&m_img, w+2*MAX_MATRIX_SIZE, 0, &tmp, 0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);

    // Create dummy bottom/left corner
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE).mirror(true, true);
    bitBlt(&m_img, 0, h+2*MAX_MATRIX_SIZE, &tmp, 0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);

    // Create dummy bottom/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE).mirror(true, true);
    bitBlt(&m_img, w+2*MAX_MATRIX_SIZE, h+2*MAX_MATRIX_SIZE, &tmp, 0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
}

ImageEffect_Refocus::~ImageEffect_Refocus()
{
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Refocus::abortPreview()
{
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_matrixSize->setEnabled(true);
    m_radius->setEnabled(true);
    m_gauss->setEnabled(true);
    m_correlation->setEnabled(true);
    m_noise->setEnabled(true);
    m_imagePreviewWidget->setEnable(true);    
    enableButton(Ok, true);  
    enableButton(User2, true);
    enableButton(User3, true);  
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all filter parameters to their default values.") );
 }

void ImageEffect_Refocus::slotUser1()
{
    if (m_dirty)
       {
       m_cancel = true;
       abortPreview();
       }
    else
       {
       m_matrixSize->blockSignals(true);
       m_radius->blockSignals(true);
       m_gauss->blockSignals(true);
       m_correlation->blockSignals(true);
       m_noise->blockSignals(true);
       
       m_matrixSize->setValue(5);
       m_radius->setValue(0.9);
       m_gauss->setValue(0.0);
       m_correlation->setValue(0.5);
       m_noise->setValue(0.01);
        
       m_matrixSize->blockSignals(false);
       m_radius->blockSignals(false);
       m_gauss->blockSignals(false);
       m_correlation->blockSignals(false);
       m_noise->blockSignals(false);
       slotEffect();    
       }
} 

void ImageEffect_Refocus::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_Refocus::slotHelp()
{
    KApplication::kApplication()->invokeHelp("refocus", "digikamimageplugins");
}

void ImageEffect_Refocus::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_Refocus::slotTimer()
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

void ImageEffect_Refocus::slotEffect()
{
    if (m_dirty) return;     // Computation already in process.
    
    m_cancel = false;
    m_dirty = true;
    m_matrixSize->setEnabled(false);
    m_radius->setEnabled(false);
    m_gauss->setEnabled(false);
    m_correlation->setEnabled(false);
    m_noise->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    enableButton(User2, false);
    enableButton(User3, false);
    
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    int    ms     = m_matrixSize->value();
    double r      = m_radius->value();
    double g      = m_gauss->value();
    double c      = m_correlation->value();
    double n      = m_noise->value();

    QRect  area   = m_imagePreviewWidget->getOriginalImageRegion();
    QRect tmpRect;
    tmpRect.setLeft(area.left()-2*ms);
    tmpRect.setRight(area.right()+2*ms);
    tmpRect.setTop(area.top()-2*ms);
    tmpRect.setBottom(area.bottom()+2*ms);
    tmpRect.moveBy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    QImage imTemp = m_img.copy(tmpRect);
        
    m_imagePreviewWidget->setProgress(0);
    refocus((uint *)imTemp.bits(), imTemp.width(), imTemp.height(), ms, r, g, c, n);
    if (m_cancel) return;
    QImage imDest = imTemp.copy(2*ms, 2*ms, area.width(), area.height());
    m_imagePreviewWidget->setPreviewImageData(imDest);
    
    abortPreview();
    m_dirty = false;
}

void ImageEffect_Refocus::slotOk()
{
    m_matrixSize->setEnabled(false);
    m_radius->setEnabled(false);
    m_gauss->setEnabled(false);
    m_correlation->setEnabled(false);
    m_noise->setEnabled(false);
    m_imagePreviewWidget->setEnable(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    m_parent->setCursor( KCursor::waitCursor() );
        
    Digikam::ImageIface iface(0, 0);
    
    uint*  data = (uint*)m_img.bits();
    int    w    = m_img.width();
    int    h    = m_img.height();
    int    ms   = m_matrixSize->value();
    double r    = m_radius->value();
    double g    = m_gauss->value();
    double c    = m_correlation->value();
    double n    = m_noise->value();
    
    m_imagePreviewWidget->setProgress(0);
    refocus(data, w, h, ms, r, g, c, n);

    if ( !m_cancel )
       {
       iface.putOriginalData(i18n("Refocus"), 
             (uint*)m_img.copy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 
                               iface.originalWidth(), iface.originalHeight())
                         .bits());
       }
       
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

void ImageEffect_Refocus::slotUser2()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Refocus Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());
    
    if ( file.open(IO_ReadOnly) )   
        {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Refocus Configuration File" )
           {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Refocus settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
           }
        
        blockSignals(true);
        m_matrixSize->setValue( stream.readLine().toInt() );
        m_radius->setValue( stream.readLine().toDouble() );
        m_gauss->setValue( stream.readLine().toDouble() );
        m_correlation->setValue( stream.readLine().toDouble() );
        m_noise->setValue( stream.readLine().toDouble() );
        blockSignals(false);
        slotEffect();  
        }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Refocus text file."));

    file.close();             
}

void ImageEffect_Refocus::slotUser3()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Refocus Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());
    
    if ( file.open(IO_WriteOnly) )   
        {
        QTextStream stream( &file );        
        stream << "# Photograph Refocus Configuration File\n";    
        stream << m_matrixSize->value() << "\n";    
        stream << m_radius->value() << "\n";    
        stream << m_gauss->value() << "\n";    
        stream << m_correlation->value() << "\n";    
        stream << m_noise->value() << "\n";    
        }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Refocus text file."));
    
    file.close();        
}

void ImageEffect_Refocus::refocus(uint* data, int width, int height, int matrixSize, 
                                  double radius, double gauss, double correlation, double noise)
{
    CMat *matrix=0;
    
    uint* newData = new uint[width*height];
    
    // Compute matrix
    
    CMat circle, gaussian, convolution;
    
    RefocusMatrix::make_gaussian_convolution (gauss, &gaussian, matrixSize);
    RefocusMatrix::make_circle_convolution (radius, &circle, matrixSize);
    RefocusMatrix::init_c_mat (&convolution, matrixSize);
    RefocusMatrix::convolve_star_mat (&convolution, &gaussian, &circle);
    
    matrix = RefocusMatrix::compute_g_matrix (&convolution, matrixSize, correlation, noise, 0.0, true);
    
    RefocusMatrix::finish_c_mat (&convolution);
    RefocusMatrix::finish_c_mat (&gaussian);
    RefocusMatrix::finish_c_mat (&circle);

    // Apply deconvolution kernel to image.
    convolve_image(data, newData, width, height, matrix->data, 2 * matrixSize + 1);
    
    if (!m_cancel)
       memcpy(data, newData, width*height*4);  

    delete matrix;   
    delete [] newData;
}

void ImageEffect_Refocus::convolve_image(const uint *orgData, uint *destData, int width, int height, 
                                         const double *const mat, int mat_size)
{
    double    matrix[mat_size][mat_size];
    double    valRed, valGreen, valBlue;
    int       x1, y1, x2, y2, index1, index2;
    
    // Big/Little Endian color manipulation compatibility.
    int red, green, blue;
    Digikam::ImageFilters::imageData imagedata;
    
    const int imageSize      = width*height;
    const int mat_offset     = mat_size / 2;
    
    memcpy (&matrix, mat, mat_size* mat_size * sizeof(double));
    
    for (y1 = 0; !m_cancel && (y1 < height); y1++)
        {
        for (x1 = 0; !m_cancel && (x1 < width); x1++)
            {
            valRed = valGreen = valBlue = 0.0;
                
            for (y2 = 0; !m_cancel && (y2 < mat_size); y2++)
                {
                for (x2 = 0; !m_cancel && (x2 < mat_size); x2++)
                    {
                    index1 = width * (y1 + y2 - mat_offset) + 
                             x1 + x2 - mat_offset;
                        
                    if ( index1 >= 0 && index1 < imageSize )
                       {
                       imagedata.raw = orgData[index1];
                       red           = (int)imagedata.channel.red;
                       green         = (int)imagedata.channel.green;
                       blue          = (int)imagedata.channel.blue;
                       valRed   += matrix[y2][x2] * red;
                       valGreen += matrix[y2][x2] * green;
                       valBlue  += matrix[y2][x2] * blue;
                       }
                    }
                }
            
            index2 = y1 * width + x1;
                          
            if (index2 >= 0 && index2 < imageSize)
                {
                // To get Alpha channel value from original (unchanged)
                imagedata.raw = orgData[index2];
                
                // Overwrite RGB values to destination.
                imagedata.channel.red   = (uchar) CLAMP (valRed, 0, 255);
                imagedata.channel.green = (uchar) CLAMP (valGreen, 0, 255);
                imagedata.channel.blue  = (uchar) CLAMP (valBlue, 0, 255);
                destData[index2]        = imagedata.raw;
                }
            }
        
        m_imagePreviewWidget->setProgress((int) (((double)y1 * 100.0) / height));
        kapp->processEvents();
        }
}

}  // NameSpace DigikamRefocusImagesPlugin

#include "imageeffect_refocus.moc"
