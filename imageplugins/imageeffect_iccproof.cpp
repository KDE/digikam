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
#include <kconfig.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kmessagebox.h>

// Digikam includes.

#include "imageiface.h"
#include "imageguidewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "iccpreviewwidget.h"
#include "icctransform.h"

// Local includes.

#include "imageeffect_iccproof.h"

namespace DigikamImagesPluginCore
{

ImageEffect_ICCProof::ImageEffect_ICCProof(QWidget* parent)
                    : Digikam::ImageDlgBase(parent,i18n("Color Management"), 
                                            "colormanagement", false)
{
    m_destinationPreviewData = 0L;

    cmEnabled = true;
    inPath = QString::null;
    spacePath = QString::null;
    proofPath = QString::null;
    displayPath = QString::null;

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

    QVButtonGroup *intentsBG = new QVButtonGroup(gboxSettings);
    intentsBG->setTitle(i18n("Select Rendering Intent"));

    m_renderingIntentsCB = new QComboBox(false, intentsBG);
    
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

    gridSettings->addMultiCellWidget(intentsBG, 9, 9, 0, 4);

    // -------------------------------------------------------------

    m_tabsWidgets = new KTabWidget(gboxSettings);
    QWidget *generalOptions = new QWidget(m_tabsWidgets);
    QWidget *inProfiles = new QWidget(m_tabsWidgets);
    QWidget *proofProfiles = new QWidget(m_tabsWidgets);
    QWidget *displayProfiles = new QWidget(m_tabsWidgets);
    QWidget *spaceProfiles = new QWidget(m_tabsWidgets);

    m_tabsWidgets->addTab(generalOptions, i18n("General"));
    QWhatsThis::add(generalOptions, i18n("<p>You can set here general parameters.</p>"));

    //---------- Zero Page Setup ----------------------------------

    QVBoxLayout *zeroPageLayout = new QVBoxLayout(generalOptions, 0, KDialog::spacingHint());

    QButtonGroup *m_optionsBG = new QButtonGroup(4, Qt::Vertical, generalOptions);
    m_optionsBG->setFrameStyle(QFrame::NoFrame);

    m_doSoftProofBox = new QCheckBox(m_optionsBG);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    QWhatsThis::add(m_doSoftProofBox, i18n("<p>The obtained transform emulates the device described"
                                           " by the \"Proofing\" profile. Useful to preview final"
                                           " result whithout rendering to physical medium.</p>"));

    m_checkGamutBox = new QCheckBox(m_optionsBG);
    m_checkGamutBox->setText(i18n("Check gamut"));
    QWhatsThis::add(m_checkGamutBox, i18n("<p>You can use this option if you want to show"
                                          " the colors that are out of the printer gamut<p>"));

    QCheckBox *m_embeddProfileBox = new QCheckBox(m_optionsBG);
    m_embeddProfileBox->setText(i18n("Embedd profile"));
    QWhatsThis::add(m_embeddProfileBox, i18n("<p>You can use this option if you want to embedd"
                                             " into the image the selected color profile.</p>"));

    m_BPCBox = new QCheckBox(m_optionsBG);
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

    QButtonGroup *inProfileBG = new QButtonGroup(6, Qt::Vertical, inProfiles);
    inProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useEmbeddedProfile = new QRadioButton(inProfileBG);
    m_useEmbeddedProfile->setText(i18n("Use embedded profile"));

    m_useSRGBDefaultProfile = new QRadioButton(inProfileBG);
    m_useSRGBDefaultProfile->setText(i18n("Use builtin sRGB profile"));

    m_useInDefaultProfile = new QRadioButton(inProfileBG);
    m_useInDefaultProfile->setText(i18n("Use default profile"));

    m_useInSelectedProfile = new QRadioButton(inProfileBG);
    m_useInSelectedProfile->setText(i18n("Use selected profile"));
    
    m_inProfilesCB = new KURLRequester(inProfileBG);
    m_inProfilesCB->setMode(KFile::File|KFile::ExistingOnly);
    m_inProfilesCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *inProfiles_dialog = m_inProfilesCB->fileDialog();
    m_iccInPreviewWidget = new Digikam::ICCPreviewWidget(inProfiles_dialog);
    inProfiles_dialog->setPreviewWidget(m_iccInPreviewWidget);
    
    QPushButton *inProfilesInfo = new QPushButton(i18n("Info"), inProfileBG);

    firstPageLayout->addWidget(inProfileBG);
    firstPageLayout->addStretch();

    //---------- End First Page ------------------------------------

    m_tabsWidgets->addTab(spaceProfiles, i18n("Workspace"));
    QWhatsThis::add(spaceProfiles, i18n("<p>Set here all parameters relevant of Workspace Color Profiles.</p>"));

    //---------- Second Page Setup ---------------------------------

    QVBoxLayout *secondPageLayout = new QVBoxLayout(spaceProfiles, 0, KDialog::spacingHint());

    QButtonGroup *spaceProfileBG = new QButtonGroup(4,Qt::Vertical, spaceProfiles);
    spaceProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useSpaceDefaultProfile = new QRadioButton(spaceProfileBG);
    m_useSpaceDefaultProfile->setText(i18n("Use default workspace profile"));

    m_useSpaceSelectedProfile = new QRadioButton(spaceProfileBG);
    m_useSpaceSelectedProfile->setText(i18n("Use selected profile"));
    
    m_spaceProfileCB = new KURLRequester(spaceProfileBG);
    m_spaceProfileCB->setMode(KFile::File|KFile::ExistingOnly);
    m_spaceProfileCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *spaceProfiles_dialog = m_spaceProfileCB->fileDialog();
    m_iccSpacePreviewWidget = new Digikam::ICCPreviewWidget(spaceProfiles_dialog);
    spaceProfiles_dialog->setPreviewWidget(m_iccSpacePreviewWidget);

    QPushButton *spaceProfilesInfo = new QPushButton(i18n("Info"), spaceProfileBG);


    secondPageLayout->addWidget(spaceProfileBG);
    secondPageLayout->addStretch();

    //---------- End Second Page -----------------------------------

     m_tabsWidgets->addTab(proofProfiles, i18n("Proofing"));
    QWhatsThis::add(proofProfiles, i18n("<p>Set here all parameters relevant of Proofing Color Profiles.</p>"));

    //---------- Third Page Setup ---------------------------------

    QVBoxLayout *thirdPageLayout = new QVBoxLayout(proofProfiles, 0, KDialog::spacingHint());

    QButtonGroup *proofProfileBG = new QButtonGroup(4,Qt::Vertical, proofProfiles);
    proofProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useProofDefaultProfile = new QRadioButton(proofProfileBG);
    m_useProofDefaultProfile->setText(i18n("Use default proof profile"));

    m_useProofSelectedProfile = new QRadioButton(proofProfileBG);
    m_useProofSelectedProfile->setText(i18n("Use selected profile"));
    
    m_proofProfileCB = new KURLRequester(proofProfileBG);
    m_proofProfileCB->setMode(KFile::File|KFile::ExistingOnly);
    m_proofProfileCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *proofProfiles_dialog = m_proofProfileCB->fileDialog();
    m_iccProofPreviewWidget = new Digikam::ICCPreviewWidget(proofProfiles_dialog);
    proofProfiles_dialog->setPreviewWidget(m_iccProofPreviewWidget);

    QPushButton *proofProfilesInfo = new QPushButton(i18n("Info"), proofProfileBG);


    thirdPageLayout->addWidget(proofProfileBG);
    thirdPageLayout->addStretch();

    //---------- End Third Page -----------------------------------

    m_tabsWidgets->addTab(displayProfiles, i18n("Display"));
    QWhatsThis::add(displayProfiles, i18n("<p>Set here all parameters relevant of Display Color Profiles.</p>"));

    //---------- Fourth Page Setup ----------------------------------

    QVBoxLayout *fourthPageLayout = new QVBoxLayout(displayProfiles, 0, KDialog::spacingHint());

    QButtonGroup *displayProfileBG = new QButtonGroup(4,Qt::Vertical, displayProfiles);
    displayProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useDisplayDefaultProfile = new QRadioButton(displayProfileBG);
    m_useDisplayDefaultProfile->setText(i18n("Use default display profile"));

    m_useDisplaySelectedProfile = new QRadioButton(displayProfileBG);
    m_useDisplaySelectedProfile->setText(i18n("Use selected profile"));

    m_displayProfileCB = new KURLRequester(displayProfileBG);
    m_displayProfileCB->setMode(KFile::File|KFile::ExistingOnly);
    m_displayProfileCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *displayProfiles_dialog = m_displayProfileCB->fileDialog();
    m_iccDisplayPreviewWidget = new Digikam::ICCPreviewWidget(displayProfiles_dialog);
    displayProfiles_dialog->setPreviewWidget(m_iccDisplayPreviewWidget);

    QPushButton *displayProfilesInfo = new QPushButton(i18n("Info"), displayProfileBG);

    fourthPageLayout->addWidget(displayProfileBG);
    fourthPageLayout->addStretch();
    
    //---------- End Fourth Page ------------------------------------

    gridSettings->addMultiCellWidget(m_tabsWidgets, 10, 10, 0, 0);

     //-- Build rendering intents options group -----------------------

    QHButtonGroup *m_testItBG = new QHButtonGroup(gboxSettings);
    m_testItBG->setFrameStyle(QFrame::NoFrame);

    m_testItBt = new QPushButton(i18n("Test"),m_testItBG);
    

    gridSettings->addMultiCellWidget(m_testItBG, 11, 11, 0, 0);

    // -------------------------------------------------------------

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

    connect(m_testItBt, SIGNAL(clicked()), this, SLOT(slotTestIt()));

    connect(inProfilesInfo, SIGNAL(clicked()), this, SLOT(slotInICCInfo()));

    connect(spaceProfilesInfo, SIGNAL(clicked()), this, SLOT(slotSpaceICCInfo()));

    connect(proofProfilesInfo, SIGNAL(clicked()), this, SLOT(slotProofICCInfo()));

    connect(displayProfilesInfo, SIGNAL(clicked()), this, SLOT(slotDisplayICCInfo()));

    connect(m_useInDefaultProfile, SIGNAL(clicked()), this, SLOT(slotCMDisabledWarning()));

    connect(m_useSpaceDefaultProfile, SIGNAL(clicked()), this, SLOT(slotCMDisabledWarning()));

    connect(m_useProofDefaultProfile, SIGNAL(clicked()), this, SLOT(slotCMDisabledWarning()));

    connect(m_useDisplayDefaultProfile, SIGNAL(clicked()), this, SLOT(slotCMDisabledWarning()));

    // -------------------------------------------------------------

    enableButtonOK( false );

    readSettings();
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

//     Digikam::ImageIface* iface = m_previewWidget->imageIface();
    iface                      = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
    if (!iface->originalHasICCEmbedded())
        m_useEmbeddedProfile->setEnabled(false);
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

//     if (original.getICCProfil().isNull())
//     {
//         m_useEmbeddedProfile->setEnabled(false);
//     }

    kapp->restoreOverrideCursor();
    
}

void ImageEffect_ICCProof::slotOk()
{

    /// @todo implement me
}

void ImageEffect_ICCProof::slotTestIt()
{
    /// @todo implement me
    kapp->setOverrideCursor(KCursor::waitCursor());

    enableButtonOK(true);

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
        delete [] m_destinationPreviewData;

//     Digikam::ImageIface* iface = m_previewWidget->imageIface();
//     m_destinationPreviewData   = iface->getPreviewData();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
}

void ImageEffect_ICCProof::readSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");

    if (!config->readBoolEntry("EnableCM", false))
    {
        cmEnabled = false;
    }
    else
    {
        inPath = config->readPathEntry("InProfileFile");
        spacePath = config->readPathEntry("WorkProfileFile");
        displayPath = config->readPathEntry("MonitorProfileFile");
        proofPath = config->readPathEntry("ProofProfileFile");
    }
}

void ImageEffect_ICCProof::slotToggledWidgets( bool t)
{
    m_useInDefaultProfile->setEnabled(t);
    m_useProofDefaultProfile->setEnabled(t);
    m_useDisplayDefaultProfile->setEnabled(t);
    m_useSpaceDefaultProfile->setEnabled(t);
}

/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotInICCInfo()
 */
void ImageEffect_ICCProof::slotInICCInfo()
{
    getICCInfo(m_inProfilesCB->url());
}


/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotProofICCInfo()
 */
void ImageEffect_ICCProof::slotProofICCInfo()
{
    getICCInfo(m_proofProfileCB->url());
}


/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotSpaceICCInfo()
 */
void ImageEffect_ICCProof::slotSpaceICCInfo()
{
    getICCInfo(m_spaceProfileCB->url());
}


/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotDisplayICCInfo()
 */
void ImageEffect_ICCProof::slotDisplayICCInfo()
{
    getICCInfo(m_displayProfileCB->url());
}

void ImageEffect_ICCProof::getICCInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(this, i18n("Sorry, there is not any selected profile"), i18n("Profile Error"));
        return;
    }
    QString intent;
    cmsHPROFILE selectedProfile;
    selectedProfile = cmsOpenProfileFromFile(QFile::encodeName(profile), "r");

    QString  profileName = QString((cmsTakeProductName(selectedProfile)));
    QString profileDescription = QString((cmsTakeProductDesc(selectedProfile)));
    QString profileManufacturer = QString(cmsTakeCopyright(selectedProfile));
    int profileIntent = cmsTakeRenderingIntent(selectedProfile);
    
    //"Decode" profile rendering intent
    switch (profileIntent)
    {
        case 0:
            intent = i18n("Perceptual");
            break;
        case 1:
            intent = i18n("Relative Colorimetric");
            break;
        case 2:
            intent = i18n("Saturation");
            break;
        case 3:
            intent = i18n("Absolute Colorimetric");
            break;
    }

    KMessageBox::information(this, i18n("<p><b>Name:</b> ") + profileName +
                                 i18n("</p><p><b>Description:</b>  ") + profileDescription +
                                 i18n("</p><p><b>Copyright:</b>  ") + profileManufacturer +
                                 i18n("</p><p><b>Rendering Intent:</b>  ") + intent + i18n("</p><p><b>Path:</b> ") +
                                 profile + "</p>",
                                 i18n("Color Profile Info"));
}

void ImageEffect_ICCProof::slotCMDisabledWarning()
{
    if (!cmEnabled)
    {
        QString message = QString(i18n("<p>You don't have enabled Color Management in Digikam preferences.</p>"));
        message.append( i18n("<p>\"Use default profile\" options will be disabled now.</p>"));
        KMessageBox::information(this, message);
        slotToggledWidgets(false);
    }
}

}// NameSpace DigikamImagesPluginCore

#include "imageeffect_iccproof.moc"
