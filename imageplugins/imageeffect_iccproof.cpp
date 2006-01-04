/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-12-21
 * Copyright 2005 by F.J. Cruz
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
 * ============================================================ */

// Qt includes.
 
#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qradiobutton.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktabwidget.h>

// Digikam includes.

#include "imageiface.h"
#include "imageguidewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"

// Local includes.

#include "imageeffect_iccproof.h"

namespace DigikamImagesPluginCore
{

ImageEffect_ICCProof::ImageEffect_ICCProof(QWidget* parent)
                    : Digikam::ImageDlgBase(parent,i18n("Color Management"), 
                                            "colormanagement", false)
{
    m_destinationPreviewData = 0L;

    setHelp("colormanagement.anchor", "digikam");

    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout *l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageGuideWidget(375, 250, frame, true,
                                                    Digikam::ImageGuideWidget::PickColorMode,
                                                    Qt::red, 1, false,
                                                    Digikam::ImageGuideWidget::TargetPreviewImage);
    l->addWidget(m_previewWidget, 0);
    QWhatsThis::add(m_previewWidget, i18n("<p>Here you can see the image preview after "
                                          "convert it with a color profile</p>"));
    setPreviewAreaWidget( frame );
    
    // -------------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout *gridSettings = new QGridLayout( gboxSettings, 10, 4, marginHint(), spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel: "), gboxSettings);
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(false, gboxSettings);
    m_channelCB->insertItem(i18n("Luminosity"));
    m_channelCB->insertItem(i18n("Red"));
    m_channelCB->insertItem(i18n("Green"));
    m_channelCB->insertItem(i18n("Blue"));
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

    //-- Build rendering intents options group -----------------------

    QVButtonGroup *m_intentsBG = new QVButtonGroup(gboxSettings);
    m_intentsBG->setTitle(i18n("Select Rendering Intent"));

    QComboBox *m_renderingIntentsCB = new QComboBox(false, m_intentsBG);
    
    m_renderingIntentsCB->insertItem("Perceptual");
    m_renderingIntentsCB->insertItem("Absolute Colorimetric");
    m_renderingIntentsCB->insertItem("Relative Colorimetric");
    m_renderingIntentsCB->insertItem("Saturation");

    QWhatsThis::add( m_renderingIntentsCB, i18n("<ul><li>Perceptual intent causes the full gamut of the image to be compressed or expanded to fill the gamut of the destination device, so that gray balance is preserved but colorimetric accuracy may not be preserved.\n"
    "In other words, if certain colors in an image fall outside of the range of colors that the output device can render, the picture intent will cause all the colors in the image to be adjusted so that the every color in the image falls within the range that can be rendered and so that the relationship between colors is preserved as much as possible.\n"
    "This intent is most suitable for display of photographs and images, and is the default intent.</li>"
    "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged.\n"
    "This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, logo colors, ...).</li>"
    "<li>Relative Colorimetric intent is defined such that any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged. Proof intent does not preserve the white point.</li>"
    "<li>Saturarion intent preserves the saturation of colors in the image at the possible expense of hue and lightness.\n"
    "Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to achieve the desired effects.\n"
    "This intent is most suitable for business graphics such as charts, where it is more important that the colors be vivid and contrast well with each other rather than a specific color.</li></ul>"));

    gridSettings->addMultiCellWidget(m_intentsBG, 9, 9, 0, 4);

    // -------------------------------------------------------------

    m_tabsWidgets = new KTabWidget(gboxSettings);
    QWidget *generalOptions = new QWidget(m_tabsWidgets);
    QWidget *inProfiles = new QWidget(m_tabsWidgets);
    QWidget *proofProfiles = new QWidget(m_tabsWidgets);
    QWidget *displayProfiles = new QWidget(m_tabsWidgets);

    m_tabsWidgets->addTab(generalOptions, i18n("General"));
    QWhatsThis::add(generalOptions, i18n("<p>You can set here general parameters.</p>"));

    //---------- Zero Page Setup ----------------------------------

    QVBoxLayout *zeroPageLayout = new QVBoxLayout(generalOptions, 0, KDialog::spacingHint());

    QButtonGroup *m_optionsBG = new QButtonGroup(4, Qt::Vertical, generalOptions);
    m_optionsBG->setFrameStyle(QFrame::NoFrame);

    QCheckBox *m_doSoftProofBox = new QCheckBox(m_optionsBG);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    QWhatsThis::add(m_doSoftProofBox, i18n("<p>The obtained transform emulates the device described"
                                           " by the \"Proofing\" profile. Useful to preview final"
                                           " result whithout rendering to physical medium.</p>"));

    QCheckBox *m_checkGamutBox = new QCheckBox(m_optionsBG);
    m_checkGamutBox->setText(i18n("Check gamut"));
    QWhatsThis::add(m_checkGamutBox, i18n("<p>You can use this option if you want to show"
                                          " the colors that are out of the printer gamut<p>"));

    QCheckBox *m_embeddProfileBox = new QCheckBox(m_optionsBG);
    m_embeddProfileBox->setText(i18n("Embedd profile"));
    QWhatsThis::add(m_embeddProfileBox, i18n("<p>You can use this option if you want to embedd"
                                             " into the image the selected color profile.</p>"));

    QCheckBox *m_BPCBox = new QCheckBox(m_optionsBG);
    m_BPCBox->setText(i18n("Use BPC"));
    QWhatsThis::add(m_BPCBox, i18n("<p>The Black Point Compensation (BPC) feature does work in conjunction "
                                   "with relative colorimetric intent. Perceptual intent should make no "
                                   "difference, although it affects some profiles.</p>"
                                   "<p>The mechanics are simple. BPC does scale full image across gray "
                                   "axis in order to accommodate the darkest tone origin media can "
                                   "render to darkest tone destination media can render. As a such, "
                                   "BPC is primarily targeting CMYK.</p>"));

    zeroPageLayout->addWidget(m_optionsBG);
    zeroPageLayout->addStretch();

    //---------- End Zero Page -----------------------------------

    m_tabsWidgets->addTab(inProfiles, i18n("Input"));
    QWhatsThis::add(inProfiles, i18n("<p>Set here all parameters relevant of Input Color Profiles.</p>"));

    //---------- First Page Setup ----------------------------------

    QVBoxLayout *firstPageLayout = new QVBoxLayout(inProfiles, 0, KDialog::spacingHint());

    QButtonGroup *m_inProfile = new QButtonGroup(6, Qt::Vertical, inProfiles);
    m_inProfile->setFrameStyle(QFrame::NoFrame);

    QRadioButton *m_useEmbeddedProfile = new QRadioButton(m_inProfile);
    m_useEmbeddedProfile->setText(i18n("Use embedded profile"));

    QRadioButton *m_useSRGBDefaultProfile = new QRadioButton(m_inProfile);
    m_useSRGBDefaultProfile->setText(i18n("Use builtin sRGB profile"));

    QRadioButton *m_useInDefaultProfile = new QRadioButton(m_inProfile);
    m_useInDefaultProfile->setText(i18n("Use default profile"));

    QRadioButton *m_useInSelectedProfile = new QRadioButton(m_inProfile);
    m_useInSelectedProfile->setText(i18n("Use selected profile"));

    QComboBox *m_inProfilesCB = new QComboBox(m_inProfile);
    
    QPushButton *m_inProfilesInfo = new QPushButton(i18n("Info"), m_inProfile);

    firstPageLayout->addWidget(m_inProfile);
    firstPageLayout->addStretch();

    //---------- End First Page ------------------------------------

    m_tabsWidgets->addTab(proofProfiles, i18n("Proofing"));
    QWhatsThis::add(proofProfiles, i18n("<p>Set here all parameters relevant of Proofing Color Profiles.</p>"));

    //---------- Second Page Setup ---------------------------------

    QVBoxLayout *secondPageLayout = new QVBoxLayout(proofProfiles, 0, KDialog::spacingHint());

    QButtonGroup *m_proofProfile = new QButtonGroup(4,Qt::Vertical, proofProfiles);
    m_proofProfile->setFrameStyle(QFrame::NoFrame);

    QRadioButton *m_useOutDefaultProfile = new QRadioButton(m_proofProfile);
    m_useOutDefaultProfile->setText(i18n("Use default proof profile"));

    QRadioButton *m_useOutSelectedProfile = new QRadioButton(m_proofProfile);
    m_useOutSelectedProfile->setText(i18n("Use selected profile"));
    
    QComboBox *m_outProfileCB = new QComboBox(m_proofProfile);

    QPushButton *m_outProfilesInfo = new QPushButton(i18n("Info"), m_proofProfile);


    secondPageLayout->addWidget(m_proofProfile);
    secondPageLayout->addStretch();

    //---------- End Second Page -----------------------------------

    m_tabsWidgets->addTab(displayProfiles, i18n("Display"));
    QWhatsThis::add(displayProfiles, i18n("<p>Set here all parameters relevant of Display Color Profiles.</p>"));

    //---------- Third Page Setup ----------------------------------

    QVBoxLayout *thirdPageLayout = new QVBoxLayout(displayProfiles, 0, KDialog::spacingHint());

    QButtonGroup *m_displayProfile = new QButtonGroup(4,Qt::Vertical, displayProfiles);
    m_displayProfile->setFrameStyle(QFrame::NoFrame);

    QRadioButton *m_useDisplayDefaultProfile = new QRadioButton(m_displayProfile);
    m_useDisplayDefaultProfile->setText(i18n("Use default display profile"));

    QRadioButton *m_useDisplaySelectedProfile = new QRadioButton(m_displayProfile);
    m_useDisplaySelectedProfile->setText(i18n("Use selected profile"));

    QComboBox *m_displayProfileCB = new QComboBox(m_displayProfile);

    QPushButton *m_DisplayProfilesInfo = new QPushButton(i18n("Info"), m_displayProfile);

    thirdPageLayout->addWidget(m_displayProfile);
    thirdPageLayout->addStretch();
    
    //---------- End Third Page ------------------------------------

    gridSettings->addMultiCellWidget(m_tabsWidgets, 10, 10, 8, 4);

    gridSettings->setRowStretch(10, 10);    
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

//     connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
//             this, SLOT(slotEffect()));
                        
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));    
            
    connect(m_previewWidget, SIGNAL(spotPositionChanged( const Digikam::DColor &, bool, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    // -------------------------------------------------------------

    enableButtonOK( false );
}

ImageEffect_ICCProof::~ImageEffect_ICCProof()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_ICCProof::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_ICCProof::slotChannelChanged( int channel )
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

void ImageEffect_ICCProof::slotScaleChanged( int scale )
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_ICCProof::slotDefault()
{
     kdDebug() << "Doing default" << endl;
    slotEffect();
}

void ImageEffect_ICCProof::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

//     double b = m_bInput->value();
//     double c = m_cInput->value() + (double)(1.00);    
//     double g = m_gInput->value() + (double)(1.00);
//     bool   o = m_overExposureIndicatorBox->isChecked();

//     enableButtonOK( b != 0.0 || c != 1.0 || g != 1.0 );
    
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
//     Digikam::BCGModifier cmod;
//     cmod.setOverIndicator(o);
//     cmod.setGamma(g);
//     cmod.setBrightness(b);
//     cmod.setContrast(c);
//     cmod.applyBCG(preview);
    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
   
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    kdDebug() << "Doing updateData" << endl;
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_ICCProof::slotOk()
{
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_iccproof.moc"
