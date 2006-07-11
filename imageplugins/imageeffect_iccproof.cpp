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

#include <config.h>
 
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

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "iccpreviewwidget.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"

// Lcms includes.

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// Local includes.

#include "imageeffect_iccproof.h"

namespace DigikamImagesPluginCore
{

ImageEffect_ICCProof::ImageEffect_ICCProof(QWidget* parent)
                    : Digikam::ImageDlgBase(parent,i18n("Color Management"), 
                                            "colormanagement", true, true)
{
    m_destinationPreviewData = 0L;

    m_cmEnabled   = true;
    m_hasICC      = false;
    m_inPath      = QString::null;
    m_spacePath   = QString::null;
    m_proofPath   = QString::null;
    m_displayPath = QString::null;

    setHelp("colormanagement.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget("colormanagement Tool Dialog", plainPage(),
                                               i18n("<p>Here you can see the image preview after "
                                                    "convert it with a color profile</p>"));
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
                                             "drawing of the "
                                             "selected image channel. This one is re-computed at any "
                                             "settings changes."));
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, 
                                                    gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    gridSettings->addMultiCellWidget(m_histogramWidget, 1, 1, 0, 2);
    gridSettings->addMultiCellWidget(m_hGradient, 2, 2, 0, 2);

    // -------------------------------------------------------------

    m_toolBoxWidgets         = new QToolBox(gboxSettings);
    QWidget *generalOptions  = new QWidget(m_toolBoxWidgets);
    QWidget *inProfiles      = new QWidget(m_toolBoxWidgets);
    QWidget *proofProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *displayProfiles = new QWidget(m_toolBoxWidgets);
    QWidget *spaceProfiles   = new QWidget(m_toolBoxWidgets);

    //---------- "General" Page Setup ----------------------------------
        
    m_toolBoxWidgets->insertItem(GENERALPAGE, generalOptions, 
                                 SmallIconSet("misc"), i18n("General Settings"));
    QWhatsThis::add(generalOptions, i18n("<p>You can set here general parameters.</p>"));

    QGridLayout *zeroPageLayout = new QGridLayout(generalOptions, 5, 1, 
                                  KDialog::marginHint(), KDialog::spacingHint());
                                   
    m_doSoftProofBox = new QCheckBox(generalOptions);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    QWhatsThis::add(m_doSoftProofBox, i18n("<p>The obtained transform emulates the device described "
                                           "by the \"Proofing\" profile. Useful to preview final "
                                           "result without rendering to physical medium.</p>"));

    m_checkGamutBox = new QCheckBox(generalOptions);
    m_checkGamutBox->setText(i18n("Check gamut"));
    QWhatsThis::add(m_checkGamutBox, i18n("<p>You can use this option if you want to show "
                                          "the colors that are out of the printer gamut<p>"));

    m_embeddProfileBox = new QCheckBox(generalOptions);
    m_embeddProfileBox->setChecked(true);
    m_embeddProfileBox->setText(i18n("Embed profile"));
    QWhatsThis::add(m_embeddProfileBox, i18n("<p>You can use this option if you want to embed "
                                             "into the image the selected work-space color profile.</p>"));

    m_BPCBox = new QCheckBox(generalOptions);
    m_BPCBox->setText(i18n("Use BPC"));
    QWhatsThis::add(m_BPCBox, i18n("<p>The Black Point Compensation (BPC) feature does work in conjunction "
                                   "with relative colorimetric intent. Perceptual intent should make no "
                                   "difference, although it affects some profiles.</p>"
                                   "<p>The mechanics are simple. BPC does scale full image across gray "
                                   "axis in order to accommodate the darkest tone origin media can "
                                   "render to darkest tone destination media can render. As a such, "
                                   "BPC is primarily targeting CMYK.</p>"));

    QLabel *intent = new QLabel(i18n("Rendering Intent:"), generalOptions);
    m_renderingIntentsCB = new QComboBox(false, generalOptions);
    m_renderingIntentsCB->insertItem("Perceptual");
    m_renderingIntentsCB->insertItem("Absolute Colorimetric");
    m_renderingIntentsCB->insertItem("Relative Colorimetric");
    m_renderingIntentsCB->insertItem("Saturation");
    QWhatsThis::add( m_renderingIntentsCB, i18n("<ul><li>Perceptual intent causes the full gamut "
                "of the image to be compressed or expanded to fill the gamut of the destination device, "
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

    QGridLayout *firstPageLayout = new QGridLayout(inProfiles, 3, 2, 
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

    firstPageLayout->addMultiCellWidget(m_inProfileBG, 0, 1, 0, 0);    
    firstPageLayout->addMultiCellWidget(inProfilesInfo, 0, 0, 2, 2);    
    firstPageLayout->addMultiCellWidget(m_inProfilesPath, 2, 2, 0, 2);    
    firstPageLayout->setColStretch(1, 10);
    firstPageLayout->setRowStretch(3, 10);

    //---------- "Workspace" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(WORKSPACEPAGE, spaceProfiles, 
                                 SmallIconSet("tablet"), i18n("Work-space Profile"));
    QWhatsThis::add(spaceProfiles, i18n("<p>Set here all parameters relevant of Workspace Color "
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
    QWhatsThis::add(proofProfiles, i18n("<p>Set here all parameters relevant of Proofing Color "
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

    //---------- "Display" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(DISPLAYPAGE, displayProfiles, 
                                 SmallIconSet("system"), i18n("Display Profile"));
    QWhatsThis::add(displayProfiles, i18n("<p>Set here all parameters relevant of Display Color "
                    "Profiles.</p>"));


    QGridLayout *fourthPageLayout = new QGridLayout(displayProfiles, 3, 2, 
                                    KDialog::marginHint(), KDialog::spacingHint());

    m_displayProfileBG = new QButtonGroup(2, Qt::Vertical, displayProfiles);
    m_displayProfileBG->setFrameStyle(QFrame::NoFrame);
    m_displayProfileBG->setInsideMargin(0);
    
    m_useDisplayDefaultProfile = new QRadioButton(m_displayProfileBG);
    m_useDisplayDefaultProfile->setText(i18n("Use default display profile"));

    m_useDisplaySelectedProfile = new QRadioButton(m_displayProfileBG);
    m_useDisplaySelectedProfile->setText(i18n("Use selected profile"));

    m_displayProfilePath = new KURLRequester(displayProfiles);
    m_displayProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    m_displayProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *displayProfiles_dialog = m_displayProfilePath->fileDialog();
    m_iccDisplayPreviewWidget = new Digikam::ICCPreviewWidget(displayProfiles_dialog);
    displayProfiles_dialog->setPreviewWidget(m_iccDisplayPreviewWidget);
 
    QPushButton *displayProfilesInfo = new QPushButton(i18n("Info..."), displayProfiles);

    fourthPageLayout->addMultiCellWidget(m_displayProfileBG, 0, 1, 0, 0);    
    fourthPageLayout->addMultiCellWidget(displayProfilesInfo, 0, 0, 2, 2);    
    fourthPageLayout->addMultiCellWidget(m_displayProfilePath, 2, 2, 0, 2);    
    fourthPageLayout->setColStretch(1, 10);
    fourthPageLayout->setRowStretch(3, 10);
    
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

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));    
            
    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(inProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotInICCInfo()));

    connect(spaceProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotSpaceICCInfo()));

    connect(proofProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotProofICCInfo()));

    connect(displayProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotDisplayICCInfo()));

    // -------------------------------------------------------------

    enableButtonOK( false );
    readSettings();
}

ImageEffect_ICCProof::~ImageEffect_ICCProof()
{
    writeSettings();
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
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
        m_displayPath = config->readPathEntry("MonitorProfileFile");
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
    m_displayProfilePath->setURL(config->readPathEntry("DisplayProfilePath", defaultICCPath)); 
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
    m_displayProfileBG->setButton(config->readNumEntry("DisplayProfileMethod", 0));
}

void ImageEffect_ICCProof::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management Tool");
    config->writeEntry("Settings Tab", m_toolBoxWidgets->currentIndex());
    config->writePathEntry("DisplayProfilePath", m_displayProfilePath->url());
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
    config->writeEntry("DisplayProfileMethod", m_displayProfileBG->selectedId());
    config->sync();
}

void ImageEffect_ICCProof::processLCMSURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
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

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface *iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
    if (iface->getEmbeddedICCFromOriginalImage().isNull())
    {
        m_useEmbeddedProfile->setEnabled(false);
    }
    else
    {
        m_hasICC = true;
        m_embeddedICC = iface->getEmbeddedICCFromOriginalImage();
    }

    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
   
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();    
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
            
            ///////////////////////////////////////
            // Get parameters for transformation //
            ///////////////////////////////////////
            
            //---------Input profile ------------------
            
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
                kdDebug() << "here" << endl;
                return;
            }
                
            //--------Proof profile ------------------
        
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
        
            //-------Workspace profile--------------
        
            if (useDefaultSpaceProfile())
            {
                tmpSpacePath = m_spacePath;
            }
            else
            {
                tmpSpacePath = m_spaceProfilePath->url();
            }
        
            //------------------------------------------
        
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
            
            if (m_embeddProfileBox->isChecked())
            {
                iface->setEmbeddedICCToOriginalImage( tmpSpacePath );
                kdDebug() << k_funcinfo << QFile::encodeName(tmpSpacePath) << endl;
            }
            
            iface->putOriginalImage("Color Management", img.bits());
            delete [] data;
        }
        kapp->restoreOverrideCursor();
    }
    accept();
}

void ImageEffect_ICCProof::slotTry()
{
    // TODO use of Display profile is not implemented -- Paco Cruz
    
    kapp->setOverrideCursor(KCursor::waitCursor());

    enableButtonOK(true);

    m_histogramWidget->stopHistogramComputation();

    Digikam::IccTransform transform;

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    Digikam::ImageIface *iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);

    QString tmpInPath    = QString::null;
    QString tmpProofPath = QString::null;
    QString tmpSpacePath = QString::null;

    bool proofCondition = false;
    bool spaceCondition = false;
    
    ///////////////////////////////////////
    // Get parameters for transformation //
    ///////////////////////////////////////
    
    //---------Input profile ------------------
    
    if (useDefaultInProfile())
    {
        tmpInPath = m_inPath;
    }
    else if (useSelectedInProfile())
    {
        tmpInPath = m_inProfilesPath->url();
    }

    //--------Proof profile ------------------

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

    //-------Workspace profile--------------

    if (useDefaultSpaceProfile())
    {
        tmpSpacePath = m_spacePath;
    }
    else
    {
        tmpSpacePath = m_spaceProfilePath->url();
    }

    spaceCondition = tmpSpacePath.isEmpty();

    //------------------------------------------

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
        QString error = i18n("<p>Your settings are not correct</p>"
                        "<p>To apply a color transform, you need at least two ICC profiles:</p>"
                        "<ul><li>An \"Input\" profile.</li>"
                        "<li>A \"Workspace\" profile.</li></ul>"
                        "<p>If you want to do a \"soft-proof\" transform, in adition to these profiles "
                        "you need a \"Proof\" one.</p>");
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
        
        iface->putPreviewImage(preview.bits());
    
        m_previewWidget->updatePreview();
    
        // Update histogram.
    
        memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
        kdDebug() << "Doing updateData" << endl;
        m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
        kapp->restoreOverrideCursor();
    }    
}

void ImageEffect_ICCProof::slotToggledWidgets( bool t)
{
    m_useInDefaultProfile->setEnabled(t);
    m_useProofDefaultProfile->setEnabled(t);
    m_useDisplayDefaultProfile->setEnabled(t);
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
        message.append(i18n("<p>This profile is built on the fly, so there is not relevant information "
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

void ImageEffect_ICCProof::slotDisplayICCInfo()
{
    if (useDefaultDisplayProfile())
    {
        getICCInfo(m_displayPath);
    }
    else
    {
        getICCInfo(m_displayProfilePath->url());
    }
}

void ImageEffect_ICCProof::getICCInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(this, i18n("Sorry, there is not any selected profile"), i18n("Profile Error"));
        return;
    }
    
    Digikam::ICCProfileInfoDlg infoDlg(this, profile);
    infoDlg.exec();
}

void ImageEffect_ICCProof::getICCInfo(QByteArray& profile)
{
    if (profile.isNull())
    {
        KMessageBox::error(this, i18n("Sorry, it seems there is no embedded profile"), i18n("Profile Error"));
        return;
    }
    QString intent;
    cmsHPROFILE selectedProfile = cmsOpenProfileFromMem(profile.data(), (DWORD)profile.size());
    QString profileName         = QString((cmsTakeProductName(selectedProfile)));
    QString profileDescription  = QString((cmsTakeProductDesc(selectedProfile)));
    QString profileManufacturer = QString(cmsTakeCopyright(selectedProfile));
    int profileIntent           = cmsTakeRenderingIntent(selectedProfile);
    
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

    QString message = i18n("<p><b>Name:</b> ");
    message.append(profileName);
    message.append(i18n("</p><p><b>Description:</b>  "));
    message.append(profileDescription);
    message.append(i18n("</p><p><b>Copyright:</b>  "));
    message.append(profileManufacturer);
    message.append(i18n("</p><p><b>Rendering Intent:</b>  "));
    message.append(intent);
    message.append(i18n("</p><p><b>Path:</b> Embedded profile</p>"));
    KMessageBox::information(this, message);
}

void ImageEffect_ICCProof::slotCMDisabledWarning()
{
    if (!m_cmEnabled)
    {
        QString message = i18n("<p>You don't have enabled Color Management in digiKam preferences.</p>");
        message.append( i18n("<p>\"Use default profile\" options will be disabled now.</p>"));
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

//-- Display Tab ---------------------------

bool ImageEffect_ICCProof::useDefaultDisplayProfile()
{
    return m_useDisplayDefaultProfile->isChecked();
}

// Load all settings from file.
void ImageEffect_ICCProof::slotUser3()
{
    KURL loadColorManagementFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                                QString( "*" ), this,
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
        m_displayProfileBG->setButton( stream.readLine().toInt() );
        m_displayProfilePath->setURL( stream.readLine() );
        m_inProfilesPath->setURL( stream.readLine() );
        m_proofProfilePath->setURL( stream.readLine() );
        m_spaceProfilePath->setURL( stream.readLine() );
        
        m_histogramWidget->reset();
        blockSignals(false);
        slotEffect();  
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Color Management text file."));

    file.close();            
}

// Save all settings to file.
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
        stream << m_displayProfileBG->selectedId() << "\n";    
        stream << m_displayProfilePath->url() << "\n";    
        stream << m_inProfilesPath->url() << "\n";    
        stream << m_proofProfilePath->url() << "\n";    
        stream << m_spaceProfilePath->url() << "\n";    
    }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the White Color Balance text file."));
    
    file.close();        
}

}// NameSpace DigikamImagesPluginCore

#include "imageeffect_iccproof.moc"
