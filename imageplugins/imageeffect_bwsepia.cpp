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
#include <qintdict.h>

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
#include "listboxpreviewitem.h"

// Local includes.

#include "imageeffect_bwsepia.h"

namespace DigikamImagesPluginCore
{

class PreviewPixmapFactory : public QObject
{
public:

    PreviewPixmapFactory(ImageEffect_BWSepia* bwSepia);

    void invalidate() { m_previewPixmapMap.clear(); }

    const QPixmap* pixmap(int id);

private:
    QPixmap makePixmap(int id);

    QIntDict<QPixmap> m_previewPixmapMap;
    ImageEffect_BWSepia* m_bwSepia;
};

PreviewPixmapFactory::PreviewPixmapFactory(ImageEffect_BWSepia* bwSepia)
    : QObject(bwSepia),
      m_bwSepia(bwSepia)
{
    m_previewPixmapMap.setAutoDelete(true);
}

const QPixmap* PreviewPixmapFactory::pixmap(int id)
{
    if (m_previewPixmapMap.find(id) == 0) {
        QPixmap pix = makePixmap(id);
        m_previewPixmapMap.insert(id, new QPixmap(pix));
    }

    QPixmap* res = m_previewPixmapMap[id];

    return res;
}

QPixmap PreviewPixmapFactory::makePixmap(int id)
{
    return m_bwSepia->getThumbnailForEffect(id);
}


class ListBoxBWPreviewItem : public Digikam::ListBoxPreviewItem
{
public:

    ListBoxBWPreviewItem(QListBox *listbox, const QString &text,
                         PreviewPixmapFactory* factory, int id)
        : ListBoxPreviewItem(listbox, QPixmap(), text),
          m_previewPixmapFactory(factory),
          m_id(id)
    {};

    virtual const QPixmap* pixmap() const;

private:
    PreviewPixmapFactory* m_previewPixmapFactory;
    int m_id;
};

const QPixmap* ListBoxBWPreviewItem::pixmap() const
{
    return m_previewPixmapFactory->pixmap(m_id);
}


// -----------------------------------------------------------------------------------


ImageEffect_BWSepia::ImageEffect_BWSepia(QWidget* parent)
                   : Digikam::ImageDlgBase(parent, i18n("Convert to Black & White"), 
                                           "convertbw", false),
                     m_destinationPreviewData(0L),
                     m_channelCB(0),
                     m_scaleBG(0),
                     m_bwFilters(0),
                     m_bwTone(0),
                     m_overExposureIndicatorBox(0),
                     m_cInput(0),
                     m_tab(0),
                     m_previewWidget(0),
                     m_histogramWidget(0),
                     m_curvesWidget(0),
                     m_curves(0),
                     m_originalImage(0),
                     m_previewPixmapFactory(0)
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

    m_bwFilters = new QListBox(m_tab);
    m_bwFilters->setColumnMode(1);
    m_bwFilters->setVariableWidth(false);
    m_bwFilters->setVariableHeight(false);
    Digikam::ListBoxWhatsThis* whatsThis = new Digikam::ListBoxWhatsThis(m_bwFilters);
    m_previewPixmapFactory = new PreviewPixmapFactory(this);

    int type = BWNoFilter;

    QListBoxItem *item = new ListBoxBWPreviewItem(m_bwFilters, i18n("No Black & White Filter"), m_previewPixmapFactory, type);
    whatsThis->add( item, i18n("<b>No Black & White Filter</b>:"
                               "<p>Do not apply a black a filter to the image.</p>"));
    
    ++type;
    item = new ListBoxBWPreviewItem(m_bwFilters, i18n("Neutral"), m_previewPixmapFactory, type);
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Neutral Black & White</b>:"
                                   "<p>Simulate black and white neutral film exposure.</p>")
                                   .arg(previewEffectPic("neutralbw")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwFilters, i18n("Green Filter"), m_previewPixmapFactory, type);
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Green Filter</b>:"
                                 "<p>Simulate black and white film exposure using green filter. "
                                 "This provides an universal asset for all scenics, especially suited for portraits "
                                 "photographed against sky.</p>").arg(previewEffectPic("bwgreen")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwFilters, i18n("Orange Filter"), m_previewPixmapFactory, type);
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Orange Filter</b>:"
                                  "<p>Simulate black and white film exposure using orange filter. "
                                  "This will enhances landscapes, marine scenes and aerial "
                                  "photography.</p>").arg(previewEffectPic("bworange")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwFilters, i18n("Red Filter"), m_previewPixmapFactory, type);
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Red Filter</b>:"
                               "<p>Simulate black and white film exposure using red filter. "
                               "Using this one creates dramatic sky effects and simulates moonlight scenes "
                               "in daytime.</p>").arg(previewEffectPic("bwred")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwFilters, i18n("Yellow Filter"), m_previewPixmapFactory, type);
    whatsThis->add( item, i18n("<img source=\"%1\"> <b>Black & White with Yellow Filter</b>:"
                                  "<p>Simulate black and white film exposure using yellow filter. "
                                  "Most natural tonal correction and improves contrast. Ideal for "
                                  "landscapes.</p>").arg(previewEffectPic("bwyellow")));

    m_tab->insertTab(m_bwFilters, i18n("Filters"), BWFiltersTab);

    // -------------------------------------------------------------

    m_bwTone = new QListBox(m_tab);
    m_bwTone->setColumnMode(1);
    m_bwTone->setVariableWidth(false);
    m_bwTone->setVariableHeight(false);
    Digikam::ListBoxWhatsThis* whatsThis2 = new Digikam::ListBoxWhatsThis(m_bwTone);

    type = BWNoTone;

    item = new ListBoxBWPreviewItem(m_bwTone, i18n("No Tone Filter"), m_previewPixmapFactory, type);
    whatsThis2->add( item, i18n("<b>No Tone Filter</b>:"
                                "<p>Do not apply a tone filter to the image.</p>"));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwTone, i18n("Sepia Tone"), m_previewPixmapFactory, type);
    whatsThis2->add( item, i18n("<img source=\"%1\"> <b>Black & White with Sepia Tone</b>:"
                                 "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                                 "the shadows-very similar to the process of bleaching a print and re-developing in a sepia "
                                 "toner.</p>").arg(previewEffectPic("sepia")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwTone, i18n("Brown Tone"), m_previewPixmapFactory, type);
    whatsThis2->add( item, i18n("<img source=\"%1\"> <b>Black & White with Brown Tone</b>:"
                                 "<p>This filter is more neutral than Sepia Tone filter.</p>").arg(previewEffectPic("browntone")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwTone, i18n("Cold Tone"), m_previewPixmapFactory, type);
    whatsThis2->add( item, i18n("<img source=\"%1\"> <b>Black & White with Cold Tone</b>:"
                                "<p>Start subtle and replicate printing on a cold tone black and white "
                                "paper such as a bromide enlarging paper.</p>").arg(previewEffectPic("coldtone")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwTone, i18n("Selenium Tone"), m_previewPixmapFactory, type);
    whatsThis2->add( item, i18n("<img source=\"%1\"> <b>Black & White with Selenium Tone</b>:"
                                    "<p>This effect replicate traditional selenium chemical toning done "
                                    "in the darkroom.</p>").arg(previewEffectPic("selenium")));

    ++type;
    item = new ListBoxBWPreviewItem(m_bwTone, i18n("Platinum Tone"), m_previewPixmapFactory, type);
    whatsThis2->add( item, i18n("<img source=\"%1\"> <b>Black & White with Platinum Tone</b>:"
                                     "<p>This effect replicate traditional platinum chemical toning done "
                                     "in the darkroom.</p>").arg(previewEffectPic("platinum")));
    
    m_tab->insertTab(m_bwTone, i18n("Tone"), ToneTab);

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

    m_bwFilters->setFocus();
    gridSettings->addMultiCellWidget(m_tab, 3, 3, 0, 4);
    gridSettings->setRowStretch(3, 10);
    setUserAreaWidget(gboxSettings);
    
    // -------------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Black and White Convertion Tool");
    m_tab->setCurrentPage(config->readNumEntry("Settings Tab", BWFiltersTab));
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

    connect(m_bwFilters, SIGNAL(highlighted(int)),
            this, SLOT(slotEffect()));

    connect(m_bwTone, SIGNAL(highlighted(int)),
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
    int w   = thumb.width();
    int h   = thumb.height();
    bool sb = thumb.sixteenBit();
    bool a  = thumb.hasAlpha();

    blackAndWhiteConversion(thumb.bits(), w, h, sb, type);

    if (m_curves && m_overExposureIndicatorBox) { // in case we're called before the ctor is done
        uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
        m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel, m_overExposureIndicatorBox->isChecked());
        m_curves->curvesLutProcess(thumb.bits(), targetData, w, h);

        Digikam::DImg preview(w, h, sb, a, targetData);
        Digikam::BCGModifier cmod;
        cmod.setContrast(m_cInput->value() + (double)(1.00));
        cmod.applyBCG(preview);

        thumb.putImageData(preview.bits());

        delete [] targetData;
    }
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
    m_bwFilters->blockSignals(true);
    m_bwTone->blockSignals(true);
    m_cInput->blockSignals(true);

    m_bwFilters->setCurrentItem(0);
    m_bwFilters->setSelected(0, true);

    m_bwTone->setCurrentItem(0);
    m_bwTone->setSelected(0, true);

    m_cInput->setValue(0.0);
    
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curves->curvesChannelReset(channel);

    m_curvesWidget->reset();
    
    m_cInput->blockSignals(false);
    m_bwTone->blockSignals(false);
    m_bwFilters->blockSignals(false);

    m_previewPixmapFactory->invalidate();
    m_bwFilters->triggerUpdate(false);
    m_bwTone->triggerUpdate(false);

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

    // Apply black and white filter.

    blackAndWhiteConversion(m_destinationPreviewData, w, h, sb, m_bwFilters->currentItem());

    // Apply color tone filter.

    blackAndWhiteConversion(m_destinationPreviewData, w, h, sb, m_bwTone->currentItem() + BWNoTone);

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

void ImageEffect_BWSepia::slotTimer()
{
    Digikam::ImageDlgBase::slotTimer();
    if (m_previewPixmapFactory && m_bwFilters && m_bwTone) {
        m_previewPixmapFactory->invalidate();
        m_bwFilters->triggerUpdate(false);
        m_bwTone->triggerUpdate(false);
    }
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
        // Apply black and white filter.
    
        blackAndWhiteConversion(data, w, h, sb, m_bwFilters->currentItem());
    
        // Apply color tone filter.
    
        blackAndWhiteConversion(data, w, h, sb, m_bwTone->currentItem() + BWNoTone);

        // Calculate and apply the curve on image.
    
        uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
        m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
        m_curves->curvesLutProcess(data, targetData, w, h);
        
        // Adjust contrast.
            
        Digikam::DImg img(w, h, sb, a, targetData);
        Digikam::BCGModifier cmod;
        cmod.setContrast(m_cInput->value() + (double)(1.00));
        cmod.applyBCG(img);

        iface->putOriginalImage(i18n("Convert to Black && White"), img.bits());

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
       case BWNoFilter:
          break;

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

       case BWNoTone:
          break;
       
       case BWSepiaTone:
          filter.changeTonality(data, w, h, sb, 162*mul, 132*mul, 101*mul);
          break;
       
       case BWBrownTone:
          filter.changeTonality(data, w, h, sb, 129*mul, 115*mul, 104*mul);
          break;
       
       case BWColdTone:
          filter.changeTonality(data, w, h, sb, 102*mul, 109*mul, 128*mul);
          break;
       
       case BWSeleniumTone:
          filter.changeTonality(data, w, h, sb, 122*mul, 115*mul, 122*mul);
          break;
       
       case BWPlatinumTone:
          filter.changeTonality(data, w, h, sb, 115*mul, 110*mul, 106*mul);
          break;
    }
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_bwsepia.moc"

