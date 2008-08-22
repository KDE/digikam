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

#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QRadioButton>
#include <QTextStream>
#include <QToolBox>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfile.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kurlrequester.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Digikam includes.

#include "ddebug.h"
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
#include "imageeffect_iccproof.moc"

using namespace KDcrawIface;

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

    m_previewWidget = new Digikam::ImageWidget("colormanagement Tool Dialog", mainWidget(),
                                               i18n("<p>Here you can see the image preview after "
                                                    "applying a color profile</p>"));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout *gridSettings = new QGridLayout( gboxSettings );

    QLabel *label1 = new QLabel(i18n("Channel: "), gboxSettings);
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"));

    // -------------------------------------------------------------

    QWidget *scaleBox = new QWidget(gboxSettings);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));

    QToolButton *linHistoButton = new QToolButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);

    QToolButton *logHistoButton = new QToolButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    m_scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    KVBox *histoBox   = new KVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram "
                                          "of the selected image channel. "
                                          "This one is updated after setting changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10,
                                                    histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // -------------------------------------------------------------

    m_toolBoxWidgets         = new QToolBox(gboxSettings);
    QWidget *generalOptions  = new QWidget(m_toolBoxWidgets);
    QWidget *inProfiles      = new QWidget(m_toolBoxWidgets);
    QWidget *spaceProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *proofProfiles   = new QWidget(m_toolBoxWidgets);
    QWidget *lightnessadjust = new QWidget(m_toolBoxWidgets);

    //---------- "General" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(GENERALPAGE, generalOptions,
                                 SmallIcon("system-run"), i18n("General Settings"));
    generalOptions->setWhatsThis( i18n("<p>Here you can set general parameters.</p>"));

    QGridLayout *zeroPageLayout = new QGridLayout(generalOptions);

    m_doSoftProofBox = new QCheckBox(generalOptions);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    m_doSoftProofBox->setWhatsThis( i18n("<p>Rendering emulation of the device described "
                                         "by the \"Proofing\" profile. Useful to preview the final "
                                         "result without rendering to a physical medium.</p>"));

    m_checkGamutBox = new QCheckBox(generalOptions);
    m_checkGamutBox->setText(i18n("Check gamut"));
    m_checkGamutBox->setWhatsThis( i18n("<p>You can use this option if you want to show "
                                        "the colors that are outside the printer's gamut<p>"));

    m_embeddProfileBox = new QCheckBox(generalOptions);
    m_embeddProfileBox->setChecked(true);
    m_embeddProfileBox->setText(i18n("Assign profile"));
    m_embeddProfileBox->setWhatsThis( i18n("<p>You can use this option to embed "
                                           "the selected workspace color profile into the image.</p>"));

    m_BPCBox = new QCheckBox(generalOptions);
    m_BPCBox->setText(i18n("Use BPC"));
    m_BPCBox->setWhatsThis( i18n("<p>The Black Point Compensation (BPC) feature does work in conjunction "
                                 "with Relative Colorimetric Intent. Perceptual intent should make no "
                                 "difference, since BPC is always on, and in Absolute Colorimetric "
                                 "Intent it is always turned off.</p>"
                                 "<p>BPC does compensate for a lack of ICC profiles in the dark tone rendering. "
                                 "With BPC the dark tones are optimally mapped (no clipping) from original media "
                                 "to the destination rendering media, e.g. the combination of paper and ink.</p>"));

    QLabel *intent       = new QLabel(i18n("Rendering Intent:"), generalOptions);
    m_renderingIntentsCB = new QComboBox(generalOptions);
    m_renderingIntentsCB->addItem("Perceptual");
    m_renderingIntentsCB->addItem("Absolute Colorimetric");
    m_renderingIntentsCB->addItem("Relative Colorimetric");
    m_renderingIntentsCB->addItem("Saturation");
    m_renderingIntentsCB->setWhatsThis( i18n("<ul><li>Perceptual intent causes the full gamut "
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

    KUrlLabel *lcmsLogoLabel = new KUrlLabel(generalOptions);
    lcmsLogoLabel->setText(QString());
    lcmsLogoLabel->setUrl("http://www.littlecms.com");
    lcmsLogoLabel->setPixmap( QPixmap( KStandardDirs::locate("data", "digikam/data/logo-lcms.png" ) ));
    lcmsLogoLabel->setToolTip(i18n("Visit Little CMS project website"));

    zeroPageLayout->addWidget(m_doSoftProofBox, 0, 0, 1, 1);
    zeroPageLayout->addWidget(m_checkGamutBox, 1, 0, 1, 1);
    zeroPageLayout->addWidget(m_embeddProfileBox, 2, 0, 1, 1);
    zeroPageLayout->addWidget(lcmsLogoLabel, 0, 1, 3, 1);
    zeroPageLayout->addWidget(m_BPCBox, 3, 0, 1, 1);
    zeroPageLayout->addWidget(intent, 4, 0, 1, 1);
    zeroPageLayout->addWidget(m_renderingIntentsCB, 4, 1, 1, 1);
    zeroPageLayout->setRowStretch(5, 10);
    zeroPageLayout->setMargin(spacingHint());
    zeroPageLayout->setSpacing(spacingHint());

    //---------- "Input" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(INPUTPAGE, inProfiles, SmallIcon("camera-photo"), i18n("Input Profile"));
    inProfiles->setWhatsThis( i18n("<p>Set here all parameters relevant of Input Color "
                                   "Profiles.</p>"));

    QGridLayout *firstPageLayout = new QGridLayout(inProfiles);

    QWidget *box1      = new QWidget(inProfiles);
    QVBoxLayout *hlay1 = new QVBoxLayout(box1);
    m_inProfileBG      = new QButtonGroup(inProfiles);

    m_useEmbeddedProfile = new QRadioButton(box1);
    m_useEmbeddedProfile->setText(i18n("Use embedded profile"));
    m_inProfileBG->addButton(m_useEmbeddedProfile, 0);

    m_useSRGBDefaultProfile = new QRadioButton(box1);
    m_useSRGBDefaultProfile->setText(i18n("Use builtin sRGB profile"));
    m_useSRGBDefaultProfile->setCheckable(true);
    m_inProfileBG->addButton(m_useSRGBDefaultProfile, 1);

    m_useInDefaultProfile = new QRadioButton(box1);
    m_useInDefaultProfile->setText(i18n("Use default profile"));
    m_inProfileBG->addButton(m_useInDefaultProfile, 2);

    m_useInSelectedProfile = new QRadioButton(box1);
    m_useInSelectedProfile->setText(i18n("Use selected profile"));
    m_inProfileBG->addButton(m_useInSelectedProfile, 3);

    hlay1->addWidget(m_useEmbeddedProfile);
    hlay1->addWidget(m_useSRGBDefaultProfile);
    hlay1->addWidget(m_useInDefaultProfile);
    hlay1->addWidget(m_useInSelectedProfile);
    hlay1->setMargin(0);
    hlay1->setSpacing(0);

    m_inProfilesPath = new KUrlRequester(inProfiles);
    m_inProfilesPath->setMode(KFile::File|KFile::ExistingOnly);
    m_inProfilesPath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));

    KFileDialog *inProfiles_dialog = m_inProfilesPath->fileDialog();
    m_iccInPreviewWidget = new Digikam::ICCPreviewWidget(inProfiles_dialog);
    inProfiles_dialog->setPreviewWidget(m_iccInPreviewWidget);

    QPushButton *inProfilesInfo = new QPushButton(i18n("Info..."), inProfiles);

    QGroupBox *pictureInfo = new QGroupBox(i18n("Camera information"), inProfiles);
    QGridLayout *infoGrid  = new QGridLayout(pictureInfo);

    QLabel *make1              = new QLabel(i18n("Make:"), pictureInfo);
    KSqueezedTextLabel *make2  = new KSqueezedTextLabel(0, pictureInfo);
    QLabel *model1             = new QLabel(i18n("Model:"), pictureInfo);
    KSqueezedTextLabel *model2 = new KSqueezedTextLabel(0, pictureInfo);
    make2->setText(iface.getPhotographInformations().make);
    model2->setText(iface.getPhotographInformations().model);

    infoGrid->addWidget(make1,  0, 0, 1, 1);
    infoGrid->addWidget(make2,  0, 1, 1, 1);
    infoGrid->addWidget(model1, 1, 0, 1, 1);
    infoGrid->addWidget(model2, 1, 1, 1, 1);
    infoGrid->setMargin(spacingHint());
    infoGrid->setSpacing(0);

    firstPageLayout->addWidget(box1, 0, 0, 2, 1);
    firstPageLayout->addWidget(inProfilesInfo, 0, 2, 1, 1);
    firstPageLayout->addWidget(m_inProfilesPath, 2, 0, 1, 3 );
    firstPageLayout->addWidget(pictureInfo, 3, 0, 1, 3 );
    firstPageLayout->setColumnStretch(1, 10);
    firstPageLayout->setRowStretch(4, 10);
    firstPageLayout->setMargin(spacingHint());
    firstPageLayout->setSpacing(spacingHint());

    //---------- "Workspace" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(WORKSPACEPAGE, spaceProfiles,
                                 SmallIcon("input-tablet"), i18n("Workspace Profile"));
    spaceProfiles->setWhatsThis( i18n("<p>Set here all parameters relevant to Color Workspace "
                                      "Profiles.</p>"));

    QGridLayout *secondPageLayout = new QGridLayout(spaceProfiles);

    QWidget *box2      = new QWidget(spaceProfiles);
    QVBoxLayout *hlay2 = new QVBoxLayout(box2);
    m_spaceProfileBG   = new QButtonGroup(box2);

    m_useSpaceDefaultProfile = new QRadioButton(box2);
    m_useSpaceDefaultProfile->setText(i18n("Use default workspace profile"));
    m_spaceProfileBG->addButton(m_useSpaceDefaultProfile, 0);

    m_useSpaceSelectedProfile = new QRadioButton(box2);
    m_useSpaceSelectedProfile->setText(i18n("Use selected profile"));
    m_spaceProfileBG->addButton(m_useSpaceSelectedProfile, 1);

    hlay2->addWidget(m_useSpaceDefaultProfile);
    hlay2->addWidget(m_useSpaceSelectedProfile);
    hlay2->setMargin(0);
    hlay2->setSpacing(0);

    m_spaceProfilePath = new KUrlRequester(spaceProfiles);
    m_spaceProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    m_spaceProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));

    KFileDialog *spaceProfiles_dialog = m_spaceProfilePath->fileDialog();
    m_iccSpacePreviewWidget = new Digikam::ICCPreviewWidget(spaceProfiles_dialog);
    spaceProfiles_dialog->setPreviewWidget(m_iccSpacePreviewWidget);

    QPushButton *spaceProfilesInfo = new QPushButton(i18n("Info..."), box2);

    secondPageLayout->addWidget(box2, 0, 0, 2, 1);
    secondPageLayout->addWidget(spaceProfilesInfo, 0, 2, 1, 1);
    secondPageLayout->addWidget(m_spaceProfilePath, 2, 0, 1, 3 );
    secondPageLayout->setColumnStretch(1, 10);
    secondPageLayout->setRowStretch(3, 10);
    secondPageLayout->setMargin(spacingHint());
    secondPageLayout->setSpacing(spacingHint());

    //---------- "Proofing" Page Setup ---------------------------------

    m_toolBoxWidgets->insertItem(PROOFINGPAGE, proofProfiles,
                                 SmallIcon("printer"), i18n("Proofing Profile"));
    proofProfiles->setWhatsThis( i18n("<p>Set here all parameters relevant to Proofing Color "
                                      "Profiles.</p>"));

    QGridLayout *thirdPageLayout = new QGridLayout(proofProfiles);

    QWidget *box3      = new QWidget(proofProfiles);
    QVBoxLayout *hlay3 = new QVBoxLayout(box3);
    m_proofProfileBG   = new QButtonGroup(box3);

    m_useProofDefaultProfile = new QRadioButton(box3);
    m_useProofDefaultProfile->setText(i18n("Use default proof profile"));
    m_proofProfileBG->addButton(m_useProofDefaultProfile, 0);

    m_useProofSelectedProfile = new QRadioButton(box3);
    m_useProofSelectedProfile->setText(i18n("Use selected profile"));
    m_proofProfileBG->addButton(m_useProofSelectedProfile, 1);

    hlay3->addWidget(m_useProofDefaultProfile);
    hlay3->addWidget(m_useProofSelectedProfile);
    hlay3->setMargin(0);
    hlay3->setSpacing(0);

    m_proofProfilePath = new KUrlRequester(proofProfiles);
    m_proofProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    m_proofProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));

    KFileDialog *proofProfiles_dialog = m_proofProfilePath->fileDialog();
    m_iccProofPreviewWidget = new Digikam::ICCPreviewWidget(proofProfiles_dialog);
    proofProfiles_dialog->setPreviewWidget(m_iccProofPreviewWidget);

    QPushButton *proofProfilesInfo = new QPushButton(i18n("Info..."), box3);

    thirdPageLayout->addWidget(box3, 0, 0, 2, 1);
    thirdPageLayout->addWidget(proofProfilesInfo, 0, 2, 1, 1);
    thirdPageLayout->addWidget(m_proofProfilePath, 2, 0, 1, 3 );
    thirdPageLayout->setColumnStretch(1, 10);
    thirdPageLayout->setRowStretch(3, 10);
    thirdPageLayout->setMargin(spacingHint());
    thirdPageLayout->setSpacing(spacingHint());

    //---------- "Lightness" Page Setup ----------------------------------

    m_toolBoxWidgets->insertItem(LIGHTNESSPAGE, lightnessadjust,
                                 SmallIcon("format-stroke-color"), i18n("Lightness Adjustments"));
    lightnessadjust->setWhatsThis( i18n("<p>Set here all lightness adjustments to the target image.</p>"));

    QGridLayout *fourPageLayout = new QGridLayout( lightnessadjust );

    Digikam::ColorGradientWidget* vGradient = new Digikam::ColorGradientWidget(
                                                  Digikam::ColorGradientWidget::Vertical,
                                                  10, lightnessadjust );
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(lightnessadjust);
    spacev->setFixedWidth(1);

    m_curvesWidget = new Digikam::CurvesWidget(256, 192, m_originalImage->bits(), m_originalImage->width(),
                                               m_originalImage->height(), m_originalImage->sixteenBit(),
                                               lightnessadjust);
    m_curvesWidget->setWhatsThis( i18n("<p>This is the curve adjustment of the image luminosity"));

    QLabel *spaceh = new QLabel(lightnessadjust);
    spaceh->setFixedHeight(1);

    Digikam::ColorGradientWidget *hGradient = new Digikam::ColorGradientWidget(
                                                  Digikam::ColorGradientWidget::Horizontal,
                                                  10, lightnessadjust );
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    m_cInput = new KIntNumInput(lightnessadjust);
    m_cInput->setLabel(i18n("Contrast:"), Qt::AlignLeft | Qt::AlignVCenter);
    m_cInput->setRange(-100, 100, 1);
    m_cInput->setSliderEnabled(true);
    m_cInput->setValue(0);
    m_cInput->setWhatsThis( i18n("<p>Set here the contrast adjustment of the image."));

    fourPageLayout->addWidget(vGradient, 0, 0, 1, 1);
    fourPageLayout->addWidget(spacev, 0, 1, 1, 1);
    fourPageLayout->addWidget(m_curvesWidget, 0, 2, 1, 1);
    fourPageLayout->addWidget(spaceh, 1, 2, 1, 1);
    fourPageLayout->addWidget(hGradient, 2, 2, 1, 1);
    fourPageLayout->addWidget(m_cInput, 4, 0, 1, 3 );
    fourPageLayout->setRowMinimumHeight(3, spacingHint());
    fourPageLayout->setRowStretch(5, 10);
    fourPageLayout->setMargin(spacingHint());
    fourPageLayout->setSpacing(0);

    // -------------------------------------------------------------

    gridSettings->addLayout(l1, 0, 0, 1, 3 );
    gridSettings->addWidget(histoBox, 1, 0, 2, 3 );
    gridSettings->addWidget(m_toolBoxWidgets, 3, 0, 1, 3 );
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);
    enableButtonOk(false);

    // -------------------------------------------------------------

    connect(lcmsLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processLCMSUrl(const QString&)));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(m_cInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_renderingIntentsCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    //-- Check box options connections -------------------------------------------

    connect(m_doSoftProofBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(m_checkGamutBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(m_BPCBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    //-- Button Group ICC profile options connections ----------------------------

    connect(m_inProfileBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEffect()));

    connect(m_spaceProfileBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEffect()));

    connect(m_proofProfileBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEffect()));

    //-- url requester ICC profile connections -----------------------------------

    connect(m_inProfilesPath, SIGNAL(urlSelected(const Kurl&)),
            this, SLOT(slotEffect()));

    connect(m_spaceProfilePath, SIGNAL(urlSelected(const Kurl&)),
            this, SLOT(slotEffect()));

    connect(m_proofProfilePath, SIGNAL(urlSelected(const Kurl&)),
            this, SLOT(slotEffect()));

    //-- Image preview widget connections ----------------------------

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    //-- ICC profile preview connections -----------------------------

    connect(inProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotInICCInfo()));

    connect(spaceProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotSpaceICCInfo()));

    connect(proofProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotProofICCInfo()));
}

ImageEffect_ICCProof::~ImageEffect_ICCProof()
{
    m_histogramWidget->stopHistogramComputation();

    delete [] m_destinationPreviewData;
}

void ImageEffect_ICCProof::readUserSettings()
{
    QString defaultICCPath = KGlobalSettings::documentPath();
    KSharedConfig::Ptr config = KGlobal::config();

    // General settings of digiKam Color Management
    KConfigGroup group = config->group("Color Management");

    if (!group.readEntry("EnableCM", false))
    {
        m_cmEnabled = false;
        slotToggledWidgets(false);
    }
    else
    {
        m_inPath      = group.readEntry("InProfileFile");
        m_spacePath   = group.readEntry("WorkProfileFile");
        m_proofPath   = group.readEntry("ProofProfileFile");

        if (QFile::exists(group.readPathEntry("DefaultPath", QString())))
        {
            defaultICCPath = group.readPathEntry("DefaultPath", QString());
        }
        else
        {
            QString message = i18n("The ICC profiles path seems to be invalid. You'll not be able to use \"Default profile\"\
                                    options.<p>Please fix this in digiKam ICC setup.");
            slotToggledWidgets( false );
            KMessageBox::information(this, message);
        }
    }

    // Plugin settings.
    group = config->group("colormanagement Tool Dialog");
    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_toolBoxWidgets->setCurrentIndex(group.readEntry("Settings Tab", (int)GENERALPAGE));
    m_inProfilesPath->setUrl(group.readPathEntry("InputProfilePath", defaultICCPath));
    m_proofProfilePath->setUrl(group.readPathEntry("ProofProfilePath", defaultICCPath));
    m_spaceProfilePath->setUrl(group.readPathEntry("SpaceProfilePath", defaultICCPath));
    m_renderingIntentsCB->setCurrentIndex(group.readEntry("RenderingIntent", 0));
    m_doSoftProofBox->setChecked(group.readEntry("DoSoftProof", false));
    m_checkGamutBox->setChecked(group.readEntry("CheckGamut", false));
    m_embeddProfileBox->setChecked(group.readEntry("EmbeddProfile", true));
    m_BPCBox->setChecked(group.readEntry("BPC", true));
    m_inProfileBG->button(group.readEntry("InputProfileMethod", 0))->setChecked(true);
    m_spaceProfileBG->button(group.readEntry("SpaceProfileMethod", 0))->setChecked(true);
    m_proofProfileBG->button(group.readEntry("ProofProfileMethod", 0))->setChecked(true);
    m_cInput->setValue(group.readEntry("ContrastAjustment", 0));

    for (int i = 0 ; i < 5 ; i++)
        m_curvesWidget->curves()->curvesChannelReset(i);

    m_curvesWidget->curves()->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
    m_curvesWidget->reset();

    for (int j = 0 ; j < 17 ; j++)
    {
        QPoint disable(-1, -1);
        QPoint p = group.readEntry(QString("CurveAjustmentPoint%1").arg(j), disable);

        if (m_originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()*255);
            p.setY(p.y()*255);
        }

        m_curvesWidget->curves()->setCurvePoint(Digikam::ImageHistogram::ValueChannel, j, p);
    }

    for (int i = 0 ; i < 5 ; i++)
        m_curvesWidget->curves()->curvesCalculateCurve(i);

    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ImageEffect_ICCProof::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("colormanagement Tool Dialog");
    group.writeEntry("Settings Tab", m_toolBoxWidgets->currentIndex());
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());
    group.writeEntry("InputProfilePath", m_inProfilesPath->url());
    group.writeEntry("ProofProfilePath", m_proofProfilePath->url());
    group.writeEntry("SpaceProfilePath", m_spaceProfilePath->url());
    group.writeEntry("RenderingIntent", m_renderingIntentsCB->currentIndex());
    group.writeEntry("DoSoftProof", m_doSoftProofBox->isChecked());
    group.writeEntry("CheckGamut", m_checkGamutBox->isChecked());
    group.writeEntry("EmbeddProfile", m_embeddProfileBox->isChecked());
    group.writeEntry("BPC", m_BPCBox->isChecked());
    group.writeEntry("InputProfileMethod", m_inProfileBG->checkedId());
    group.writeEntry("SpaceProfileMethod", m_spaceProfileBG->checkedId());
    group.writeEntry("ProofProfileMethod", m_proofProfileBG->checkedId());
    group.writeEntry("ContrastAjustment", m_cInput->value());

    for (int j = 0 ; j < 17 ; j++)
    {
        QPoint p = m_curvesWidget->curves()->getCurvePoint(Digikam::ImageHistogram::ValueChannel, j);

        if (m_originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()/255);
            p.setY(p.y()/255);
        }

        group.writeEntry(QString("CurveAjustmentPoint%1").arg(j), p);
    }

    group.sync();
}

void ImageEffect_ICCProof::processLCMSUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
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

    m_histogramWidget->repaint();
}

void ImageEffect_ICCProof::slotScaleChanged( int scale )
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_ICCProof::resetValues()
{
    m_cInput->blockSignals(true);
    m_cInput->setValue(0);

    for (int i = 0 ; i < 5 ; i++)
       m_curvesWidget->curves()->curvesChannelReset(i);

    m_curvesWidget->reset();
    m_cInput->blockSignals(false);
}

void ImageEffect_ICCProof::slotEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    enableButtonOk(true);
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
        tmpInPath = m_inProfilesPath->url().path();
        QFileInfo info(tmpInPath);
        if (!info.exists() || !info.isReadable() || !info.isFile() )
        {
            KMessageBox::information(this, i18n("<p>The selected ICC input profile path seems to be invalid.<p>"
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
        tmpProofPath = m_proofProfilePath->url().path();
        QFileInfo info(tmpProofPath);
        if (!info.exists() || !info.isReadable() || !info.isFile() )
        {
            KMessageBox::information(this, i18n("<p>The selected ICC proof profile path seems to be invalid.<p>"
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
        tmpSpacePath = m_spaceProfilePath->url().path();
        QFileInfo info(tmpSpacePath);
        if (!info.exists() || !info.isReadable() || !info.isFile() )
        {
            KMessageBox::information(this, i18n("<p>The selected ICC workspace profile path seems to be invalid.<p>"
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
        enableButtonOk(false);
    }
    else
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.apply(preview, m_embeddedICC, m_renderingIntentsCB->currentIndex(), useBPC(),
                            m_checkGamutBox->isChecked(), useBuiltinProfile());
        }
        else
        {
            QByteArray fakeProfile = QByteArray();
            transform.apply(preview, fakeProfile, m_renderingIntentsCB->currentIndex(), useBPC(),
                            m_checkGamutBox->isChecked(), useBuiltinProfile());
        }

        //-- Calculate and apply the curve on image after transformation -------------

        Digikam::DImg preview2(w, h, sb, a, 0, false);
        m_curvesWidget->curves()->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
        m_curvesWidget->curves()->curvesLutProcess(preview.bits(), preview2.bits(), w, h);

        //-- Adjust contrast ---------------------------------------------------------

        Digikam::BCGModifier cmod;
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

void ImageEffect_ICCProof::finalRendering()
{
    if (!m_doSoftProofBox->isChecked())
    {
        kapp->setOverrideCursor( Qt::WaitCursor );

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
                tmpInPath = m_inProfilesPath->url().path();
                QFileInfo info(tmpInPath);
                if (!info.exists() || !info.isReadable() || !info.isFile() )
                {
                    KMessageBox::information(this, i18n("<p>The selected ICC input profile path seems "
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
                tmpProofPath = m_proofProfilePath->url().path();
                QFileInfo info(tmpProofPath);
                if (!info.exists() || !info.isReadable() || !info.isFile() )
                {
                    KMessageBox::information(this, i18n("<p>The selected ICC proof profile path seems "
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
                tmpSpacePath = m_spaceProfilePath->url().path();
                QFileInfo info(tmpSpacePath);
                if (!info.exists() || !info.isReadable() || !info.isFile() )
                {
                    KMessageBox::information(this, i18n("<p>The selected ICC workspace profile path seems "
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
                transform.apply(img, m_embeddedICC, m_renderingIntentsCB->currentIndex(), useBPC(),
                                m_checkGamutBox->isChecked(), useBuiltinProfile());
            }
            else
            {
                QByteArray fakeProfile = QByteArray();
                transform.apply(img, fakeProfile, m_renderingIntentsCB->currentIndex(), useBPC(),
                                m_checkGamutBox->isChecked(), useBuiltinProfile());
            }

            //-- Embed the workspace profile if necessary --------------------------------

            if (m_embeddProfileBox->isChecked())
            {
                iface->setEmbeddedICCToOriginalImage( tmpSpacePath );
                DDebug() << QFile::encodeName(tmpSpacePath) << endl;
            }

            //-- Calculate and apply the curve on image after transformation -------------

            Digikam::DImg img2(w, h, sb, a, 0, false);
            m_curvesWidget->curves()->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);
            m_curvesWidget->curves()->curvesLutProcess(img.bits(), img2.bits(), w, h);

            //-- Adjust contrast ---------------------------------------------------------

            Digikam::BCGModifier cmod;
            cmod.setContrast((double)(m_cInput->value()/100.0) + 1.00);
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
        getICCInfo(m_inProfilesPath->url().path());
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
        getICCInfo(m_proofProfilePath->url().path());
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
        getICCInfo(m_spaceProfilePath->url().path());
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

    Digikam::ICCProfileInfoDlg infoDlg(this, QString(), profile);
    infoDlg.exec();
}

void ImageEffect_ICCProof::slotCMDisabledWarning()
{
    if (!m_cmEnabled)
    {
        QString message = i18n("<p>You have not enabled Color Management in the digiKam preferences.</p>");
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
    KUrl loadColorManagementFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                                QString( "*" ), this,
                                                QString( i18n("Color Management Settings File to Load")) );
    if( loadColorManagementFile.isEmpty() )
       return;

    QFile file(loadColorManagementFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );

        if ( stream.readLine() != "# Color Management Configuration File" )
        {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a Color Management settings text file.",
                             loadColorManagementFile.fileName()));
           file.close();
           return;
        }

        blockSignals(true);

        m_renderingIntentsCB->setCurrentIndex( stream.readLine().toInt() );
        m_doSoftProofBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_checkGamutBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_embeddProfileBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_BPCBox->setChecked( (bool)(stream.readLine().toUInt()) );
        m_inProfileBG->button( stream.readLine().toInt() )->setChecked(true);
        m_spaceProfileBG->button( stream.readLine().toInt() )->setChecked(true);
        m_proofProfileBG->button( stream.readLine().toInt() )->setChecked(true);
        m_inProfilesPath->setUrl( stream.readLine() );
        m_proofProfilePath->setUrl( stream.readLine() );
        m_spaceProfilePath->setUrl( stream.readLine() );
        m_cInput->setValue( stream.readLine().toInt() );

        for (int i = 0 ; i < 5 ; i++)
            m_curvesWidget->curves()->curvesChannelReset(i);

        m_curvesWidget->curves()->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
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

            m_curvesWidget->curves()->setCurvePoint(Digikam::ImageHistogram::ValueChannel, j, p);
        }

        blockSignals(false);

        for (int i = 0 ; i < 5 ; i++)
           m_curvesWidget->curves()->curvesCalculateCurve(i);

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
    KUrl saveColorManagementFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                                QString( "*" ), this,
                                                QString( i18n("Color Management Settings File to Save")) );
    if( saveColorManagementFile.isEmpty() )
       return;

    QFile file(saveColorManagementFile.path());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Color Management Configuration File\n";
        stream << m_renderingIntentsCB->currentIndex() << "\n";
        stream << m_doSoftProofBox->isChecked() << "\n";
        stream << m_checkGamutBox->isChecked() << "\n";
        stream << m_embeddProfileBox->isChecked() << "\n";
        stream << m_BPCBox->isChecked() << "\n";
        stream << m_inProfileBG->checkedId() << "\n";
        stream << m_spaceProfileBG->checkedId() << "\n";
        stream << m_proofProfileBG->checkedId() << "\n";
        stream << m_inProfilesPath->url().path() << "\n";
        stream << m_proofProfilePath->url().path() << "\n";
        stream << m_spaceProfilePath->url().path() << "\n";
        stream << m_cInput->value() << "\n";

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint p = m_curvesWidget->curves()->getCurvePoint(Digikam::ImageHistogram::ValueChannel, j);
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
