/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-06
 * Description : Black and White conversion tool.
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h> 
#include <qlistbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <knuminput.h>
#include <ktabwidget.h>
#include <kconfig.h>

// Digikam includes.

#include "imageiface.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "dimgimagefilters.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "curveswidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "bcgmodifier.h"

// Local includes.

#include "imageeffect_bwsepia.h"

namespace DigikamImagesPluginCore
{

class ListBoxWhatsThis : public QWhatsThis 
{

public:

    ListBoxWhatsThis(QListBox* w) : QWhatsThis(w), m_listBox(w) {}
    virtual QString text (const QPoint &);
    void add(QListBoxItem*, const QString& text);

protected:

    QMap<QListBoxItem*, QString>  m_itemWhatsThisMap;
    QListBox                     *m_listBox;
};

QString ListBoxWhatsThis::text(const QPoint &p)
{
    QListBoxItem* item = m_listBox->itemAt(p);

    if (item != 0) 
        return m_itemWhatsThisMap[item];

    return QString::null;
}

void ListBoxWhatsThis::add(QListBoxItem* item, const QString& text)
{
    m_itemWhatsThisMap[item] = text;
}

class ListBoxPreviewItem : public QListBoxPixmap
{
public:
    ListBoxPreviewItem(QListBox *listbox, const QPixmap &pix, const QString &text)
        : QListBoxPixmap(listbox, pix, text) {};

    virtual int height ( const QListBox * lb ) const;
    virtual int width ( const QListBox * lb ) const;
};

int ListBoxPreviewItem::height(const QListBox *lb) const
{
    int height = QListBoxPixmap::height(lb);
    return QMAX(height, pixmap()->height() + 5);
}

int ListBoxPreviewItem::width(const QListBox *lb) const
{
    int width = QListBoxPixmap::width(lb);
    return QMAX(width, pixmap()->width() + 5);
}

// -----------------------------------------------------------------------------------

ImageEffect_BWSepia::ImageEffect_BWSepia(QWidget* parent)
                   : Digikam::ImageDlgBase(parent, i18n("Convert to Black & White"), 
                                           "convertbw", false),
                     m_destinationPreviewData(0L)
{
    setHelp("blackandwhitetool.anchor", "digikam");

    Digikam::ImageIface iface(0, 0);
    m_originalImage  = iface.getOriginalImg();
    m_thumbnailImage = m_originalImage->smoothScale(128, 128, QSize::ScaleMin);
    m_curves         = new Digikam::ImageCurves(m_originalImage->sixteenBit());

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget(plainPage(),
                                               i18n("<p>Here you can see the black and white conversion tool preview. "
                                                    "You can pick color on image "
                                                    "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------
        
    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 4, 4, marginHint(), spacingHint());

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
    l1->addWidget(m_scaleBG);
    l1->addStretch(10);
    
    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 4);

    // -------------------------------------------------------------

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, gboxSettings, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing of the "
                                             "selected image channel. This one is re-computed at any "
                                             "settings changes."));
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    gridSettings->addMultiCellWidget(m_histogramWidget, 1, 1, 0, 4);
    gridSettings->addMultiCellWidget(m_hGradient, 2, 2, 0, 4);

    // -------------------------------------------------------------

    m_tab = new KTabWidget(gboxSettings);

    m_bwTools = new QListBox(m_tab);
    m_bwTools->setColumnMode(1);
    m_bwTools->setVariableWidth(false);
    m_bwTools->setVariableHeight(false);
    ListBoxWhatsThis* whatsThis = new ListBoxWhatsThis(m_bwTools);

    int type = 0;
    QPixmap pix = getThumbnailForEffect(type);
    
    QListBoxItem *item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Neutral"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Neutral Black & White</b>:"
                                   "<p>Simulate black and white neutral film exposure.</p>")
                                   .arg(previewEffectPic("neutralbw")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Green Filter"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Green Filter</b>:"
                                 "<p>Simulate black and white film exposure using green filter. "
                                 "This provides an universal asset for all scenics, especially suited for portraits "
                                 "photographed against sky.</p>").arg(previewEffectPic("bwgreen")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Orange Filter"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Orange Filter</b>:"
                                  "<p>Simulate black and white film exposure using orange filter. "
                                  "This will enhances landscapes, marine scenes and aerial "
                                  "photography.</p>").arg(previewEffectPic("bworange")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Red Filter"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Red Filter</b>:"
                               "<p>Simulate black and white film exposure using red filter. "
                               "Using this one creates dramatic sky effects and simulates moonlight scenes "
                               "in daytime.</p>").arg(previewEffectPic("bwred")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Yellow Filter"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Yellow Filter</b>:"
                                  "<p>Simulate black and white film exposure using yellow filter. "
                                  "Most natural tonal correction and improves contrast. Ideal for "
                                  "landscapes.</p>").arg(previewEffectPic("bwyellow")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Sepia Tone"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Sepia Tone</b>:"
                                 "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                                 "the shadows-very similar to the process of bleaching a print and re-developing in a sepia "
                                 "toner.</p>").arg(previewEffectPic("sepia")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Brown Tone"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Brown Tone</b>:"
                                 "<p>This filter is more neutral than Sepia Tone filter.</p>").arg(previewEffectPic("browntone")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Cold Tone"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Cold Tone</b>:"
                                "<p>Start subtle and replicate printing on a cold tone black and white "
                                "paper such as a bromide enlarging paper.</p>").arg(previewEffectPic("coldtone")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Selenium Tone"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Selenium Tone</b>:"
                                    "<p>This effect replicate traditional selenium chemical toning done "
                                    "in the darkroom.</p>").arg(previewEffectPic("selenium")));

    ++type;
    pix = getThumbnailForEffect(type);
    item = new ListBoxPreviewItem(m_bwTools, pix, i18n("Platinum Tone"));
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Platinum Tone</b>:"
                                     "<p>This effect replicate traditional platinum chemical toning done "
                                     "in the darkroom.</p>").arg(previewEffectPic("platinum")));
    
    m_tab->insertTab(m_bwTools, i18n("Tone"), ToneTab);

    // -------------------------------------------------------------
    
    QWidget* tab2 = new QWidget( m_tab );
    QGridLayout* gridTab2 = new QGridLayout( tab2, 4, 1, marginHint(), spacingHint());

    Digikam::ColorGradientWidget* vGradient = new Digikam::ColorGradientWidget(
                                                  Digikam::ColorGradientWidget::Vertical,
                                                  10, tab2 );
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );
    gridTab2->addMultiCellWidget(vGradient, 0, 0, 0, 0);
    
    m_curvesWidget = new Digikam::CurvesWidget(256, 256, m_originalImage->bits(), m_originalImage->width(),
                                               m_originalImage->height(), m_originalImage->sixteenBit(),
                                               m_curves, tab2);
    QWhatsThis::add( m_curvesWidget, i18n("<p>This is the curve adjustment of the image luminosity"));
    gridTab2->addMultiCellWidget(m_curvesWidget, 0, 0, 1, 1);

    Digikam::ColorGradientWidget *hGradient = new Digikam::ColorGradientWidget(
                                                  Digikam::ColorGradientWidget::Horizontal,
                                                  10, tab2 );
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    gridTab2->addMultiCellWidget(hGradient, 1, 1, 1, 1);
    
    m_cInput       = new KDoubleNumInput(tab2);
    m_cInput->setLabel(i18n("Contrast:"), AlignLeft | AlignVCenter);
    m_cInput->setPrecision(2);
    m_cInput->setRange(-1.0, 1.0, 0.01, true);
    m_cInput->setValue(0.0);
    QWhatsThis::add( m_cInput, i18n("<p>Set here the contrast adjustment of the image."));
    gridTab2->addMultiCellWidget(m_cInput, 2, 2, 0, 1);

    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), tab2);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels "
                                                      "from the target image preview will be over-colored. "
                                                      "This will not have an effect on the final rendering."));
    gridTab2->addMultiCellWidget(m_overExposureIndicatorBox, 3, 3, 0, 1);

    gridTab2->setRowStretch(4, 10);
    
    m_tab->insertTab(tab2, i18n("Lightness"), LuminosityTab);

    // -------------------------------------------------------------

    m_bwTools->setFocus();
    gridSettings->addMultiCellWidget(m_tab, 3, 3, 0, 4);
    gridSettings->setRowStretch(3, 10);
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Black and White Convertion Tool");
    m_tab->setCurrentPage(config->readNumEntry("Settings Tab", ToneTab));
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));

    // Reset all parameters to the default values.
    QTimer::singleShot(0, this, SLOT(slotDefault()));
    
    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor & )));
    
    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_bwTools, SIGNAL(highlighted(int)),
            this, SLOT(slotEffect()));

    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));
    
    connect(m_cInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));
                     
    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));                       
    
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
}

ImageEffect_BWSepia::~ImageEffect_BWSepia()
{
    m_histogramWidget->stopHistogramComputation();

    KConfig* config = kapp->config();
    config->setGroup("Black and White Convertion Tool");
    config->writeEntry("Settings Tab", m_tab->currentPageIndex());
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->sync();

    delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
    delete m_curvesWidget;
    delete m_curves;
}

QPixmap ImageEffect_BWSepia::getThumbnailForEffect(int type) 
{
    Digikam::DImg thumb = m_thumbnailImage.copy();
    blackAndWhiteConversion(thumb.bits(), thumb.width(), thumb.height(), thumb.sixteenBit(), type);
    return (thumb.convertToPixmap());
}

void ImageEffect_BWSepia::slotChannelChanged(int channel)
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

void ImageEffect_BWSepia::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
    m_curvesWidget->m_scaleType = scale;
    m_curvesWidget->repaint(false);
}

void ImageEffect_BWSepia::slotSpotColorChanged(const Digikam::DColor &color)
{
    m_curvesWidget->setCurveGuide(color);
}

void ImageEffect_BWSepia::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

QString ImageEffect_BWSepia::previewEffectPic(QString name)
{
    KGlobal::dirs()->addResourceType(name.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return ( KGlobal::dirs()->findResourceDir(name.ascii(), name + ".png") + name + ".png" );
}

void ImageEffect_BWSepia::slotDefault()
{
    m_bwTools->blockSignals(true);
    m_cInput->blockSignals(true);

    m_bwTools->setCurrentItem(0);
    m_bwTools->setSelected(0, true);
    m_cInput->setValue(0.0);
    
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curves->curvesChannelReset(channel);

    m_curvesWidget->reset();
    
    m_cInput->blockSignals(false);
    m_bwTools->blockSignals(false);

    slotEffect();
}

void ImageEffect_BWSepia::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    m_histogramWidget->stopHistogramComputation();

    delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface      = m_previewWidget->imageIface();
    m_destinationPreviewData        = iface->getPreviewImage();
    int w                           = iface->previewWidth();
    int h                           = iface->previewHeight();
    bool a                          = iface->previewHasAlpha();
    bool sb                         = iface->previewSixteenBit();

    // Convert to black and white.

    blackAndWhiteConversion(m_destinationPreviewData, w, h, sb, m_bwTools->currentItem());

    // Calculate and apply the curve on image.
    
    uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel, m_overExposureIndicatorBox->isChecked());
    m_curves->curvesLutProcess(m_destinationPreviewData, targetData, w, h);

    // Adjust contrast.
    
    Digikam::DImg preview(w, h, sb, a, targetData);
    Digikam::BCGModifier cmod;
    cmod.setContrast(m_cInput->value() + (double)(1.00));
    cmod.applyBCG(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
    
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] targetData;
    
    kapp->restoreOverrideCursor();
}

void ImageEffect_BWSepia::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();
    
    if (data) 
    {
       int type = m_bwTools->currentItem();

       // Convert to black and white.
    
       blackAndWhiteConversion(data, w, h, sb, type);
       
       // Calculate and apply the curve on image.

       uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
       m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
       m_curves->curvesLutProcess(data, targetData, w, h);
       
       // Adjust contrast.
          
       Digikam::DImg img(w, h, sb, a, targetData);
       Digikam::BCGModifier cmod;
       cmod.setContrast(m_cInput->value() + (double)(1.00));
       cmod.applyBCG(img);
       
       QString name;
       
       switch (type)
       {
          case BWNeutral:
             name = i18n("Neutral Black && White");
          break;

          case BWGreenFilter:
             name = i18n("Black && White With Green Filter");
          break;

          case BWOrangeFilter:
             name = i18n("Black && White With Orange Filter");
          break;

          case BWRedFilter:
             name = i18n("Black && White With Red Filter");
          break;

          case BWYellowFilter:
             name = i18n("Black && White With Yellow Filter");
          break;

          case BWSepia:
             name = i18n("Black && White Sepia");
          break;

          case BWBrown:
             name = i18n("Black && White Brown");
          break;

          case BWCold:
             name = i18n("Black && White Cold");
          break;

          case BWSelenium:
             name = i18n("Black && White Selenium");
          break;
          
          case BWPlatinum:
             name = i18n("Black && White Platinum");
          break;
       }
          
       iface->putOriginalImage(name, img.bits());
       delete [] data;
       delete [] targetData;
    }

    kapp->restoreOverrideCursor();
    accept();
}

// This method is based on the Convert to Black & White tutorial (channel mixer method) 
// from GimpGuru.org web site available at this url : http://www.gimpguru.org/Tutorials/Color2BW/

void ImageEffect_BWSepia::blackAndWhiteConversion(uchar *data, int w, int h, bool sb, int type)
{
    // Value to multiply RGB 8 bits component of mask used by changeTonality() method.
    int mul = sb ? 255 : 1;
    Digikam::DImgImageFilters filter;
    
    switch (type)
    {
       case BWNeutral:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.3, 0.59 , 0.11,                                // Red channel gains.
                   0.0, 1.0,   0.0,                                 // Green channel gains (not used).
                   0.0, 0.0,   1.0);                                // Blue channel gains (not used).
          break;
       
       case BWGreenFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.1, 0.7, 0.2,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWOrangeFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.78, 0.22, 0.0,                                 // Red channel gains.
                   0.0,  1.0,  0.0,                                 // Green channel gains (not used).
                   0.0,  0.0,  1.0);                                // Blue channel gains (not used).
          break;
       
       case BWRedFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.9, 0.1, 0.0,                                   // Red channel gains.
                   0.0, 1.0, 0.0,                                   // Green channel gains (not used).
                   0.0, 0.0, 1.0);                                  // Blue channel gains (not used).
          break;
       
       case BWYellowFilter:
          filter.channelMixerImage(data, w, h, sb,  // Image data.
                   true,                                            // Preserve luminosity.
                   true,                                            // Monochrome.
                   0.6, 0.28, 0.12,                                 // Red channel gains.
                   0.0, 1.0,  0.0,                                  // Green channel gains (not used).
                   0.0, 0.0,  1.0);                                 // Blue channel gains (not used).
          break;
       
       case BWSepia:
          filter.changeTonality(data, w, h, sb, 162*mul, 132*mul, 101*mul);
          break;
       
       case BWBrown:
          filter.changeTonality(data, w, h, sb, 129*mul, 115*mul, 104*mul);
          break;
       
       case BWCold:
          filter.changeTonality(data, w, h, sb, 102*mul, 109*mul, 128*mul);
          break;
       
       case BWSelenium:
          filter.changeTonality(data, w, h, sb, 122*mul, 115*mul, 122*mul);
          break;
       
       case BWPlatinum:
          filter.changeTonality(data, w, h, sb, 115*mul, 110*mul, 106*mul);
          break;
    }
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_bwsepia.moc"

