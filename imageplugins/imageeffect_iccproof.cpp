/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-12-21
 * Copyright 2005-2006 by F.J. Cruz
 *           2006 by Gilles Caulier
 *
 * Description : digiKam image editor tool to correct picture 
 *               colors using an ICC color profile
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
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpoint.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qradiobutton.h>
#include <qfile.h>
#include <qtoolbox.h>
#include <qtextstream.h>

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
#include <kurllabel.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <ksqueezedtextlabel.h>

// Digikam includes.

#include "bcgmodifier.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "curveswidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "iccpreviewwidget.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"

// Local includes.

#include "imageeffect_iccproof.h"

namespace DigikamImagesPluginCore
{

ImageEffect_ICCProof::ImageEffect_ICCProof(QWidget* parent)
                    : Digikam::ImageDlgBase(parent,i18n("Color Management"), 
                                            "colormanagement", true, false)
{
    m_destinationPreviewData = 0;
    m_cmEnabled              = true;
    m_hasICC                 = false;

    setHelp("colormanagement.anchor", "digikam");

    Digikam::ImageIface iface(0, 0);
    m_originalImage = iface.getOriginalImg();
    m_embeddedICC   = iface.getEmbeddedICCFromOriginalImage();
    m_curves        = new Digikam::ImageCurves(m_originalImage->sixteenBit());

    m_previewWidget = new Digikam::ImageWidget("colormanagement Tool Dialog", plainPage(),
                                               i18n("<p>Here you can see the image preview after "
                                                    "applying a color profile</p>"));
    setPreviewAreaWidget(m_previewWidget); 

    // -------------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout *gridSettings = new QGridLayout( gboxSettings, 3, 2, marginHint(), spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel: "), gboxSettings);
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(false, gboxSettings);
    m_channelCB->insertItem(i18n("Luminosity"));
    m_channelCB->insertItem(i18n("Red"));
    m_channelCB->insertItem(i18n("Green"));
    m_channelCB->insertItem(i18n("Blue"));
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red channel values.<p>"
                                       "<b>Green</b>: display the green channel values.<p>"
                                       "<b>Blue</b>: display the blue channel values.<p>"));

    m_scaleBG = new QHButtonGroup(gboxSettings);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal values are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal values are big; "
                                     "if it is used, all values (small and large) will be visible on the "
                                     "graph."));

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

    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 2);

    // -------------------------------------------------------------

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, gboxSettings, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram "
                                             "of the selected image channel. " 
					     "This one is updated after setting changes."));

    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, 
                                                    gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    gridSettings->addMultiCellWidget(m_histogramWidget, 1, 1, 0, 2);
    gridSettings->addMultiCellWidget(m_hGradient, 2, 2, 0, 2);

    // -------------------------------------------------------------

    m_toolBoxWidgets         = new QToolBox(gboxSettings);
    QWidget *generalOptions  = new QWidget(m_toolBoxWidgets);
    QWidget *inProfiles      = new QWidget(m_toolBoxWidgets);
    QWidget *spaceProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *proofProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *lightnessadjust = new QWidget(m_toolBoxWidgets);

    //---------- "General" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(GENERALPAGE, generalOptions, 
                                 SmallIconSet("misc"), i18n("General Settings"));
    QWhatsThis::add(generalOptions, i18n("<p>Here you can set general parameters.</p>"));

    QGridLayout *zeroPageLayout = new QGridLayout(generalOptions, 5, 1, 
                                  KDialog::marginHint(), KDialog::spacingHint());

    m_doSoftProofBox = new QCheckBox(generalOptions);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    QWhatsThis::add(m_doSoftProofBox, i18n("<p>Rendering emulation of the device described "
                                           "by the \"Proofing\" profile. Useful to preview final "
                                           "result without rendering to physical medium.</p>"));

    m_checkGamutBox = new QCheckBox(generalOptions);
    m_checkGamutBox->setText(i18n("Check gamut"));
    QWhatsThis::add(m_checkGamutBox, i18n("<p>You can use this option if you want to show "
                                          "the colors that are outside the printer's gamut<p>"));

    m_embeddProfileBox = new QCheckBox(generalOptions);
    m_embeddProfileBox->setChecked(true);
    m_embeddProfileBox->setText(i18n("Embed profile"));
    QWhatsThis::add(m_embeddProfileBox, i18n("<p>You can use this option to embed "
                                             "the selected work-space color profile into the image.</p>"));

    m_BPCBox = new QCheckBox(generalOptions);
    m_BPCBox->setText(i18n("Use BPC"));
    QWhatsThis::add(m_BPCBox, i18n("<p>The Black Point Compensation (BPC) feature does work in conjunction "
                                   "with Relative Colorimetric Intent. Perceptual intent should make no "
                                   "difference, since BPC is always on, and in Absolute Colorimetric "
				   "Intent it is always turned off.</p>"
                                   "<p>BPC does compensate a lack of ICC profiles in the dark tone rendering."
                                   "With BPC the dark tones are optimally mapped (no clipping) from original media "
                                   "to the destination media can render, e.g. the combination paper/ink.</p>"));

    QLabel *intent = new QLabel(i18n("Rendering Intent:"), generalOptions);
    m_renderingIntentsCB = new QComboBox(false, generalOptions);
    m_renderingIntentsCB->insertItem("Perceptual");
    m_renderingIntentsCB->insertItem("Absolute Colorimetric");
    m_renderingIntentsCB->insertItem("Relative Colorimetric");
    m_renderingIntentsCB->insertItem("Saturation");
    QWhatsThis::add( m_renderingIntentsCB, i18n("<ul><li>Perceptual intent causes the full gamut "
                "of the image to be compressed or expanded to fill the gamut of the destination media, "
                "so that gray balance is preserved but colorimetric accuracy may not be preserved.<br>"
                "In other words, if certain colors in an image fall outside of the range of colors that "
                "the output device can render, the picture intent will cause all the colors in the image "
                "to be adjusted so that the every color in the image falls within the range that can be "
                "rendered and so that the relationship between colors is preserved as much as possible.<br>"
                "This intent is most suitable for display of photographs and images, and is the default "
                "intent.</li>"
                "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the "
                "output device can render are adjusted to the closest color that can be rendered, while all "
                "other colors are left unchanged.<br>"
                "This intent preserves the white point and is most suitable for spot colors (Pantone, "
                "TruMatch, logo colors, ...).</li>"
                "<li>Relative Colorimetric intent is defined such that any colors that fall outside the "
                "range that the output device can render are adjusted to the closest color that can be "
                "rendered, while all other colors are left unchanged. Proof intent does not preserve "
                "the white point.</li>"
                "<li>Saturation intent preserves the saturation of colors in the image at the possible "
                "expense of hue and lightness.<br>"
                "Implementation of this intent remains somewhat problematic, and the ICC is still working "
                "on methods to achieve the desired effects.<br>"
                "This intent is most suitable for business graphics such as charts, where it is more "
                "important that the colors be vivid and contrast well with each other rather than a "
                "specific color.</li></ul>"));

    KURLLabel *lcmsLogoLabel = new KURLLabel(generalOptions);
    lcmsLogoLabel->setAlignment( AlignTop | AlignRight );
    lcmsLogoLabel->setText(QString::null);
    lcmsLogoLabel->setURL("http://www.littlecms.com");
    KGlobal::dirs()->addResourceType("lcmslogo", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("lcmslogo", "lcmslogo.png");
    lcmsLogoLabel->setPixmap( QPixmap( directory + "lcmslogo.png" ) );
    QToolTip::add(lcmsLogoLabel, i18n("Visit Little CMS project website"));

    zeroPageLayout->addMultiCellWidget(m_doSoftProofBox, 0, 0, 0, 0);
    zeroPageLayout->addMultiCellWidget(m_checkGamutBox, 1, 1, 0, 0);
    zeroPageLayout->addMultiCellWidget(m_embeddProfileBox, 2, 2, 0, 0);
    zeroPageLayout->addMultiCellWidget(lcmsLogoLabel, 0, 2, 1, 1);
    zeroPageLayout->addMultiCellWidget(m_BPCBox, 3, 3, 0, 0);
    zeroPageLayout->addMultiCellWidget(intent, 4, 4, 0, 0);
    zeroPageLayout->addMultiCellWidget(m_renderingIntentsCB, 4, 4, 1, 1);
    zeroPageLayout->setRowStretch(5, 10);

    //---------- "Input" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(INPUTPAGE, inProfiles, SmallIconSet("camera"), i18n("Input Profile"));
    QWhatsThis::add(inProfiles, i18n("<p>Set here all parameters relevant of Input Color "
                    "Profiles.</p>"));

    QGridLayout *firstPageLayout = new QGridLayout(inProfiles, 4, 2,
                                   KDialog::marginHint(), KDialog::spacingHint());

    m_inProfileBG = new QButtonGroup(4, Qt::Vertical, inProfiles);
    m_inProfileBG->setFrameStyle(QFrame::NoFrame);
    m_inProfileBG->setInsideMargin(0);

    m_useEmbeddedProfile = new QRadioButton(m_inProfileBG);
    m_useEmbeddedProfile->setText(i18n("Use embedded profile"));

    m_useSRGBDefaultProfile = new QRadioButton(m_inProfileBG);
    m_useSRGBDefaultProfile->setText(i18n("Use builtin sRGB profile"));
    m_useSRGBDefaultProfile->setChecked(true);

    m_useInDefaultProfile = new QRadioButton(m_inProfileBG);
    m_useInDefaultProfile->setText(i18n("Use default profile"));

    m_useInSelectedProfile = new QRadioButton(m_inProfileBG);
    m_useInSelectedProfile->setText(i18n("Use selected profile"));

    m_inProfilesPath = new KURLRequester(inProfiles);
    m_inProfilesPath->setMode(KFile::File|KFile::ExistingOnly);
    m_inProfilesPath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *inProfiles_dialog = m_inProfilesPath->fileDialog();
    m_iccInPreviewWidget = new Digikam::ICCPreviewWidget(inProfiles_dialog);
    inProfiles_dialog->setPreviewWidget(m_iccInPreviewWidget);

    QPushButton *inProfilesInfo = new QPushButton(i18n("Info..."), inProfiles);

    QGroupBox *pictureInfo = new QGroupBox(2, Qt::Horizontal, i18n("Picture Information"), inProfiles);
    new QLabel(i18n("Make:"), pictureInfo);
    KSqueezedTextLabel *make  = new KSqueezedTextLabel(0, pictureInfo);
    new QLabel(i18n("Model:"), pictureInfo);
    KSqueezedTextLabel *model = new KSqueezedTextLabel(0, pictureInfo);
    make->setText(iface.getPhotographInformations().make);
    model->setText(iface.getPhotographInformations().model);

    firstPageLayout->addMultiCellWidget(m_inProfileBG, 0, 1, 0, 0);
    firstPageLayout->addMultiCellWidget(inProfilesInfo, 0, 0, 2, 2);
    firstPageLayout->addMultiCellWidget(m_inProfilesPath, 2, 2, 0, 2);
    firstPageLayout->addMultiCellWidget(pictureInfo, 3, 3, 0, 2);
    firstPageLayout->setColStretch(1, 10);
    firstPageLayout->setRowStretch(4, 10);

    //---------- "Workspace" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(WORKSPACEPAGE, spaceProfiles, 
                                 SmallIconSet("tablet"), i18n("Work-space Profile"));
    QWhatsThis::add(spaceProfiles, i18n("<p>Set here all parameters relevant of Color Work-space "
                    "Profiles.</p>"));

    QGridLayout *secondPageLayout = new QGridLayout(spaceProfiles, 3, 2, 
                                    KDialog::marginHint(), KDialog::spacingHint());

    m_spaceProfileBG = new QButtonGroup(2, Qt::Vertical, spaceProfiles);
    m_spaceProfileBG->setFrameStyle(QFrame::NoFrame);
    m_spaceProfileBG->setInsideMargin(0);

    m_useSpaceDefaultProfile = new QRadioButton(m_spaceProfileBG);
    m_useSpaceDefaultProfile->setText(i18n("Use default workspace profile"));

    m_useSpaceSelectedProfile = new QRadioButton(m_spaceProfileBG);
    m_useSpaceSelectedProfile->setText(i18n("Use selected profile"));

    m_spaceProfilePath = new KURLRequester(spaceProfiles);
    m_spaceProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    m_spaceProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *spaceProfiles_dialog = m_spaceProfilePath->fileDialog();
    m_iccSpacePreviewWidget = new Digikam::ICCPreviewWidget(spaceProfiles_dialog);
    spaceProfiles_dialog->setPreviewWidget(m_iccSpacePreviewWidget);

    QPushButton *spaceProfilesInfo = new QPushButton(i18n("Info..."), spaceProfiles);

    secondPageLayout->addMultiCellWidget(m_spaceProfileBG, 0, 1, 0, 0);    
    secondPageLayout->addMultiCellWidget(spaceProfilesInfo, 0, 0, 2, 2);    
    secondPageLayout->addMultiCellWidget(m_spaceProfilePath, 2, 2, 0, 2);    
    secondPageLayout->setColStretch(1, 10);
    secondPageLayout->setRowStretch(3, 10);

    //---------- "Proofing" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(PROOFINGPAGE, proofProfiles, 
                                 SmallIconSet("printer1"), i18n("Proofing Profile"));
    QWhatsThis::add(proofProfiles, i18n("<p>Set here all parameters relevant to Proofing Color "
                    "Profiles.</p>"));

    QGridLayout *thirdPageLayout = new QGridLayout(proofProfiles, 3, 2, 
                                   KDialog::marginHint(), KDialog::spacingHint());

    m_proofProfileBG = new QButtonGroup(2, Qt::Vertical, proofProfiles);
    m_proofProfileBG->setFrameStyle(QFrame::NoFrame);
    m_proofProfileBG->setInsideMargin(0);

    m_useProofDefaultProfile = new QRadioButton(m_proofProfileBG);
    m_useProofDefaultProfile->setText(i18n("Use default proof profile"));

    m_useProofSelectedProfile = new QRadioButton(m_proofProfileBG);
    m_useProofSelectedProfile->setText(i18n("Use selected profile"));

    m_proofProfilePath = new KURLRequester(proofProfiles);
    m_proofProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    m_proofProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *proofProfiles_dialog = m_proofProfilePath->fileDialog();
    m_iccProofPreviewWidget = new Digikam::ICCPreviewWidget(proofProfiles_dialog);
    proofProfiles_dialog->setPreviewWidget(m_iccProofPreviewWidget);

    QPushButton *proofProfilesInfo = new QPushButton(i18n("Info..."), proofProfiles);

    thirdPageLayout->addMultiCellWidget(m_proofProfileBG, 0, 1, 0, 0);    
    thirdPageLayout->addMultiCellWidget(proofProfilesInfo, 0, 0, 2, 2);    
    thirdPageLayout->addMultiCellWidget(m_proofProfilePath, 2, 2, 0, 2);    
    thirdPageLayout->setColStretch(1, 10);
    thirdPageLayout->setRowStretch(3, 10);

    //---------- "Lightness" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(LIGHTNESSPAGE, lightnessadjust, 
                                 SmallIconSet("blend"), i18n("Lightness Adjustments"));
    QWhatsThis::add(lightnessadjust, i18n("<p>Set here all lightness adjustments of target image.</p>"));

    QGridLayout *fourPageLayout = new QGridLayout( lightnessadjust, 4, 1, marginHint(), spacingHint());

    Digikam::ColorGradientWidget* vGradient = new Digikam::ColorGradientWidget(
                                                  Digikam::ColorGradientWidget::Vertical,
                                                  10, lightnessadjust );
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    m_curvesWidget = new Digikam::CurvesWidget(256, 140, m_originalImage->bits(), m_originalImage->width(),
                                               m_originalImage->height(), m_originalImage->sixteenBit(),
                                               m_curves, lightnessadjust);
    QWhatsThis::add( m_curvesWidget, i18n("<p>This is the curve adjustment of the image luminosity"));

    Digikam::ColorGradientWidget *hGradient = new Digikam::ColorGradientWidget(
                                                  Digikam::ColorGradientWidget::Horizontal,
                                                  10, lightnessadjust );
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    m_cInput = new KDoubleNumInput(lightnessadjust);
    m_cInput->setLabel(i18n("Contrast:"), AlignLeft | AlignVCenter);
    m_cInput->setPrecision(2);
    m_cInput->setRange(-1.0, 1.0, 0.01, true);
    m_cInput->setValue(0.0);
    QWhatsThis::add( m_cInput, i18n("<p>Set here the contrast adjustment of the image."));

    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), lightnessadjust);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels "
                                                      "from the target image preview will be over-colored. "
                                                      "This will not have an effect on the final rendering."));

    fourPageLayout->addMultiCellWidget(vGradient, 0, 0, 0, 0);
    fourPageLayout->addMultiCellWidget(m_curvesWidget, 0, 0, 1, 1);
    fourPageLayout->addMultiCellWidget(hGradient, 1, 1, 1, 1);
    fourPageLayout->addMultiCellWidget(m_cInput, 2, 2, 0, 1);
    fourPageLayout->addMultiCellWidget(m_overExposureIndicatorBox, 3, 3, 0, 1);
    fourPageLayout->setRowStretch(4, 10);

    // -------------------------------------------------------------

    gridSettings->addMultiCellWidget(m_toolBoxWidgets, 3, 3, 0, 2);
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(lcmsLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processLCMSURL(const QString&)));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(m_cInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_renderingIntentsCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    //-- Check box options connections -------------------------------------------

    connect(m_doSoftProofBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));      

    connect(m_checkGamutBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));      

    connect(m_BPCBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));      

    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));      

    //-- Button Group ICC profile options connections ----------------------------

    connect(m_inProfileBG, SIGNAL(released (int)),
            this, SLOT(slotEffect())); 

    connect(m_spaceProfileBG, SIGNAL(released (int)),
            this, SLOT(slotEffect())); 

    connect(m_proofProfileBG, SIGNAL(released (int)),
            this, SLOT(slotEffect())); 

    //-- url requester ICC profile connections -----------------------------------

    connect(m_inProfilesPath, SIGNAL(urlSelected(const QString&)),
            this, SLOT(slotEffect()));      

    connect(m_spaceProfilePath, SIGNAL(urlSelected(const QString&)),
            this, SLOT(slotEffect()));      

    connect(m_proofProfilePath, SIGNAL(urlSelected(const QString&)),
            this, SLOT(slotEffect()));      

    //-- Image preview widget connections ----------------------------
    
    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
            
    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    //-- ICC profile preview connections -----------------------------

    connect(inProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotInICCInfo()));

    connect(spaceProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotSpaceICCInfo()));

    connect(proofProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotProofICCInfo()));

    // -------------------------------------------------------------

    enableButtonOK( false );
    readSettings();
}

ImageEffect_ICCProof::~ImageEffect_ICCProof()
{
    writeSettings();
    m_histogramWidget->stopHistogramComputation();

    delete [] m_destinationPreviewData;
    delete m_histogramWidget;
    delete m_previewWidget;
    delete m_curvesWidget;
    delete m_curves;
}

void ImageEffect_ICCProof::readSettings()
{
    QString defaultICCPath = KGlobalSettings::documentPath();
    KConfig* config        = kapp->config();
    
    // General settings of digiKam Color Management                            
    config->setGroup("Color Management");

    if (!config->readBoolEntry("EnableCM", false))
    {
        m_cmEnabled = false;
        slotToggledWidgets(false);
    }
    else
    {
        m_inPath      = config->readPathEntry("InProfileFile");
        m_spacePath   = config->readPathEntry("WorkProfileFile");
        m_proofPath   = config->readPathEntry("ProofProfileFile");
        
        if (QFile::exists(config->readPathEntry("DefaultPath")))
        {
            defaultICCPath = config->readPathEntry("DefaultPath");
        }
        else
        {
            QString message = i18n("ICC profiles path seems to be invalid. You'll not be able to use \"Default profile\"\
                                    options.<p>Please solve it in digiKam ICC setup.");
            slotToggledWidgets( false );
            KMessageBox::information(this, message);
        }
    }
    
    // Plugin settings.
    config->setGroup("Color Management Tool");
    m_toolBoxWidgets->setCurrentIndex(config->readNumEntry("Settings Tab", GENERALPAGE));        
    m_inProfilesPath->setURL(config->readPathEntry("InputProfilePath", defaultICCPath)); 
    m_proofProfilePath->setURL(config->readPathEntry("ProofProfilePath", defaultICCPath)); 
    m_spaceProfilePath->setURL(config->readPathEntry("SpaceProfilePath", defaultICCPath));
    m_renderingIntentsCB->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
    m_doSoftProofBox->setChecked(config->readBoolEntry("DoSoftProof", false));
    m_checkGamutBox->setChecked(config->readBoolEntry("CheckGamut", false));
    m_embeddProfileBox->setChecked(config->readBoolEntry("EmbeddProfile", true));
    m_BPCBox->setChecked(config->readBoolEntry("BPC", true));
    m_inProfileBG->setButton(config->readNumEntry("InputProfileMethod", 0));
    m_spaceProfileBG->setButton(config->readNumEntry("SpaceProfileMethod", 0));
    m_proofProfileBG->setButton(config->readNumEntry("ProofProfileMethod", 0));
    m_cInput->setValue(config->readDoubleNumEntry("ContrastAjustment", 0.0));
    m_overExposureIndicatorBox->setChecked(config->readBoolEntry("OverExpoIndicator", false));

    for (int i = 0 ; i < 5 ; i++)
        m_curves->curvesChannelReset(i);

    m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
    m_curvesWidget->reset();

    for (int j = 0 ; j < 17 ; j++)
    {
        QPoint disable(-1, 0);
        QPoint p = config->readPointEntry(QString("CurveAjustmentPoint%1").arg(j), &disable);

        if (m_originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()*255);
            p.setY(p.y()*255);
        }

        m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, j, p);
    }

    for (int i = 0 ; i < 5 ; i++)
        m_curves->curvesCalculateCurve(i);

    m_curvesWidget->m_channelType = Digikam::CurvesWidget::ValueHistogram;
    m_curvesWidget->curveTypeChanged();
}

void ImageEffect_ICCProof::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management Tool");
    config->writeEntry("Settings Tab", m_toolBoxWidgets->currentIndex());
    config->writePathEntry("InputProfilePath", m_inProfilesPath->url());
    config->writePathEntry("ProofProfilePath", m_proofProfilePath->url());
    config->writePathEntry("SpaceProfilePath", m_spaceProfilePath->url());
    config->writeEntry("RenderingIntent", m_renderingIntentsCB->currentItem());
    config->writeEntry("DoSoftProof", m_doSoftProofBox->isChecked());
    config->writeEntry("CheckGamut", m_checkGamutBox->isChecked());
    config->writeEntry("EmbeddProfile", m_embeddProfileBox->isChecked());
    config->writeEntry("BPC", m_BPCBox->isChecked());
    config->writeEntry("InputProfileMethod", m_inProfileBG->selectedId());
    config->writeEntry("SpaceProfileMethod", m_spaceProfileBG->selectedId());
    config->writeEntry("ProofProfileMethod", m_proofProfileBG->selectedId());
    config->writeEntry("ContrastAjustment", m_cInput->value());
    config->writeEntry("OverExpoIndicator", m_overExposureIndicatorBox->isChecked());

    for (int j = 0 ; j < 17 ; j++)
    {
        QPoint p = m_curves->getCurvePoint(Digikam::ImageHistogram::ValueChannel, j);

        if (m_originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()/255);
            p.setY(p.y()/255);
        }

        config->writeEntry(QString("CurveAjustmentPoint%1").arg(j), p);
    }

    config->sync();
}

void ImageEffect_ICCProof::processLCMSURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_ICCProof::slotSpotColorChanged(const Digikam::DColor &color)
{
    m_curvesWidget->setCurveGuide(color);
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
    m_cInput->blockSignals(true);
    m_cInput->setValue(0.0);

    for (int i = 0 ; i < 5 ; i++)
       m_curves->curvesChannelReset(i);

    m_curvesWidget->reset();
    m_cInput->blockSignals(false);
    slotEffect();
}

void ImageEffect_ICCProof::slotEffect()
{
    kapp->setOverrideCursor(KCursor::waitCursor());
    enableButtonOK(true);
    m_histogramWidget->stopHistogramComputation();

    Digikam::IccTransform transform;

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface *iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int  w                     = iface->previewWidth();
    int  h                     = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);

    QString tmpInPath    = QString::null;
    QString tmpProofPath = QString::null;
    QString tmpSpacePath = QString::null;

    bool proofCondition = false;
    bool spaceCondition = false;
    
    //-- Input profile parameters ------------------
    
    if (useDefaultInProfile())
    {
        tmpInPath = m_inPath;
    }
    else if (useSelectedInProfile())
    {
        tmpInPath = m_inProfilesPath->url();
    }

    //-- Proof profile parameters ------------------

    if (useDefaultProofProfile())
    {
        tmpProofPath = m_proofPath;
    }
    else
    {
        tmpProofPath = m_proofProfilePath->url();
    }

    if (m_doSoftProofBox->isChecked())
        proofCondition = tmpProofPath.isEmpty();

    //-- Workspace profile parameters --------------

    if (useDefaultSpaceProfile())
    {
        tmpSpacePath = m_spacePath;
    }
    else
    {
        tmpSpacePath = m_spaceProfilePath->url();
    }

    spaceCondition = tmpSpacePath.isEmpty();

    //-- Perform the color transformations ------------------

    transform.getTransformType(m_doSoftProofBox->isChecked());

    if (m_doSoftProofBox->isChecked())
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.setProfiles( tmpSpacePath, tmpProofPath, true );
        }
        else
        {
            transform.setProfiles( tmpInPath, tmpSpacePath, tmpProofPath);
        }
    }
    else
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.setProfiles( tmpSpacePath );
        }
        else
        {
            transform.setProfiles( tmpInPath, tmpSpacePath );
        }
    }

    if ( proofCondition || spaceCondition )
    {
        kapp->restoreOverrideCursor();
        QString error = i18n("<p>Your settings are not sufficient.</p>"
                        "<p>To apply a color transform, you need at least two ICC profiles:</p>"
                        "<ul><li>An \"Input\" profile.</li>"
                        "<li>A \"Workspace\" profile.</li></ul>"
                        "<p>If you want to do a \"soft-proof\" transform, in addition to these profiles "
                        "you need a \"Proof\" profile.</p>");
        KMessageBox::information(this, error);
        enableButtonOK(false);
    }
    else
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.apply(preview, m_embeddedICC, m_renderingIntentsCB->currentItem(), useBPC(),
                            m_checkGamutBox->isChecked(), useBuiltinProfile());
        }
        else
        {
            QByteArray fakeProfile = QByteArray();
            transform.apply(preview, fakeProfile, m_renderingIntentsCB->currentItem(), useBPC(),
                            m_checkGamutBox->isChecked(), useBuiltinProfile());
        }
        
        //-- Calculate and apply the curve on image after transformation -------------
        
        Digikam::DImg preview2(w, h, sb, a, 0, false);
        m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel,
                                 m_overExposureIndicatorBox->isChecked());
        m_curves->curvesLutProcess(preview.bits(), preview2.bits(), w, h);
    
        //-- Adjust contrast ---------------------------------------------------------
        
        Digikam::BCGModifier cmod;
        cmod.setContrast(m_cInput->value() + (double)(1.00));
        cmod.applyBCG(preview2);

        iface->putPreviewImage(preview2.bits());
        m_previewWidget->updatePreview();

        //-- Update histogram --------------------------------------------------------
    
        memcpy(m_destinationPreviewData, preview2.bits(), preview2.numBytes());
        m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
        kapp->restoreOverrideCursor();
    }    
}

void ImageEffect_ICCProof::finalRendering()
{
    if (!m_doSoftProofBox->isChecked())
    {
        kapp->setOverrideCursor( KCursor::waitCursor() );
        
        Digikam::ImageIface *iface = m_previewWidget->imageIface();
        uchar *data                = iface->getOriginalImage();
        int w                      = iface->originalWidth();
        int h                      = iface->originalHeight();
        bool a                     = iface->originalHasAlpha();
        bool sb                    = iface->originalSixteenBit();

        if (data)
        {
            Digikam::IccTransform transform;
            
            Digikam::DImg img(w, h, sb, a, data);

            QString tmpInPath;
            QString tmpProofPath;
            QString tmpSpacePath;
            bool proofCondition;
            
            //-- Input profile parameters ------------------
            
            if (useDefaultInProfile())
            {
                tmpInPath = m_inPath;
            }
            else if (useSelectedInProfile())
            {
                tmpInPath = m_inProfilesPath->url();
            }

            if (tmpInPath.isNull() && !m_useEmbeddedProfile->isChecked() &&    
                !m_useSRGBDefaultProfile->isChecked())
            {
                KMessageBox::information(this, "Profile error");
                return;
            }
                
            //-- Proof profile parameters ------------------
        
            if (useDefaultProofProfile())
            {
                tmpProofPath = m_proofPath;
            }
            else
            {
                tmpProofPath = m_proofProfilePath->url();
            }

            if (tmpProofPath.isNull())
                proofCondition = false;
        
            //-- Workspace profile parameters --------------
        
            if (useDefaultSpaceProfile())
            {
                tmpSpacePath = m_spacePath;
            }
            else
            {
                tmpSpacePath = m_spaceProfilePath->url();
            }
        
            //-- Perform the color transformations ------------------
        
            transform.getTransformType(m_doSoftProofBox->isChecked());
        
            if (m_doSoftProofBox->isChecked())
            {
                if (m_useEmbeddedProfile->isChecked())
                {
                    transform.setProfiles( tmpSpacePath, tmpProofPath, true );
                }
                else
                {
                    transform.setProfiles( tmpInPath, tmpSpacePath, tmpProofPath);
                }
            }
            else
            {
                if (m_useEmbeddedProfile->isChecked())
                {
                    transform.setProfiles( tmpSpacePath );
                }
                else
                {
                    transform.setProfiles( tmpInPath, tmpSpacePath );
                }
            }
            
            if (m_useEmbeddedProfile->isChecked())
            {
                transform.apply(img, m_embeddedICC, m_renderingIntentsCB->currentItem(), useBPC(),
                                m_checkGamutBox->isChecked(), useBuiltinProfile());
            }
            else
            {
                QByteArray fakeProfile = QByteArray();
                transform.apply(img, fakeProfile, m_renderingIntentsCB->currentItem(), useBPC(),
                                m_checkGamutBox->isChecked(), useBuiltinProfile());
            }
    
            //-- Embed the workspace profile if necessary --------------------------------
   
            if (m_embeddProfileBox->isChecked())
            {
                iface->setEmbeddedICCToOriginalImage( tmpSpacePath );
                kdDebug() << k_funcinfo << QFile::encodeName(tmpSpacePath) << endl;
            }

            //-- Calculate and apply the curve on image after transformation -------------
        
            Digikam::DImg img2(w, h, sb, a, 0, false);
            m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel, false);
            m_curves->curvesLutProcess(img.bits(), img2.bits(), w, h);
        
            //-- Adjust contrast ---------------------------------------------------------
            
            Digikam::BCGModifier cmod;
            cmod.setContrast(m_cInput->value() + (double)(1.00));
            cmod.applyBCG(img2);
    
            iface->putOriginalImage("Color Management", img2.bits());
            delete [] data;
        }

        kapp->restoreOverrideCursor();
    }

    accept();
}

void ImageEffect_ICCProof::slotToggledWidgets( bool t)
{
    m_useInDefaultProfile->setEnabled(t);
    m_useProofDefaultProfile->setEnabled(t);
    m_useSpaceDefaultProfile->setEnabled(t);
}

void ImageEffect_ICCProof::slotInICCInfo()
{
    if (useEmbeddedProfile())
    {
        getICCInfo(m_embeddedICC);
    }
    else if(useBuiltinProfile())
    {
        QString message = i18n("<p>You have selected the \"Default builtin sRGB profile\"</p>");
        message.append(i18n("<p>This profile is built on the fly, so there is no relevant information "
                            "about it.</p>"));
        KMessageBox::information(this, message);
    }
    else if (useDefaultInProfile())
    {
        getICCInfo(m_inPath);
    }
    else if (useSelectedInProfile())
    {
        getICCInfo(m_inProfilesPath->url());
    }
}

void ImageEffect_ICCProof::slotProofICCInfo()
{
    if (useDefaultProofProfile())
    {
        getICCInfo(m_proofPath);
    }
    else
    {
        getICCInfo(m_proofProfilePath->url());
    }
}

void ImageEffect_ICCProof::slotSpaceICCInfo()
{
    if (useDefaultSpaceProfile())
    {
        getICCInfo(m_spacePath);
    }
    else
    {
        getICCInfo(m_spaceProfilePath->url());
    }
}

void ImageEffect_ICCProof::getICCInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(this, i18n("Sorry, there is no selected profile"), i18n("Profile Error"));
        return;
    }
    
    Digikam::ICCProfileInfoDlg infoDlg(this, profile);
    infoDlg.exec();
}

void ImageEffect_ICCProof::getICCInfo(const QByteArray& profile)
{
    if (profile.isNull())
    {
        KMessageBox::error(this, i18n("Sorry, it seems there is no embedded profile"), i18n("Profile Error"));
        return;
    }

    Digikam::ICCProfileInfoDlg infoDlg(this, QString::null, profile);
    infoDlg.exec();
}

void ImageEffect_ICCProof::slotCMDisabledWarning()
{
    if (!m_cmEnabled)
    {
        QString message = i18n("<p>You do not have enabled Color Management in digiKam preferences.</p>");
        message.append( i18n("<p>\"Use of default profile\" options will be disabled now.</p>"));
        KMessageBox::information(this, message);
        slotToggledWidgets(false);
    }
}

//-- General Tab ---------------------------

bool ImageEffect_ICCProof::useBPC()
{
    return m_BPCBox->isChecked();
}

bool ImageEffect_ICCProof::doProof()
{
    return m_doSoftProofBox->isChecked();
}

bool ImageEffect_ICCProof::checkGamut()
{
    return m_checkGamutBox->isChecked();
}

bool ImageEffect_ICCProof::embedProfile()
{
    return m_embeddProfileBox->isChecked();
}

//-- Input Tab ---------------------------

bool ImageEffect_ICCProof::useEmbeddedProfile()
{
    return m_useEmbeddedProfile->isChecked();
}

bool ImageEffect_ICCProof::useBuiltinProfile()
{
    return m_useSRGBDefaultProfile->isChecked();
}

bool ImageEffect_ICCProof::useDefaultInProfile()
{
    return m_useInDefaultProfile->isChecked();
}

bool ImageEffect_ICCProof::useSelectedInProfile()
{
    return m_useInSelectedProfile->isChecked();
}

//-- Workspace Tab ---------------------------

bool ImageEffect_ICCProof::useDefaultSpaceProfile()
{
    return m_useSpaceDefaultProfile->isChecked();
}

//-- Proofing Tab ---------------------------

bool ImageEffect_ICCProof::useDefaultProofProfile()
{
    return m_useProofDefaultProfile->isChecked();
}

//-- Load all settings from file --------------------------------------

void ImageEffect_ICCProof::slotUser3()
{
    KURL loadColorManagementFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                                i18n("%1|Color profile file").arg("*.icc *.icm|"), this,
                                                QString( i18n("Color Management Settings File to Load")) );
    if( loadColorManagementFile.isEmpty() )
       return;

    QFile file(loadColorManagementFile.path());
    
    if ( file.open(IO_ReadOnly) )   
    {
        QTextStream stream( &file );

        if ( stream.readLine() != "# Color Management Configuration File" )
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Color Management settings text file.")
                        .arg(loadColorManagementFile.fileName()));
           file.close();            
           return;
        }
        
        blockSignals(true);
        
        m_renderingIntentsCB->setCurrentItem( stream.readLine().toInt() );
        m_doSoftProofBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_checkGamutBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_embeddProfileBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_BPCBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_inProfileBG->setButton( stream.readLine().toInt() );
        m_spaceProfileBG->setButton( stream.readLine().toInt() );
        m_proofProfileBG->setButton( stream.readLine().toInt() );
        m_inProfilesPath->setURL( stream.readLine() );
        m_proofProfilePath->setURL( stream.readLine() );
        m_spaceProfilePath->setURL( stream.readLine() );
        m_cInput->setValue( stream.readLine().toDouble() );

        for (int i = 0 ; i < 5 ; i++)
            m_curves->curvesChannelReset(i);

        m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
        m_curvesWidget->reset();

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint disable(-1, -1);
            QPoint p;
            p.setX( stream.readLine().toInt() );
            p.setY( stream.readLine().toInt() );
    
            if (m_originalImage->sixteenBit() && p != disable)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }
    
            m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, j, p);
        }

        blockSignals(false);

        for (int i = 0 ; i < 5 ; i++)
           m_curves->curvesCalculateCurve(i);

        m_curvesWidget->m_channelType = Digikam::CurvesWidget::ValueHistogram;
        m_curvesWidget->curveTypeChanged();

        m_histogramWidget->reset();
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Color Management text file."));

    file.close();            
}

//-- Save all settings to file ---------------------------------------

void ImageEffect_ICCProof::slotUser2()
{
    KURL saveColorManagementFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                                QString( "*" ), this,
                                                QString( i18n("Color Management Settings File to Save")) );
    if( saveColorManagementFile.isEmpty() )
       return;

    QFile file(saveColorManagementFile.path());
    
    if ( file.open(IO_WriteOnly) )   
    {
        QTextStream stream( &file );        
        stream << "# Color Management Configuration File\n";    
        stream << m_renderingIntentsCB->currentItem() << "\n";    
        stream << m_doSoftProofBox->isChecked() << "\n";    
        stream << m_checkGamutBox->isChecked() << "\n";    
        stream << m_embeddProfileBox->isChecked() << "\n";    
        stream << m_BPCBox->isChecked() << "\n";    
        stream << m_inProfileBG->selectedId() << "\n";    
        stream << m_spaceProfileBG->selectedId() << "\n";    
        stream << m_proofProfileBG->selectedId() << "\n";    
        stream << m_inProfilesPath->url() << "\n";    
        stream << m_proofProfilePath->url() << "\n";    
        stream << m_spaceProfilePath->url() << "\n";    
        stream << m_cInput->value() << "\n";    

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint p = m_curves->getCurvePoint(Digikam::ImageHistogram::ValueChannel, j);
            if (m_originalImage->sixteenBit())
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
            }
            stream << p.x() << "\n";
            stream << p.y() << "\n";
        }
    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Color Management text file."));
    
    file.close();        
}

} // NameSpace DigikamImagesPluginCore

#include "imageeffect_iccproof.moc"
