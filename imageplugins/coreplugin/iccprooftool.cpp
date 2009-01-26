/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-21
 * Description : digiKam image editor tool to correct picture
 *               colors using an ICC color profile
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextstream.h>
#include <qtoolbox.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qvbuttongroup.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfile.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <kurllabel.h>
#include <kurlrequester.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Digikam includes.

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "ddebug.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "iccpreviewwidget.h"
#include "iccprofileinfodlg.h"
#include "icctransform.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"

// Local includes.

#include "iccprooftool.h"
#include "iccprooftool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

ICCProofTool::ICCProofTool(QObject* parent)
            : EditorTool(parent)
{
    setName("colormanagement");
    setToolName(i18n("Color Management"));
    setToolIcon(SmallIcon("colormanagement"));
    setToolHelp("colormanagement.anchor");

    m_destinationPreviewData = 0;
    m_cmEnabled              = true;
    m_hasICC                 = false;

    ImageIface iface(0, 0);
    m_originalImage = iface.getOriginalImg();
    m_embeddedICC   = iface.getEmbeddedICCFromOriginalImage();

    m_previewWidget = new ImageWidget("colormanagement Tool",0,
                                      i18n("<p>Here you can see the image preview after "
                                           "applying a color profile</p>"));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout *gridSettings = new QGridLayout(m_gboxSettings->plainPage(), 3, 2);

    QLabel *label1 = new QLabel(i18n("Channel: "), m_gboxSettings->plainPage());
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(false, m_gboxSettings->plainPage());
    m_channelCB->insertItem(i18n("Luminosity"));
    m_channelCB->insertItem(i18n("Red"));
    m_channelCB->insertItem(i18n("Green"));
    m_channelCB->insertItem(i18n("Blue"));
    QWhatsThis::add( m_channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red channel values.<p>"
                                       "<b>Green</b>: display the green channel values.<p>"
                                       "<b>Blue</b>: display the blue channel values.<p>"));

    m_scaleBG = new QHButtonGroup(m_gboxSettings->plainPage());
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin(0);
    QWhatsThis::add(m_scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                    "If the image's maximal values are small, you can use the linear scale.<p>"
                                    "Logarithmic scale can be used when the maximal values are big; "
                                    "if it is used, all values (small and large) will be visible on the "
                                    "graph."));

    QPushButton *linHistoButton = new QPushButton(m_scaleBG);
    QToolTip::add(linHistoButton, i18n("<p>Linear"));
    m_scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap(QPixmap(directory + "histogram-lin.png"));
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton(m_scaleBG);
    QToolTip::add(logHistoButton, i18n("<p>Logarithmic"));
    m_scaleBG->insert(logHistoButton, HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap(QPixmap(directory + "histogram-log.png"));
    logHistoButton->setToggleButton(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(m_scaleBG);

    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 2);

    // -------------------------------------------------------------

    QVBox *histoBox   = new QVBox(m_gboxSettings->plainPage());
    m_histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram "
                                             "of the selected image channel. "
                                             "This one is updated after setting changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10,
                                                    histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    gridSettings->addMultiCellWidget(histoBox, 1, 2, 0, 2);

    // -------------------------------------------------------------

    m_toolBoxWidgets         = new QToolBox(m_gboxSettings->plainPage());
    QWidget *generalOptions  = new QWidget(m_toolBoxWidgets);
    QWidget *inProfiles      = new QWidget(m_toolBoxWidgets);
    QWidget *spaceProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *proofProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *lightnessadjust = new QWidget(m_toolBoxWidgets);

    //---------- "General" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(GENERALPAGE, generalOptions,
                                 SmallIconSet("misc"), i18n("General Settings"));
    QWhatsThis::add(generalOptions, i18n("<p>Here you can set general parameters.</p>"));

    QGridLayout *zeroPageLayout = new QGridLayout(generalOptions, 5, 1);

    m_doSoftProofBox = new QCheckBox(generalOptions);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    QWhatsThis::add(m_doSoftProofBox, i18n("<p>Rendering emulation of the device described "
                                           "by the \"Proofing\" profile. Useful to preview the final "
                                           "result without rendering to physical medium.</p>"));

    m_checkGamutBox = new QCheckBox(generalOptions);
    m_checkGamutBox->setText(i18n("Check gamut"));
    QWhatsThis::add(m_checkGamutBox, i18n("<p>You can use this option if you want to show "
                                          "the colors that are outside the printer's gamut<p>"));

    m_embeddProfileBox = new QCheckBox(generalOptions);
    m_embeddProfileBox->setChecked(true);
    m_embeddProfileBox->setText(i18n("Assign profile"));
    QWhatsThis::add(m_embeddProfileBox, i18n("<p>You can use this option to embed "
                                             "the selected workspace color profile into the image.</p>"));

    m_BPCBox = new QCheckBox(generalOptions);
    m_BPCBox->setText(i18n("Use BPC"));
    QWhatsThis::add(m_BPCBox, i18n("<p>The Black Point Compensation (BPC) feature does work in conjunction "
                                   "with Relative Colorimetric Intent. Perceptual intent should make no "
                                   "difference, since BPC is always on, and in Absolute Colorimetric "
                                   "Intent it is always turned off.</p>"
                                   "<p>BPC does compensate for a lack of ICC profiles in the dark tone rendering. "
                                   "With BPC the dark tones are optimally mapped (no clipping) from original media "
                                   "to the destination rendering media, e.g. the combination of paper and ink.</p>"));

    QLabel *intent = new QLabel(i18n("Rendering Intent:"), generalOptions);
    m_renderingIntentsCB = new RComboBox(generalOptions);
    m_renderingIntentsCB->insertItem("Perceptual");
    m_renderingIntentsCB->insertItem("Absolute Colorimetric");
    m_renderingIntentsCB->insertItem("Relative Colorimetric");
    m_renderingIntentsCB->insertItem("Saturation");
    m_renderingIntentsCB->setDefaultItem(0);
    QWhatsThis::add( m_renderingIntentsCB, i18n("<ul><li>Perceptual intent causes the full gamut "
                "of the image to be compressed or expanded to fill the gamut of the destination media, "
                "so that gray balance is preserved but colorimetric accuracy may not be preserved.<br>"
                "In other words, if certain colors in an image fall outside of the range of colors that "
                "the output device can render, the image intent will cause all the colors in the image "
                "to be adjusted so that every color in the image falls within the range that can be "
                "rendered and so that the relationship between colors is preserved as much as possible.<br>"
                "This intent is most suitable for display of photographs and images, and is the default "
                "intent.</li>"
                "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the "
                "output device can render to be adjusted to the closest color that can be rendered, while all "
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
    lcmsLogoLabel->setAlignment(AlignTop | AlignRight);
    lcmsLogoLabel->setText(QString());
    lcmsLogoLabel->setURL("http://www.littlecms.com");
    KGlobal::dirs()->addResourceType("logo-lcms", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("logo-lcms", "logo-lcms.png");
    lcmsLogoLabel->setPixmap(QPixmap(directory + "logo-lcms.png"));
    QToolTip::add(lcmsLogoLabel, i18n("Visit Little CMS project website"));

    zeroPageLayout->addMultiCellWidget(m_doSoftProofBox,        0, 0, 0, 0);
    zeroPageLayout->addMultiCellWidget(m_checkGamutBox,         1, 1, 0, 0);
    zeroPageLayout->addMultiCellWidget(m_embeddProfileBox,      2, 2, 0, 0);
    zeroPageLayout->addMultiCellWidget(lcmsLogoLabel,           0, 2, 1, 1);
    zeroPageLayout->addMultiCellWidget(m_BPCBox,                3, 3, 0, 0);
    zeroPageLayout->addMultiCellWidget(intent,                  4, 4, 0, 0);
    zeroPageLayout->addMultiCellWidget(m_renderingIntentsCB,    4, 4, 1, 1);
    zeroPageLayout->setRowStretch(5, 10);

    //---------- "Input" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(INPUTPAGE, inProfiles, SmallIconSet("camera"), i18n("Input Profile"));
    QWhatsThis::add(inProfiles, i18n("<p>Set here all parameters relevant of Input Color "
                    "Profiles.</p>"));

    QGridLayout *firstPageLayout = new QGridLayout(inProfiles, 4, 2);

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
    m_iccInPreviewWidget = new ICCPreviewWidget(inProfiles_dialog);
    inProfiles_dialog->setPreviewWidget(m_iccInPreviewWidget);

    QPushButton *inProfilesInfo = new QPushButton(i18n("Info..."), inProfiles);

    QGroupBox *pictureInfo = new QGroupBox(2, Qt::Horizontal, i18n("Camera information"), inProfiles);
    new QLabel(i18n("Make:"), pictureInfo);
    KSqueezedTextLabel *make  = new KSqueezedTextLabel(0, pictureInfo);
    new QLabel(i18n("Model:"), pictureInfo);
    KSqueezedTextLabel *model = new KSqueezedTextLabel(0, pictureInfo);
    make->setText(iface.getPhotographInformations().make);
    model->setText(iface.getPhotographInformations().model);

    firstPageLayout->addMultiCellWidget(m_inProfileBG,      0, 1, 0, 0);
    firstPageLayout->addMultiCellWidget(inProfilesInfo,     0, 0, 2, 2);
    firstPageLayout->addMultiCellWidget(m_inProfilesPath,   2, 2, 0, 2);
    firstPageLayout->addMultiCellWidget(pictureInfo,        3, 3, 0, 2);
    firstPageLayout->setColStretch(1, 10);
    firstPageLayout->setRowStretch(4, 10);

    //---------- "Workspace" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(WORKSPACEPAGE, spaceProfiles,
                                 SmallIconSet("tablet"), i18n("Workspace Profile"));
    QWhatsThis::add(spaceProfiles, i18n("<p>Set here all parameters relevant to Color Workspace "
                    "Profiles.</p>"));

    QGridLayout *secondPageLayout = new QGridLayout(spaceProfiles, 3, 2);

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
    m_iccSpacePreviewWidget = new ICCPreviewWidget(spaceProfiles_dialog);
    spaceProfiles_dialog->setPreviewWidget(m_iccSpacePreviewWidget);

    QPushButton *spaceProfilesInfo = new QPushButton(i18n("Info..."), spaceProfiles);

    secondPageLayout->addMultiCellWidget(m_spaceProfileBG,      0, 1, 0, 0);
    secondPageLayout->addMultiCellWidget(spaceProfilesInfo,     0, 0, 2, 2);
    secondPageLayout->addMultiCellWidget(m_spaceProfilePath,    2, 2, 0, 2);
    secondPageLayout->setColStretch(1, 10);
    secondPageLayout->setRowStretch(3, 10);

    //---------- "Proofing" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(PROOFINGPAGE, proofProfiles,
                                 SmallIconSet("printer1"), i18n("Proofing Profile"));
    QWhatsThis::add(proofProfiles, i18n("<p>Set here all parameters relevant to Proofing Color "
                    "Profiles.</p>"));

    QGridLayout *thirdPageLayout = new QGridLayout(proofProfiles, 3, 2);

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
    m_iccProofPreviewWidget = new ICCPreviewWidget(proofProfiles_dialog);
    proofProfiles_dialog->setPreviewWidget(m_iccProofPreviewWidget);

    QPushButton *proofProfilesInfo = new QPushButton(i18n("Info..."), proofProfiles);

    thirdPageLayout->addMultiCellWidget(m_proofProfileBG,   0, 1, 0, 0);
    thirdPageLayout->addMultiCellWidget(proofProfilesInfo,  0, 0, 2, 2);
    thirdPageLayout->addMultiCellWidget(m_proofProfilePath, 2, 2, 0, 2);
    thirdPageLayout->setColStretch(1, 10);
    thirdPageLayout->setRowStretch(3, 10);

    //---------- "Lightness" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(LIGHTNESSPAGE, lightnessadjust,
                                 SmallIconSet("blend"), i18n("Lightness Adjustments"));
    QWhatsThis::add(lightnessadjust, i18n("<p>Set here all lightness adjustments to the target image.</p>"));

    QGridLayout *fourPageLayout = new QGridLayout( lightnessadjust, 5, 2);

    ColorGradientWidget* vGradient = new ColorGradientWidget(ColorGradientWidget::Vertical,
                                                             10, lightnessadjust );
    vGradient->setColors(QColor("white"), QColor("black"));

    QLabel *spacev = new QLabel(lightnessadjust);
    spacev->setFixedWidth(1);

    m_curvesWidget = new CurvesWidget(256, 192, m_originalImage->bits(), m_originalImage->width(),
                                               m_originalImage->height(), m_originalImage->sixteenBit(),
                                               lightnessadjust);
    QWhatsThis::add( m_curvesWidget, i18n("<p>This is the curve adjustment of the image luminosity"));

    QLabel *spaceh = new QLabel(lightnessadjust);
    spaceh->setFixedHeight(1);

    ColorGradientWidget *hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal,
                                                             10,
                                                             lightnessadjust);

    hGradient->setColors(QColor("black"), QColor("white"));

    m_cInput = new RIntNumInput(lightnessadjust);
    m_cInput->input()->setLabel(i18n("Contrast:"), AlignLeft | AlignVCenter);
    m_cInput->setRange(-100, 100, 1);
    m_cInput->setDefaultValue(0);
    QWhatsThis::add( m_cInput, i18n("<p>Set here the contrast adjustment of the image."));

    fourPageLayout->addMultiCellWidget(vGradient,       0, 0, 0, 0);
    fourPageLayout->addMultiCellWidget(spacev,          0, 0, 1, 1);
    fourPageLayout->addMultiCellWidget(m_curvesWidget,  0, 0, 2, 2);
    fourPageLayout->addMultiCellWidget(spaceh,          1, 1, 2, 2);
    fourPageLayout->addMultiCellWidget(hGradient,       2, 2, 2, 2);
    fourPageLayout->addMultiCellWidget(m_cInput,        4, 4, 0, 2);
//    fourPageLayout->setRowSpacing(3);
    fourPageLayout->setRowStretch(5, 10);

    // -------------------------------------------------------------

    gridSettings->addMultiCellWidget(m_toolBoxWidgets, 3, 3, 0, 2);

    setToolSettings(m_gboxSettings);
    m_gboxSettings->enableButton(EditorToolSettings::Ok, false);
    init();

    // -------------------------------------------------------------

    connect(lcmsLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processLCMSURL(const QString&)));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(m_cInput, SIGNAL(valueChanged (int)),
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
}

ICCProofTool::~ICCProofTool()
{
    if (m_destinationPreviewData)
        delete [] m_destinationPreviewData;
}

void ICCProofTool::readSettings()
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
            QString message = i18n("The ICC profiles path seems to be invalid. You won't be able to use the \"Default profile\"\
                                    options.<p>Please fix this in the digiKam ICC setup.");
            slotToggledWidgets( false );
            KMessageBox::information(kapp->activeWindow(), message);
        }
    }

    // Plugin settings.
    config->setGroup("colormanagement Tool");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0)); // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    m_toolBoxWidgets->setCurrentIndex(config->readNumEntry("Settings Tab", GENERALPAGE));
    m_inProfilesPath->setURL(config->readPathEntry("InputProfilePath", defaultICCPath));
    m_proofProfilePath->setURL(config->readPathEntry("ProofProfilePath", defaultICCPath));
    m_spaceProfilePath->setURL(config->readPathEntry("SpaceProfilePath", defaultICCPath));
    m_renderingIntentsCB->setCurrentItem(config->readNumEntry("RenderingIntent", m_renderingIntentsCB->defaultItem()));
    m_doSoftProofBox->setChecked(config->readBoolEntry("DoSoftProof", false));
    m_checkGamutBox->setChecked(config->readBoolEntry("CheckGamut", false));
    m_embeddProfileBox->setChecked(config->readBoolEntry("EmbeddProfile", true));
    m_BPCBox->setChecked(config->readBoolEntry("BPC", true));
    m_inProfileBG->setButton(config->readNumEntry("InputProfileMethod", 0));
    m_spaceProfileBG->setButton(config->readNumEntry("SpaceProfileMethod", 0));
    m_proofProfileBG->setButton(config->readNumEntry("ProofProfileMethod", 0));
    m_cInput->setValue(config->readNumEntry("ContrastAjustment", m_cInput->defaultValue()));

    for (int i = 0 ; i < 5 ; i++)
        m_curvesWidget->curves()->curvesChannelReset(i);

    m_curvesWidget->curves()->setCurveType(m_curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
    m_curvesWidget->reset();

    for (int j = 0 ; j < 17 ; j++)
    {
        QPoint disable(-1, -1);
        QPoint p = config->readPointEntry(QString("CurveAjustmentPoint%1").arg(j), &disable);

        if (m_originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()*255);
            p.setY(p.y()*255);
        }

        m_curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, j, p);
    }

    for (int i = 0 ; i < 5 ; i++)
        m_curvesWidget->curves()->curvesCalculateCurve(i);

    m_histogramWidget->reset();
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
}

void ICCProofTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("colormanagement Tool");
    config->writeEntry("Settings Tab", m_toolBoxWidgets->currentIndex());
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
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

    for (int j = 0; j < 17; j++)
    {
        QPoint p = m_curvesWidget->curves()->getCurvePoint(ImageHistogram::ValueChannel, j);

        if (m_originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x() / 255);
            p.setY(p.y() / 255);
        }

        config->writeEntry(QString("CurveAjustmentPoint%1").arg(j), p);
    }

    m_previewWidget->writeSettings();
    config->sync();
}

void ICCProofTool::processLCMSURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ICCProofTool::slotSpotColorChanged(const DColor &color)
{
    m_curvesWidget->setCurveGuide(color);
}

void ICCProofTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ICCProofTool::slotChannelChanged( int channel )
{
    switch (channel)
    {
    case LuminosityChannel:
        m_histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
        m_hGradient->setColors(QColor("black"), QColor("white"));
        break;

    case RedChannel:
        m_histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
        m_hGradient->setColors(QColor("black"), QColor("red"));
        break;

    case GreenChannel:
        m_histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
        m_hGradient->setColors(QColor("black"), QColor("green"));
        break;

    case BlueChannel:
        m_histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
        m_hGradient->setColors(QColor("black"), QColor("blue"));
        break;
    }

    m_histogramWidget->repaint(false);
}

void ICCProofTool::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ICCProofTool::slotResetSettings()
{
    m_cInput->blockSignals(true);
    m_renderingIntentsCB->blockSignals(true);

    m_cInput->slotReset();
    m_renderingIntentsCB->slotReset();

    for (int i = 0 ; i < 5 ; i++)
       m_curvesWidget->curves()->curvesChannelReset(i);

    m_curvesWidget->reset();
    m_cInput->blockSignals(false);
    m_renderingIntentsCB->blockSignals(false);
}

void ICCProofTool::slotEffect()
{
    kapp->setOverrideCursor(KCursor::waitCursor());
    m_gboxSettings->enableButton(EditorToolSettings::Ok, true);
    m_histogramWidget->stopHistogramComputation();

    IccTransform transform;

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    ImageIface *iface = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int  w                     = iface->previewWidth();
    int  h                     = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    DImg preview(w, h, sb, a, m_destinationPreviewData);

    QString tmpInPath    = QString();
    QString tmpProofPath = QString();
    QString tmpSpacePath = QString();

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
        QFileInfo info(tmpInPath);
        if (!info.exists() || !info.isReadable() || !info.isFile())
        {
            KMessageBox::information(kapp->activeWindow(),
                                     i18n("<p>The selected ICC input profile path seems to be invalid.<p>"
                                          "Please check it."));
            return;
        }
    }

    //-- Proof profile parameters ------------------

    if (useDefaultProofProfile())
    {
        tmpProofPath = m_proofPath;
    }
    else
    {
        tmpProofPath = m_proofProfilePath->url();
        QFileInfo info(tmpProofPath);
        if (!info.exists() || !info.isReadable() || !info.isFile())
        {
            KMessageBox::information(kapp->activeWindow(),
                                     i18n("<p>The selected ICC proof profile path seems to be invalid.<p>"
                                          "Please check it."));
            return;
        }
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
        QFileInfo info(tmpSpacePath);
        if (!info.exists() || !info.isReadable() || !info.isFile())
        {
            KMessageBox::information(kapp->activeWindow(),
                                     i18n("<p>Selected ICC workspace profile path seems to be invalid.<p>"
                                          "Please check it."));
            return;
        }
    }

    spaceCondition = tmpSpacePath.isEmpty();

    //-- Perform the color transformations ------------------

    transform.getTransformType(m_doSoftProofBox->isChecked());

    if (m_doSoftProofBox->isChecked())
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.setProfiles(tmpSpacePath, tmpProofPath, true);
        }
        else
        {
            transform.setProfiles(tmpInPath, tmpSpacePath, tmpProofPath);
        }
    }
    else
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.setProfiles(tmpSpacePath);
        }
        else
        {
            transform.setProfiles(tmpInPath, tmpSpacePath);
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
        KMessageBox::information(kapp->activeWindow(), error);
        m_gboxSettings->enableButton(EditorToolSettings::Ok, false);
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

        DImg preview2(w, h, sb, a, 0, false);
        m_curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);
        m_curvesWidget->curves()->curvesLutProcess(preview.bits(), preview2.bits(), w, h);

        //-- Adjust contrast ---------------------------------------------------------

        BCGModifier cmod;
        cmod.setContrast((double)(m_cInput->value()/100.0) + 1.00);
        cmod.applyBCG(preview2);

        iface->putPreviewImage(preview2.bits());
        m_previewWidget->updatePreview();

        //-- Update histogram --------------------------------------------------------

        memcpy(m_destinationPreviewData, preview2.bits(), preview2.numBytes());
        m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
        kapp->restoreOverrideCursor();
    }
}

void ICCProofTool::finalRendering()
{
    if (!m_doSoftProofBox->isChecked())
    {
        kapp->setOverrideCursor( KCursor::waitCursor() );

        ImageIface *iface = m_previewWidget->imageIface();
        uchar *data                = iface->getOriginalImage();
        int w                      = iface->originalWidth();
        int h                      = iface->originalHeight();
        bool a                     = iface->originalHasAlpha();
        bool sb                    = iface->originalSixteenBit();

        if (data)
        {
            IccTransform transform;

            DImg img(w, h, sb, a, data);

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
                QFileInfo info(tmpInPath);
                if (!info.exists() || !info.isReadable() || !info.isFile())
                {
                    KMessageBox::information(kapp->activeWindow(),
                                             i18n("<p>Selected ICC input profile path seems "
                                                  "to be invalid.<p>Please check it."));
                    return;
                }
            }

            //-- Proof profile parameters ------------------

            if (useDefaultProofProfile())
            {
                tmpProofPath = m_proofPath;
            }
            else
            {
                tmpProofPath = m_proofProfilePath->url();
                QFileInfo info(tmpProofPath);
                if (!info.exists() || !info.isReadable() || !info.isFile())
                {
                    KMessageBox::information(kapp->activeWindow(),
                                             i18n("<p>The selected ICC proof profile path seems "
                                                  "to be invalid.<p>Please check it."));
                    return;
                }
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
                QFileInfo info(tmpSpacePath);
                if (!info.exists() || !info.isReadable() || !info.isFile())
                {
                    KMessageBox::information(kapp->activeWindow(),
                                             i18n("<p>Selected ICC workspace profile path seems "
                                                  "to be invalid.<p>Please check it."));
                    return;
                }
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
                iface->setEmbeddedICCToOriginalImage(tmpSpacePath);
                DDebug() << k_funcinfo << QFile::encodeName(tmpSpacePath) << endl;
            }

            //-- Calculate and apply the curve on image after transformation -------------

            DImg img2(w, h, sb, a, 0, false);
            m_curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);
            m_curvesWidget->curves()->curvesLutProcess(img.bits(), img2.bits(), w, h);

            //-- Adjust contrast ---------------------------------------------------------

            BCGModifier cmod;
            cmod.setContrast((double)(m_cInput->value()/100.0) + 1.00);
            cmod.applyBCG(img2);

            iface->putOriginalImage("Color Management", img2.bits());
            delete [] data;
        }

        kapp->restoreOverrideCursor();
    }
}

void ICCProofTool::slotToggledWidgets( bool t)
{
    m_useInDefaultProfile->setEnabled(t);
    m_useProofDefaultProfile->setEnabled(t);
    m_useSpaceDefaultProfile->setEnabled(t);
}

void ICCProofTool::slotInICCInfo()
{
    if (useEmbeddedProfile())
    {
        getICCInfo(m_embeddedICC);
    }
    else if (useBuiltinProfile())
    {
        QString message = i18n("<p>You have selected the \"Default builtin sRGB profile\"</p>");
        message.append(i18n("<p>This profile is built on the fly, so there is no relevant information "
                            "about it.</p>"));
        KMessageBox::information(kapp->activeWindow(), message);
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

void ICCProofTool::slotProofICCInfo()
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

void ICCProofTool::slotSpaceICCInfo()
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

void ICCProofTool::getICCInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Sorry, there is no selected profile"),
                           i18n("Profile Error"));
        return;
    }

    ICCProfileInfoDlg infoDlg(kapp->activeWindow(), profile);
    infoDlg.exec();
}

void ICCProofTool::getICCInfo(const QByteArray& profile)
{
    if (profile.isNull())
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Sorry, it seems there is no embedded profile"),
                           i18n("Profile Error"));
        return;
    }

    ICCProfileInfoDlg infoDlg(kapp->activeWindow(), QString(), profile);
    infoDlg.exec();
}

void ICCProofTool::slotCMDisabledWarning()
{
    if (!m_cmEnabled)
    {
        QString message = i18n("<p>You have not enabled Color Management in the digiKam preferences.</p>");
        message.append(i18n("<p>\"Use of default profile\" options will be disabled now.</p>"));
        KMessageBox::information(kapp->activeWindow(), message);
        slotToggledWidgets(false);
    }
}

//-- General Tab ---------------------------

bool ICCProofTool::useBPC()
{
    return m_BPCBox->isChecked();
}

bool ICCProofTool::doProof()
{
    return m_doSoftProofBox->isChecked();
}

bool ICCProofTool::checkGamut()
{
    return m_checkGamutBox->isChecked();
}

bool ICCProofTool::embedProfile()
{
    return m_embeddProfileBox->isChecked();
}

//-- Input Tab ---------------------------

bool ICCProofTool::useEmbeddedProfile()
{
    return m_useEmbeddedProfile->isChecked();
}

bool ICCProofTool::useBuiltinProfile()
{
    return m_useSRGBDefaultProfile->isChecked();
}

bool ICCProofTool::useDefaultInProfile()
{
    return m_useInDefaultProfile->isChecked();
}

bool ICCProofTool::useSelectedInProfile()
{
    return m_useInSelectedProfile->isChecked();
}

//-- Workspace Tab ---------------------------

bool ICCProofTool::useDefaultSpaceProfile()
{
    return m_useSpaceDefaultProfile->isChecked();
}

//-- Proofing Tab ---------------------------

bool ICCProofTool::useDefaultProofProfile()
{
    return m_useProofDefaultProfile->isChecked();
}

//-- Load all settings from file --------------------------------------

void ICCProofTool::slotLoadSettings()
{
    KURL loadColorManagementFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                                           QString("*"), kapp->activeWindow(),
                                                           QString(i18n("Color Management Settings File to Load")));
    if (loadColorManagementFile.isEmpty())
        return;

    QFile file(loadColorManagementFile.path());

    if (file.open(IO_ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != "# Color Management Configuration File")
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Color Management settings text file.")
                               .arg(loadColorManagementFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);

        m_renderingIntentsCB->setCurrentItem(stream.readLine().toInt());
        m_doSoftProofBox->setChecked((bool) (stream.readLine().toUInt()));
        m_checkGamutBox->setChecked((bool) (stream.readLine().toUInt()));
        m_embeddProfileBox->setChecked((bool) (stream.readLine().toUInt()));
        m_BPCBox->setChecked((bool) (stream.readLine().toUInt()));
        m_inProfileBG->setButton(stream.readLine().toInt());
        m_spaceProfileBG->setButton(stream.readLine().toInt());
        m_proofProfileBG->setButton(stream.readLine().toInt());
        m_inProfilesPath->setURL(stream.readLine());
        m_proofProfilePath->setURL(stream.readLine());
        m_spaceProfilePath->setURL(stream.readLine());
        m_cInput->setValue(stream.readLine().toInt());

        for (int i = 0 ; i < 5 ; i++)
            m_curvesWidget->curves()->curvesChannelReset(i);

        m_curvesWidget->curves()->setCurveType(m_curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
        m_curvesWidget->reset();

        for (int j = 0; j < 17; j++)
        {
            QPoint disable(-1, -1);
            QPoint p;
            p.setX(stream.readLine().toInt());
            p.setY(stream.readLine().toInt());

            if (m_originalImage->sixteenBit() && p != disable)
            {
                p.setX(p.x() * 255);
                p.setY(p.y() * 255);
            }

            m_curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, j, p);
        }

        blockSignals(false);

        for (int i = 0 ; i < 5 ; i++)
           m_curvesWidget->curves()->curvesCalculateCurve(i);

        m_histogramWidget->reset();
        slotEffect();
    }
    else
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Color Management text file."));

    file.close();
}

//-- Save all settings to file ---------------------------------------

void ICCProofTool::slotSaveAsSettings()
{
    KURL saveColorManagementFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                                QString( "*" ), kapp->activeWindow(),
                                                QString(i18n("Color Management Settings File to Save")));
    if (saveColorManagementFile.isEmpty())
        return;

    QFile file(saveColorManagementFile.path());

    if (file.open(IO_WriteOnly))
    {
        QTextStream stream(&file);
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

        for (int j = 0; j < 17; j++)
        {
            QPoint p = m_curvesWidget->curves()->getCurvePoint(ImageHistogram::ValueChannel, j);
            if (m_originalImage->sixteenBit())
            {
                p.setX(p.x() / 255);
                p.setY(p.y() / 255);
            }
            stream << p.x() << "\n";
            stream << p.y() << "\n";
        }
    }
    else
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Color Management text file."));

    file.close();
}

} // NameSpace DigikamImagesPluginCore
