/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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
 
#include <qcolor.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h> 
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "bcgmodifier.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_redeye.h"
#include "imageeffect_redeye.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_RedEye::ImageEffect_RedEye(QWidget* parent)
                  : Digikam::ImageDlgBase(parent, i18n("Red Eye Reduction"), 
                                          "redeye", false)
{
    m_destinationPreviewData = 0L;
    setHelp("redeyecorrectiontool.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("redeye Tool Dialog", plainPage(),
                                               i18n("<p>Here you can see the image selection preview with "
                                                    "red eye reduction applied."), 
                                               true, Digikam::ImageGuideWidget::PickColorMode, true, true);
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------
                
    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 6, 4, marginHint(), spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gboxSettings );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    m_scaleBG = new QHButtonGroup(gboxSettings);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));
    
    QPushButton *linHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    m_scaleBG->insert(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);
    
    QPushButton *logHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    m_scaleBG->insert(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);       

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(m_scaleBG);
    
    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 4);

    // -------------------------------------------------------------

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, gboxSettings, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram "
                                             "drawing of the selected image channel. This one is "
                                             "re-computed at any settings changes."));
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    gridSettings->addMultiCellWidget(m_histogramWidget, 1, 1, 0, 4);
    gridSettings->addMultiCellWidget(m_hGradient, 2, 2, 0, 4);

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Red Threshold:"), gboxSettings);
    m_redThreshold = new QComboBox( false, gboxSettings );
    m_redThreshold->insertItem( i18n("Mild"), Mild );
    m_redThreshold->insertItem( i18n("Aggressive"), Aggressive );
    QWhatsThis::add( m_redThreshold, i18n("<p>Set here red color threshold to use:</p>"
                     "<b>Mild</b>: use this option if other parts of the face are also selected."
                     "<b>Aggressive</b>: use this option only if eye(s) have been selected exactly."));
    gridSettings->addMultiCellWidget(label2, 3, 3, 0, 4);
    gridSettings->addMultiCellWidget(m_redThreshold, 4, 4, 0, 4);

    QHBox *hbox      = new QHBox(gboxSettings);
    QLabel *label3   = new QLabel(i18n("Coloring Taint:"), hbox);
    m_coloringButton = new KColorButton(Qt::black, hbox);
    gridSettings->addMultiCellWidget(hbox, 5, 5, 0, 4);

    gridSettings->setRowStretch(6, 10);    
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));    
            
    connect(m_redThreshold, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));   

    connect(m_coloringButton, SIGNAL(changed (const QColor&)),
            this, SLOT(slotEffect())); 
}

ImageEffect_RedEye::~ImageEffect_RedEye()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_RedEye::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case LuminosityChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            break;
    
        case RedChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            break;
    
        case GreenChannel:         
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            break;
    
        case BlueChannel:         
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            break;
    }

    m_histogramWidget->repaint(false);
}

void ImageEffect_RedEye::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_RedEye::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_RedEye::readUserSettings()
{
    QColor black(Qt::black);
    KConfig* config = kapp->config();
    config->setGroup("redeye Tool Dialog");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));
    m_redThreshold->setCurrentItem(config->readNumEntry("RedThreshold", Mild));
    m_coloringButton->setColor(config->readColorEntry("ColoringTaint", &black));
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void ImageEffect_RedEye::writeUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("redeye Tool Dialog");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("RedThreshold", m_redThreshold->currentItem());
    config->writeEntry("ColoringTaint", m_coloringButton->color());
    config->sync();
}

void ImageEffect_RedEye::resetValues()
{
    QColor black(Qt::black);
    m_redThreshold->blockSignals(true);	
    m_coloringButton->blockSignals(true);	
    m_redThreshold->setCurrentItem(Mild);
    m_coloringButton->setColor(black);
    m_redThreshold->blockSignals(false); 
    m_coloringButton->blockSignals(false);       
} 

void ImageEffect_RedEye::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg selection(w, h, sb, a, m_destinationPreviewData);

    redEyeFilter(selection);

    iface->putPreviewImage(selection.bits());
    m_previewWidget->updatePreview();

    // Update histogram.
   
    memcpy(m_destinationPreviewData, selection.bits(), selection.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_RedEye::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getImageSelection();
    int w                      = iface->selectedWidth();
    int h                      = iface->selectedHeight();
    bool sixteenBit            = iface->originalSixteenBit();
    bool hasAlpha              = iface->originalHasAlpha();
    Digikam::DImg selection(w, h, sixteenBit, hasAlpha, data);
    delete [] data;

    redEyeFilter(selection);

    iface->putImageSelection(i18n("Red Eyes Correction"), selection.bits());

    kapp->restoreOverrideCursor();
    accept();
}

void ImageEffect_RedEye::redEyeFilter(Digikam::DImg& selection)
{
    Digikam::DImg newSelection = selection.copy();

    bool aggressive = (m_redThreshold->currentItem() == Mild) ? false : true;
    QColor coloring = m_coloringButton->color();

    struct channel
    {
        float red_gain;
        float green_gain;
        float blue_gain;
    };

    channel red_chan, green_chan, blue_chan;

    red_chan.red_gain     = 0.1;
    red_chan.green_gain   = 0.6;
    red_chan.blue_gain    = 0.3;

    green_chan.red_gain   = 0.0;
    green_chan.green_gain = 1.0;
    green_chan.blue_gain  = 0.0;

    blue_chan.red_gain    = 0.0;
    blue_chan.green_gain  = 0.0;
    blue_chan.blue_gain   = 1.0;

    float red_norm, green_norm, blue_norm;
    int   level = 64;

    red_norm   = 1.0 / (red_chan.red_gain   + red_chan.green_gain   + red_chan.blue_gain);
    green_norm = 1.0 / (green_chan.red_gain + green_chan.green_gain + green_chan.blue_gain);
    blue_norm  = 1.0 / (blue_chan.red_gain  + blue_chan.green_gain  + blue_chan.blue_gain);

    red_norm   *= coloring.red()   / level;
    green_norm *= coloring.green() / level;
    blue_norm  *= coloring.blue()  / level;

    if (!selection.sixteenBit())         // 8 bits image.
    {
        uchar* ptr  = selection.bits();
        uchar* nptr = newSelection.bits();
        uchar  r, g, b, a, r1, g1, b1;

        for (uint i = 0 ; i < selection.width() * selection.height() ; i++) 
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if ( aggressive || r >= ( 2 * g) )
            {
                r1 = QMIN(255, (int)(red_norm * (red_chan.red_gain   * r +
                                                 red_chan.green_gain * g +
                                                 red_chan.blue_gain  * b)));

                g1 = QMIN(255, (int)(green_norm * (green_chan.red_gain   * r +
                                                   green_chan.green_gain * g +
                                                   green_chan.blue_gain  * b)));

                b1 = QMIN(255, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                  blue_chan.green_gain * g +
                                                  blue_chan.blue_gain  * b)));

                nptr[0] = b1;
                nptr[1] = g1;
                nptr[2] = r1;
                nptr[3] = QMIN( (int)((r-g) / 150.0 * 255.0), 255);
            }

            ptr += 4;
            nptr+= 4;
        }
    }
    else                                 // 16 bits image.
    {
        unsigned short* ptr  = (unsigned short*)selection.bits();
        unsigned short* nptr = (unsigned short*)newSelection.bits();
        unsigned short  r, g, b, a, r1, g1, b1;

        for (uint i = 0 ; i < selection.width() * selection.height() ; i++) 
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if ( aggressive || r >= ( 2 * g) )
            {
                r1 = QMIN(65535, (int)(red_norm * 256 * (red_chan.red_gain   * r +
                                                         red_chan.green_gain * g +
                                                         red_chan.blue_gain  * b)));

                g1 = QMIN(65535, (int)(green_norm *  256 * (green_chan.red_gain   * r +
                                                            green_chan.green_gain * g +
                                                            green_chan.blue_gain  * b)));

                b1 = QMIN(65535, (int)(blue_norm * 256 * (blue_chan.red_gain   * r +
                                                          blue_chan.green_gain * g +
                                                          blue_chan.blue_gain  * b)));

                nptr[0] = b1;
                nptr[1] = g1;
                nptr[2] = r1;
                nptr[3] = QMIN( (int)((r-g) / 38400.0 * 65535.0), 65535);
            }

            ptr += 4;
            nptr+= 4;
        }
    }

    // - Perform pixels blending using alpha channel to blur a little the result.

    selection.bitBlend_RGBA2RGB(newSelection, 0, 0, selection.width(), selection.height());
}

}  // NameSpace DigikamImagesPluginCore

