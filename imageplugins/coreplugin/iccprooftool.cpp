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
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iccprooftool.h"
#include "iccprooftool.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
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

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfile.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kurlrequester.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "iccpreviewwidget.h"
#include "iccprofileinfodlg.h"
#include "icctransform.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class ICCProofToolPriv
{
public:

    ICCProofToolPriv()
    {
        cmEnabled                 = false;
        hasICC                    = false;
        destinationPreviewData    = 0;
        doSoftProofBox            = 0;
        checkGamutBox             = 0;
        embeddProfileBox          = 0;
        BPCBox                    = 0;
        useEmbeddedProfile        = 0;
        useInDefaultProfile       = 0;
        useInSelectedProfile      = 0;
        useProofDefaultProfile    = 0;
        useProofSelectedProfile   = 0;
        useSpaceDefaultProfile    = 0;
        useSpaceSelectedProfile   = 0;
        useSRGBDefaultProfile     = 0;
        optionsBG                 = 0;
        inProfileBG               = 0;
        spaceProfileBG            = 0;
        proofProfileBG            = 0;
        renderingIntentBG         = 0;
        profilesBG                = 0;
        toolBoxWidgets            = 0;
        inProfilesPath            = 0;
        spaceProfilePath          = 0;
        proofProfilePath          = 0;
        cInput                    = 0;
        renderingIntentsCB        = 0;
        originalImage             = 0;
        curvesWidget              = 0;
        previewWidget             = 0;
        iccInPreviewWidget        = 0;
        iccSpacePreviewWidget     = 0;
        iccProofPreviewWidget     = 0;
        gboxSettings              = 0;
    }

    bool                cmEnabled;
    bool                hasICC;

    uchar*              destinationPreviewData;

    QCheckBox*          doSoftProofBox;
    QCheckBox*          checkGamutBox;
    QCheckBox*          embeddProfileBox;
    QCheckBox*          BPCBox;

    QRadioButton*       useEmbeddedProfile;
    QRadioButton*       useInDefaultProfile;
    QRadioButton*       useInSelectedProfile;
    QRadioButton*       useProofDefaultProfile;
    QRadioButton*       useProofSelectedProfile;
    QRadioButton*       useSpaceDefaultProfile;
    QRadioButton*       useSpaceSelectedProfile;
    QRadioButton*       useSRGBDefaultProfile;

    QString             inPath;
    QString             spacePath;
    QString             proofPath;

    QButtonGroup*       optionsBG;
    QButtonGroup*       inProfileBG;
    QButtonGroup*       spaceProfileBG;
    QButtonGroup*       proofProfileBG;
    QButtonGroup*       renderingIntentBG;
    QButtonGroup*       profilesBG;

    QByteArray          embeddedICC;

    QToolBox*           toolBoxWidgets;

    KUrlRequester*      inProfilesPath;
    KUrlRequester*      spaceProfilePath;
    KUrlRequester*      proofProfilePath;

    ICCPreviewWidget*   iccInPreviewWidget;
    ICCPreviewWidget*   iccSpacePreviewWidget;
    ICCPreviewWidget*   iccProofPreviewWidget;

    RIntNumInput*       cInput;
    RComboBox*          renderingIntentsCB;

    DImg*               originalImage;
    CurvesWidget*       curvesWidget;
    ImageWidget*        previewWidget;
    EditorToolSettings* gboxSettings;
};

ICCProofTool::ICCProofTool(QObject* parent)
            : EditorTool(parent),
              d(new ICCProofToolPriv)
{
    setObjectName("colormanagement");
    setToolName(i18n("Color Management"));
    setToolIcon(SmallIcon("colormanagement"));

    d->destinationPreviewData = 0;
    d->cmEnabled              = true;
    d->hasICC                 = false;

    ImageIface iface(0, 0);
    d->originalImage = iface.getOriginalImg();
    d->embeddedICC   = iface.getEmbeddedICCFromOriginalImage();

    d->previewWidget = new ImageWidget("colormanagement Tool",0,
                                       i18n("<p>A preview of the image after "
                                            "applying a color profile is shown here.</p>"));
    setToolView(d->previewWidget);

    // -------------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Load|
                                             EditorToolSettings::SaveAs|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel,
                                             EditorToolSettings::Histogram);


    QGridLayout *gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    d->toolBoxWidgets        = new QToolBox(d->gboxSettings->plainPage());
    QWidget *generalOptions  = new QWidget(d->toolBoxWidgets);
    QWidget *inProfiles      = new QWidget(d->toolBoxWidgets);
    QWidget *spaceProfiles   = new QWidget(d->toolBoxWidgets);
    QWidget *proofProfiles   = new QWidget(d->toolBoxWidgets);
    QWidget *lightnessadjust = new QWidget(d->toolBoxWidgets);

    //---------- "General" Page Setup ----------------------------------

    d->toolBoxWidgets->insertItem(GENERALPAGE, generalOptions,
                                  SmallIcon("system-run"), i18n("General Settings"));
    generalOptions->setWhatsThis( i18n("<p>Here you can set general parameters.</p>"));

    QGridLayout *zeroPageLayout = new QGridLayout(generalOptions);

    d->doSoftProofBox = new QCheckBox(generalOptions);
    d->doSoftProofBox->setText(i18n("Soft-proofing"));
    d->doSoftProofBox->setWhatsThis(i18n("Rendering emulation of the device described "
                                         "by the \"Proofing\" profile. Useful to preview the final "
                                         "result without rendering to a physical medium."));

    d->checkGamutBox = new QCheckBox(generalOptions);
    d->checkGamutBox->setText(i18n("Check gamut"));
    d->checkGamutBox->setWhatsThis(i18n("You can use this option if you want to show "
                                        "the colors that are outside the printer's gamut"));

    d->embeddProfileBox = new QCheckBox(generalOptions);
    d->embeddProfileBox->setChecked(true);
    d->embeddProfileBox->setText(i18n("Assign profile"));
    d->embeddProfileBox->setWhatsThis(i18n("You can use this option to embed "
                                           "the selected workspace color profile into the image."));

    d->BPCBox = new QCheckBox(generalOptions);
    d->BPCBox->setText(i18n("Use BPC"));
    d->BPCBox->setWhatsThis(i18n("<p>The Black Point Compensation (BPC) feature works in conjunction "
                                 "with Relative Colorimetric Intent. With Perceptual Intent, it should make no "
                                 "difference, since BPC is always on, and with Absolute Colorimetric, "
                                 "Intent it is always turned off.</p>"
                                 "<p>BPC compensates for a lack of ICC profiles for rendering dark tones. "
                                 "With BPC the dark tones are optimally mapped (no clipping) from original the medium "
                                 "to the destination rendering medium, e.g. the combination of paper and ink.</p>"));

    QLabel *intent        = new QLabel(i18n("Rendering Intent:"), generalOptions);
    d->renderingIntentsCB = new RComboBox(generalOptions);
    d->renderingIntentsCB->addItem("Perceptual");
    d->renderingIntentsCB->addItem("Absolute Colorimetric");
    d->renderingIntentsCB->addItem("Relative Colorimetric");
    d->renderingIntentsCB->addItem("Saturation");
    d->renderingIntentsCB->setDefaultIndex(0);
    d->renderingIntentsCB->setWhatsThis( i18n("<ul><li>Perceptual intent causes the full gamut "
                "of the image to be compressed or expanded to fill the gamut of the destination media, "
                "so that gray balance is preserved but colorimetric accuracy may not be preserved.<br/>"
                "In other words, if certain colors in an image fall outside of the range of colors that "
                "the output device can render, the image intent will cause all the colors in the image "
                "to be adjusted so that every color in the image falls within the range that can be "
                "rendered and so that the relationship between colors is preserved as much as possible.<br/>"
                "This intent is most suitable for display of photographs and images, and is the default "
                "intent.</li>"
                "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the "
                "output device can render to be adjusted to the closest color that can be rendered, while all "
                "other colors are left unchanged.<br/>"
                "This intent preserves the white point and is most suitable for spot colors (Pantone, "
                "TruMatch, logo colors, ...).</li>"
                "<li>Relative Colorimetric intent is defined such that any colors that fall outside the "
                "range that the output device can render are adjusted to the closest color that can be "
                "rendered, while all other colors are left unchanged. Proof intent does not preserve "
                "the white point.</li>"
                "<li>Saturation intent preserves the saturation of colors in the image at the possible "
                "expense of hue and lightness.<br/>"
                "Implementation of this intent remains somewhat problematic, and the ICC is still working "
                "on methods to achieve the desired effects.<br/>"
                "This intent is most suitable for business graphics such as charts, where it is more "
                "important that the colors be vivid and contrast well with each other rather than a "
                "specific color.</li></ul>"));

    KUrlLabel *lcmsLogoLabel = new KUrlLabel(generalOptions);
    lcmsLogoLabel->setAlignment(Qt::AlignRight);
    lcmsLogoLabel->setText(QString());
    lcmsLogoLabel->setUrl("http://www.littlecms.com");
    lcmsLogoLabel->setPixmap( QPixmap( KStandardDirs::locate("data", "digikam/data/logo-lcms.png" ) ));
    lcmsLogoLabel->setToolTip(i18n("Visit Little CMS project website"));

    zeroPageLayout->addWidget(d->doSoftProofBox,     0, 0, 1, 1);
    zeroPageLayout->addWidget(lcmsLogoLabel,         0, 1, 1, 1);
    zeroPageLayout->addWidget(d->checkGamutBox,      1, 0, 1, 1);
    zeroPageLayout->addWidget(d->embeddProfileBox,   2, 0, 1, 1);
    zeroPageLayout->addWidget(d->BPCBox,             3, 0, 1, 1);
    zeroPageLayout->addWidget(intent,                4, 0, 1, 1);
    zeroPageLayout->addWidget(d->renderingIntentsCB, 4, 1, 1, 1);
    zeroPageLayout->setRowStretch(5, 10);
    zeroPageLayout->setMargin(d->gboxSettings->spacingHint());
    zeroPageLayout->setSpacing(d->gboxSettings->spacingHint());

    //---------- "Input" Page Setup ----------------------------------

    d->toolBoxWidgets->insertItem(INPUTPAGE, inProfiles, SmallIcon("camera-photo"), i18n("Input Profile"));
    inProfiles->setWhatsThis( i18n("<p>Set here all parameters relevant to Input Color "
                                   "Profiles.</p>"));

    QGridLayout *firstPageLayout = new QGridLayout(inProfiles);

    QWidget *box1      = new QWidget(inProfiles);
    QVBoxLayout *hlay1 = new QVBoxLayout(box1);
    d->inProfileBG     = new QButtonGroup(inProfiles);

    d->useEmbeddedProfile = new QRadioButton(box1);
    d->useEmbeddedProfile->setText(i18n("Use embedded profile"));
    d->inProfileBG->addButton(d->useEmbeddedProfile, 0);

    d->useSRGBDefaultProfile = new QRadioButton(box1);
    d->useSRGBDefaultProfile->setText(i18n("Use built-in sRGB profile"));
    d->useSRGBDefaultProfile->setCheckable(true);
    d->inProfileBG->addButton(d->useSRGBDefaultProfile, 1);

    d->useInDefaultProfile = new QRadioButton(box1);
    d->useInDefaultProfile->setText(i18n("Use default profile"));
    d->inProfileBG->addButton(d->useInDefaultProfile, 2);

    d->useInSelectedProfile = new QRadioButton(box1);
    d->useInSelectedProfile->setText(i18n("Use selected profile"));
    d->inProfileBG->addButton(d->useInSelectedProfile, 3);

    hlay1->addWidget(d->useEmbeddedProfile);
    hlay1->addWidget(d->useSRGBDefaultProfile);
    hlay1->addWidget(d->useInDefaultProfile);
    hlay1->addWidget(d->useInSelectedProfile);
    hlay1->setMargin(0);
    hlay1->setSpacing(0);

    d->inProfilesPath = new KUrlRequester(inProfiles);
    d->inProfilesPath->setMode(KFile::File|KFile::ExistingOnly);
    d->inProfilesPath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));

    KFileDialog *inProfiles_dialog = d->inProfilesPath->fileDialog();
    d->iccInPreviewWidget = new ICCPreviewWidget(inProfiles_dialog);
    inProfiles_dialog->setPreviewWidget(d->iccInPreviewWidget);

    QPushButton *inProfilesInfo = new QPushButton(i18n("Info..."), inProfiles);

    QGroupBox *pictureInfo = new QGroupBox(i18n("Camera information"), inProfiles);
    QGridLayout *infoGrid  = new QGridLayout(pictureInfo);

    QLabel *make1              = new QLabel(i18nc("camera make", "Make:"), pictureInfo);
    KSqueezedTextLabel *make2  = new KSqueezedTextLabel(0, pictureInfo);
    QLabel *model1             = new QLabel(i18nc("camera model", "Model:"), pictureInfo);
    KSqueezedTextLabel *model2 = new KSqueezedTextLabel(0, pictureInfo);
    make2->setText(iface.getPhotographInformation().make);
    model2->setText(iface.getPhotographInformation().model);

    infoGrid->addWidget(make1,  0, 0, 1, 1);
    infoGrid->addWidget(make2,  0, 1, 1, 1);
    infoGrid->addWidget(model1, 1, 0, 1, 1);
    infoGrid->addWidget(model2, 1, 1, 1, 1);
    infoGrid->setMargin(d->gboxSettings->spacingHint());
    infoGrid->setSpacing(0);

    firstPageLayout->addWidget(box1,              0, 0, 2, 1);
    firstPageLayout->addWidget(inProfilesInfo,    0, 2, 1, 1);
    firstPageLayout->addWidget(d->inProfilesPath, 2, 0, 1, 3);
    firstPageLayout->addWidget(pictureInfo,       3, 0, 1, 3);
    firstPageLayout->setColumnStretch(1, 10);
    firstPageLayout->setRowStretch(4, 10);
    firstPageLayout->setMargin(d->gboxSettings->spacingHint());
    firstPageLayout->setSpacing(d->gboxSettings->spacingHint());

    //---------- "Workspace" Page Setup ---------------------------------

    d->toolBoxWidgets->insertItem(WORKSPACEPAGE, spaceProfiles,
                                 SmallIcon("input-tablet"), i18n("Workspace Profile"));
    spaceProfiles->setWhatsThis( i18n("<p>Set here all parameters relevant to Color Workspace "
                                      "Profiles.</p>"));

    QGridLayout *secondPageLayout = new QGridLayout(spaceProfiles);

    QWidget *box2      = new QWidget(spaceProfiles);
    QVBoxLayout *hlay2 = new QVBoxLayout(box2);
    d->spaceProfileBG  = new QButtonGroup(box2);

    d->useSpaceDefaultProfile = new QRadioButton(box2);
    d->useSpaceDefaultProfile->setText(i18n("Use default workspace profile"));
    d->spaceProfileBG->addButton(d->useSpaceDefaultProfile, 0);

    d->useSpaceSelectedProfile = new QRadioButton(box2);
    d->useSpaceSelectedProfile->setText(i18n("Use selected profile"));
    d->spaceProfileBG->addButton(d->useSpaceSelectedProfile, 1);

    hlay2->addWidget(d->useSpaceDefaultProfile);
    hlay2->addWidget(d->useSpaceSelectedProfile);
    hlay2->setMargin(0);
    hlay2->setSpacing(0);

    d->spaceProfilePath = new KUrlRequester(spaceProfiles);
    d->spaceProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    d->spaceProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));

    KFileDialog *spaceProfiles_dialog = d->spaceProfilePath->fileDialog();
    d->iccSpacePreviewWidget = new ICCPreviewWidget(spaceProfiles_dialog);
    spaceProfiles_dialog->setPreviewWidget(d->iccSpacePreviewWidget);

    QPushButton *spaceProfilesInfo = new QPushButton(i18n("Info..."), box2);

    secondPageLayout->addWidget(box2,                0, 0, 2, 1);
    secondPageLayout->addWidget(spaceProfilesInfo,   0, 2, 1, 1);
    secondPageLayout->addWidget(d->spaceProfilePath, 2, 0, 1, 3);
    secondPageLayout->setColumnStretch(1, 10);
    secondPageLayout->setRowStretch(3, 10);
    secondPageLayout->setMargin(d->gboxSettings->spacingHint());
    secondPageLayout->setSpacing(d->gboxSettings->spacingHint());

    //---------- "Proofing" Page Setup ---------------------------------

    d->toolBoxWidgets->insertItem(PROOFINGPAGE, proofProfiles,
                                 SmallIcon("printer"), i18n("Proofing Profile"));
    proofProfiles->setWhatsThis( i18n("<p>Set here all parameters relevant to Proofing Color "
                                      "Profiles.</p>"));

    QGridLayout *thirdPageLayout = new QGridLayout(proofProfiles);

    QWidget *box3       = new QWidget(proofProfiles);
    QVBoxLayout *hlay3  = new QVBoxLayout(box3);
    d->proofProfileBG   = new QButtonGroup(box3);

    d->useProofDefaultProfile = new QRadioButton(box3);
    d->useProofDefaultProfile->setText(i18n("Use default proof profile"));
    d->proofProfileBG->addButton(d->useProofDefaultProfile, 0);

    d->useProofSelectedProfile = new QRadioButton(box3);
    d->useProofSelectedProfile->setText(i18n("Use selected profile"));
    d->proofProfileBG->addButton(d->useProofSelectedProfile, 1);

    hlay3->addWidget(d->useProofDefaultProfile);
    hlay3->addWidget(d->useProofSelectedProfile);
    hlay3->setMargin(0);
    hlay3->setSpacing(0);

    d->proofProfilePath = new KUrlRequester(proofProfiles);
    d->proofProfilePath->setMode(KFile::File|KFile::ExistingOnly);
    d->proofProfilePath->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));

    KFileDialog *proofProfiles_dialog = d->proofProfilePath->fileDialog();
    d->iccProofPreviewWidget = new ICCPreviewWidget(proofProfiles_dialog);
    proofProfiles_dialog->setPreviewWidget(d->iccProofPreviewWidget);

    QPushButton *proofProfilesInfo = new QPushButton(i18n("Info..."), box3);

    thirdPageLayout->addWidget(box3,                0, 0, 2, 1);
    thirdPageLayout->addWidget(proofProfilesInfo,   0, 2, 1, 1);
    thirdPageLayout->addWidget(d->proofProfilePath, 2, 0, 1, 3);
    thirdPageLayout->setColumnStretch(1, 10);
    thirdPageLayout->setRowStretch(3, 10);
    thirdPageLayout->setMargin(d->gboxSettings->spacingHint());
    thirdPageLayout->setSpacing(d->gboxSettings->spacingHint());

    //---------- "Lightness" Page Setup ----------------------------------

    d->toolBoxWidgets->insertItem(LIGHTNESSPAGE, lightnessadjust,
                                 SmallIcon("format-stroke-color"), i18n("Lightness Adjustments"));
    lightnessadjust->setWhatsThis( i18n("<p>Set here all lightness adjustments to the target image.</p>"));

    QGridLayout *fourPageLayout = new QGridLayout( lightnessadjust );

    ColorGradientWidget* vGradient = new ColorGradientWidget(Qt::Vertical, 10, lightnessadjust);
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(lightnessadjust);
    spacev->setFixedWidth(1);

    d->curvesWidget = new CurvesWidget(256, 192, d->originalImage->bits(), d->originalImage->width(),
                                                 d->originalImage->height(), d->originalImage->sixteenBit(),
                                                 lightnessadjust);
    d->curvesWidget->setWhatsThis( i18n("This is the curve adjustment of the image luminosity"));

    QLabel *spaceh = new QLabel(lightnessadjust);
    spaceh->setFixedHeight(1);

    ColorGradientWidget *hGradient = new ColorGradientWidget(Qt::Horizontal, 10, lightnessadjust);
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    d->cInput = new RIntNumInput(lightnessadjust);
    d->cInput->input()->setLabel(i18n("Contrast:"), Qt::AlignLeft | Qt::AlignVCenter);
    d->cInput->setRange(-100, 100, 1);
    d->cInput->setSliderEnabled(true);
    d->cInput->setDefaultValue(0);
    d->cInput->setWhatsThis( i18n("Set here the contrast adjustment of the image."));

    fourPageLayout->addWidget(vGradient,       0, 0, 1, 1);
    fourPageLayout->addWidget(spacev,          0, 1, 1, 1);
    fourPageLayout->addWidget(d->curvesWidget, 0, 2, 1, 1);
    fourPageLayout->addWidget(spaceh,          1, 2, 1, 1);
    fourPageLayout->addWidget(hGradient,       2, 2, 1, 1);
    fourPageLayout->addWidget(d->cInput,       4, 0, 1, 3);
    fourPageLayout->setRowMinimumHeight(3, d->gboxSettings->spacingHint());
    fourPageLayout->setRowStretch(5, 10);
    fourPageLayout->setMargin(d->gboxSettings->spacingHint());
    fourPageLayout->setSpacing(0);

    // -------------------------------------------------------------

    gridSettings->addWidget(d->toolBoxWidgets, 0, 0, 1, 3);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    d->gboxSettings->enableButton(EditorToolSettings::Ok, false);
    init();

    // -------------------------------------------------------------

    connect(lcmsLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processLCMSUrl(const QString&)));

    connect(d->curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(d->cInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->renderingIntentsCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    //-- Check box options connections -------------------------------------------

    connect(d->doSoftProofBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(d->checkGamutBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(d->BPCBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    //-- Button Group ICC profile options connections ----------------------------

    connect(d->inProfileBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEffect()));

    connect(d->spaceProfileBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEffect()));

    connect(d->proofProfileBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotEffect()));

    //-- url requester ICC profile connections -----------------------------------

    connect(d->inProfilesPath, SIGNAL(urlSelected(const Kurl&)),
            this, SLOT(slotEffect()));

    connect(d->spaceProfilePath, SIGNAL(urlSelected(const Kurl&)),
            this, SLOT(slotEffect()));

    connect(d->proofProfilePath, SIGNAL(urlSelected(const Kurl&)),
            this, SLOT(slotEffect()));

    //-- Image preview widget connections ----------------------------

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

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
    delete [] d->destinationPreviewData;
    delete d;
}

void ICCProofTool::readSettings()
{
    QString defaultICCPath = KGlobalSettings::documentPath();
    KSharedConfig::Ptr config = KGlobal::config();

    // General settings of digiKam Color Management
    KConfigGroup group = config->group("Color Management");

    if (!group.readEntry("EnableCM", false))
    {
        d->cmEnabled = false;
        slotToggledWidgets(false);
    }
    else
    {
        d->inPath      = group.readEntry("InProfileFile");
        d->spacePath   = group.readEntry("WorkProfileFile");
        d->proofPath   = group.readEntry("ProofProfileFile");

        if (QFile::exists(group.readPathEntry("DefaultPath", QString())))
        {
            defaultICCPath = group.readPathEntry("DefaultPath", QString());
        }
        else
        {
            QString message = i18n("<p>The ICC profiles path seems to be invalid. "
                                   "You will not be able to use \"Default profile\" options.</p>"
                                   "<p>Please fix this in the digiKam ICC setup.</p>");
            slotToggledWidgets(false);
            KMessageBox::information(kapp->activeWindow(), message);
        }
    }

    // Plugin settings.
    group = config->group("colormanagement Tool");


    d->toolBoxWidgets->setCurrentIndex(group.readEntry("Settings Tab", (int)GENERALPAGE));
    d->inProfilesPath->setUrl(group.readPathEntry("InputProfilePath", defaultICCPath));
    d->proofProfilePath->setUrl(group.readPathEntry("ProofProfilePath", defaultICCPath));
    d->spaceProfilePath->setUrl(group.readPathEntry("SpaceProfilePath", defaultICCPath));
    d->renderingIntentsCB->setCurrentIndex(group.readEntry("RenderingIntent", d->renderingIntentsCB->defaultIndex()));
    d->doSoftProofBox->setChecked(group.readEntry("DoSoftProof", false));
    d->checkGamutBox->setChecked(group.readEntry("CheckGamut", false));
    d->embeddProfileBox->setChecked(group.readEntry("EmbeddProfile", true));
    d->BPCBox->setChecked(group.readEntry("BPC", true));
    d->inProfileBG->button(group.readEntry("InputProfileMethod", 0))->setChecked(true);
    d->spaceProfileBG->button(group.readEntry("SpaceProfileMethod", 0))->setChecked(true);
    d->proofProfileBG->button(group.readEntry("ProofProfileMethod", 0))->setChecked(true);
    d->cInput->setValue(group.readEntry("ContrastAdjustment", d->cInput->defaultValue()));

    for (int i = 0 ; i < 5 ; ++i)
        d->curvesWidget->curves()->curvesChannelReset(i);

    d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
    d->curvesWidget->reset();

    for (int j = 0 ; j < 17 ; ++j)
    {
        QPoint disable(-1, -1);
        QPoint p = group.readEntry(QString("CurveAdjustmentPoint%1").arg(j), disable);

        if (d->originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()*255);
            p.setY(p.y()*255);
        }

        d->curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, j, p);
    }

    for (int i = 0 ; i < 5 ; ++i)
        d->curvesWidget->curves()->curvesCalculateCurve(i);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                    (int)EditorToolSettings::LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                    (int)CurvesWidget::LogScaleHistogram));
}

void ICCProofTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("colormanagement Tool");
    group.writeEntry("Settings Tab", d->toolBoxWidgets->currentIndex());
    group.writeEntry("Histogram Channel", d->gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", d->gboxSettings->histogramBox()->scale());
    group.writeEntry("InputProfilePath", d->inProfilesPath->url());
    group.writeEntry("ProofProfilePath", d->proofProfilePath->url());
    group.writeEntry("SpaceProfilePath", d->spaceProfilePath->url());
    group.writeEntry("RenderingIntent", d->renderingIntentsCB->currentIndex());
    group.writeEntry("DoSoftProof", d->doSoftProofBox->isChecked());
    group.writeEntry("CheckGamut", d->checkGamutBox->isChecked());
    group.writeEntry("EmbeddProfile", d->embeddProfileBox->isChecked());
    group.writeEntry("BPC", d->BPCBox->isChecked());
    group.writeEntry("InputProfileMethod", d->inProfileBG->checkedId());
    group.writeEntry("SpaceProfileMethod", d->spaceProfileBG->checkedId());
    group.writeEntry("ProofProfileMethod", d->proofProfileBG->checkedId());
    group.writeEntry("ContrastAdjustment", d->cInput->value());

    for (int j = 0 ; j < 17 ; ++j)
    {
        QPoint p = d->curvesWidget->curves()->getCurvePoint(ImageHistogram::ValueChannel, j);

        if (d->originalImage->sixteenBit() && p.x() != -1)
        {
            p.setX(p.x()/255);
            p.setY(p.y()/255);
        }

        group.writeEntry(QString("CurveAdjustmentPoint%1").arg(j), p);
    }

    d->previewWidget->writeSettings();
    group.sync();
}

void ICCProofTool::processLCMSUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void ICCProofTool::slotSpotColorChanged(const DColor& color)
{
    d->curvesWidget->setCurveGuide(color);
}

void ICCProofTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ICCProofTool::slotResetSettings()
{
    d->cInput->blockSignals(true);
    d->cInput->slotReset();

    for (int i = 0 ; i < 5 ; ++i)
       d->curvesWidget->curves()->curvesChannelReset(i);

    d->curvesWidget->reset();
    d->cInput->blockSignals(false);

    slotEffect();
}

void ICCProofTool::slotEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    d->gboxSettings->enableButton(EditorToolSettings::Ok, true);
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    IccTransform transform;

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    ImageIface *iface = d->previewWidget->imageIface();
    d->destinationPreviewData   = iface->getPreviewImage();
    int  w                     = iface->previewWidth();
    int  h                     = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    DImg preview(w, h, sb, a, d->destinationPreviewData);

    QString tmpInPath;
    QString tmpProofPath;
    QString tmpSpacePath;

    bool proofCondition = false;
    bool spaceCondition = false;

    //-- Input profile parameters ------------------

    if (useDefaultInProfile())
    {
        tmpInPath = d->inPath;
    }
    else if (useSelectedInProfile())
    {
        tmpInPath = d->inProfilesPath->url().path();
        QFileInfo info(tmpInPath);
        if (!info.exists() || !info.isReadable() || !info.isFile() )
        {
            KMessageBox::information(kapp->activeWindow(),
                                     i18n("<p>The selected ICC input profile path seems to be invalid.</p>"
                                          "<p>Please check it.</p>"));
            return;
        }
    }

    //-- Proof profile parameters ------------------

    if (useDefaultProofProfile())
    {
        tmpProofPath = d->proofPath;
    }
    else
    {
        tmpProofPath = d->proofProfilePath->url().path();
        QFileInfo info(tmpProofPath);
        if (!info.exists() || !info.isReadable() || !info.isFile() )
        {
            KMessageBox::information(kapp->activeWindow(),
                                     i18n("<p>The selected ICC proof profile path seems to be invalid.</p>"
                                          "<p>Please check it.</p>"));
            return;
        }
    }

    if (d->doSoftProofBox->isChecked())
        proofCondition = tmpProofPath.isEmpty();

    //-- Workspace profile parameters --------------

    if (useDefaultSpaceProfile())
    {
        tmpSpacePath = d->spacePath;
    }
    else
    {
        tmpSpacePath = d->spaceProfilePath->url().path();
        QFileInfo info(tmpSpacePath);
        if (!info.exists() || !info.isReadable() || !info.isFile() )
        {
            KMessageBox::information(kapp->activeWindow(),
                                     i18n("<p>The selected ICC workspace profile path seems to be invalid.</p>"
                                          "<p>Please check it.</p>"));
            return;
        }
    }

    spaceCondition = tmpSpacePath.isEmpty();

    //-- Perform the color transformations ------------------

    transform.getTransformType(d->doSoftProofBox->isChecked());

    if (d->doSoftProofBox->isChecked())
    {
        if (d->useEmbeddedProfile->isChecked())
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
        if (d->useEmbeddedProfile->isChecked())
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
        KMessageBox::information(kapp->activeWindow(), error);
        d->gboxSettings->enableButton(EditorToolSettings::Ok, false);
    }
    else
    {
        if (d->useEmbeddedProfile->isChecked())
        {
            transform.apply(preview, d->embeddedICC, d->renderingIntentsCB->currentIndex(), useBPC(),
                            d->checkGamutBox->isChecked(), useBuiltinProfile());
        }
        else
        {
            QByteArray fakeProfile = QByteArray();
            transform.apply(preview, fakeProfile, d->renderingIntentsCB->currentIndex(), useBPC(),
                            d->checkGamutBox->isChecked(), useBuiltinProfile());
        }

        //-- Calculate and apply the curve on image after transformation -------------

        DImg preview2(w, h, sb, a, 0, false);
        d->curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);
        d->curvesWidget->curves()->curvesLutProcess(preview.bits(), preview2.bits(), w, h);

        //-- Adjust contrast ---------------------------------------------------------

        BCGModifier cmod;
        cmod.setContrast((double)(d->cInput->value()/100.0) + 1.00);
        cmod.applyBCG(preview2);

        iface->putPreviewImage(preview2.bits());
        d->previewWidget->updatePreview();

        //-- Update histogram --------------------------------------------------------

        memcpy(d->destinationPreviewData, preview2.bits(), preview2.numBytes());
        d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);
        kapp->restoreOverrideCursor();
    }
}

void ICCProofTool::finalRendering()
{
    if (!d->doSoftProofBox->isChecked())
    {
        kapp->setOverrideCursor( Qt::WaitCursor );

        ImageIface *iface = d->previewWidget->imageIface();
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
                tmpInPath = d->inPath;
            }
            else if (useSelectedInProfile())
            {
                tmpInPath = d->inProfilesPath->url().path();
                QFileInfo info(tmpInPath);
                if (!info.exists() || !info.isReadable() || !info.isFile() )
                {
                    KMessageBox::information(kapp->activeWindow(),
                                             i18n("<p>The selected ICC input profile path seems to be invalid.</p>"
                                                  "<p>Please check it.</p>"));
                    return;
                }
            }

            //-- Proof profile parameters ------------------

            if (useDefaultProofProfile())
            {
                tmpProofPath = d->proofPath;
            }
            else
            {
                tmpProofPath = d->proofProfilePath->url().path();
                QFileInfo info(tmpProofPath);
                if (!info.exists() || !info.isReadable() || !info.isFile() )
                {
                    KMessageBox::information(kapp->activeWindow(),
                                             i18n("<p>The selected ICC proof profile path seems to be invalid.</p>"
                                                  "<p>Please check it.</p>"));
                    return;
                }
            }

            if (tmpProofPath.isNull())
                proofCondition = false;

            //-- Workspace profile parameters --------------

            if (useDefaultSpaceProfile())
            {
                tmpSpacePath = d->spacePath;
            }
            else
            {
                tmpSpacePath = d->spaceProfilePath->url().path();
                QFileInfo info(tmpSpacePath);
                if (!info.exists() || !info.isReadable() || !info.isFile() )
                {
                    KMessageBox::information(kapp->activeWindow(),
                                             i18n("<p>The selected ICC workspace profile path seems to be invalid.</p>"
                                                   "<p>Please check it.</p>"));
                    return;
                }
            }

            //-- Perform the color transformations ------------------

            transform.getTransformType(d->doSoftProofBox->isChecked());

            if (d->doSoftProofBox->isChecked())
            {
                if (d->useEmbeddedProfile->isChecked())
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
                if (d->useEmbeddedProfile->isChecked())
                {
                    transform.setProfiles( tmpSpacePath );
                }
                else
                {
                    transform.setProfiles( tmpInPath, tmpSpacePath );
                }
            }

            if (d->useEmbeddedProfile->isChecked())
            {
                transform.apply(img, d->embeddedICC, d->renderingIntentsCB->currentIndex(), useBPC(),
                                d->checkGamutBox->isChecked(), useBuiltinProfile());
            }
            else
            {
                QByteArray fakeProfile = QByteArray();
                transform.apply(img, fakeProfile, d->renderingIntentsCB->currentIndex(), useBPC(),
                                d->checkGamutBox->isChecked(), useBuiltinProfile());
            }

            //-- Embed the workspace profile if necessary --------------------------------

            if (d->embeddProfileBox->isChecked())
            {
                iface->setEmbeddedICCToOriginalImage( tmpSpacePath );
                kDebug(50006) << QFile::encodeName(tmpSpacePath) << endl;
            }

            //-- Calculate and apply the curve on image after transformation -------------

            DImg img2(w, h, sb, a, 0, false);
            d->curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);
            d->curvesWidget->curves()->curvesLutProcess(img.bits(), img2.bits(), w, h);

            //-- Adjust contrast ---------------------------------------------------------

            BCGModifier cmod;
            cmod.setContrast((double)(d->cInput->value()/100.0) + 1.00);
            cmod.applyBCG(img2);

            iface->putOriginalImage("Color Management", img2.bits());
            delete [] data;
        }

        kapp->restoreOverrideCursor();
    }
}

void ICCProofTool::slotToggledWidgets( bool t)
{
    d->useInDefaultProfile->setEnabled(t);
    d->useProofDefaultProfile->setEnabled(t);
    d->useSpaceDefaultProfile->setEnabled(t);
}

void ICCProofTool::slotInICCInfo()
{
    if (useEmbeddedProfile())
    {
        getICCInfo(d->embeddedICC);
    }
    else if(useBuiltinProfile())
    {
        QString message = i18n("<p>You have selected the \"Default built-in sRGB profile\"</p>");
        message.append(i18n("<p>This profile is built on the fly, so there is no relevant information "
                            "about it.</p>"));
        KMessageBox::information(kapp->activeWindow(), message);
    }
    else if (useDefaultInProfile())
    {
        getICCInfo(d->inPath);
    }
    else if (useSelectedInProfile())
    {
        getICCInfo(d->inProfilesPath->url().path());
    }
}

void ICCProofTool::slotProofICCInfo()
{
    if (useDefaultProofProfile())
    {
        getICCInfo(d->proofPath);
    }
    else
    {
        getICCInfo(d->proofProfilePath->url().path());
    }
}

void ICCProofTool::slotSpaceICCInfo()
{
    if (useDefaultSpaceProfile())
    {
        getICCInfo(d->spacePath);
    }
    else
    {
        getICCInfo(d->spaceProfilePath->url().path());
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
                           i18n("Sorry, it seems there is no embedded profile"), i18n("Profile Error"));
        return;
    }

    ICCProfileInfoDlg infoDlg(kapp->activeWindow(), QString(), profile);
    infoDlg.exec();
}

void ICCProofTool::slotCMDisabledWarning()
{
    if (!d->cmEnabled)
    {
        QString message = i18n("<p>You have not enabled Color Management in the digiKam preferences.</p>");
        message.append( i18n("<p>\"Use of default profile\" options will be disabled now.</p>"));
        KMessageBox::information(kapp->activeWindow(), message);
        slotToggledWidgets(false);
    }
}

//-- General Tab ---------------------------

bool ICCProofTool::useBPC()
{
    return d->BPCBox->isChecked();
}

bool ICCProofTool::doProof()
{
    return d->doSoftProofBox->isChecked();
}

bool ICCProofTool::checkGamut()
{
    return d->checkGamutBox->isChecked();
}

bool ICCProofTool::embedProfile()
{
    return d->embeddProfileBox->isChecked();
}

//-- Input Tab ---------------------------

bool ICCProofTool::useEmbeddedProfile()
{
    return d->useEmbeddedProfile->isChecked();
}

bool ICCProofTool::useBuiltinProfile()
{
    return d->useSRGBDefaultProfile->isChecked();
}

bool ICCProofTool::useDefaultInProfile()
{
    return d->useInDefaultProfile->isChecked();
}

bool ICCProofTool::useSelectedInProfile()
{
    return d->useInSelectedProfile->isChecked();
}

//-- Workspace Tab ---------------------------

bool ICCProofTool::useDefaultSpaceProfile()
{
    return d->useSpaceDefaultProfile->isChecked();
}

//-- Proofing Tab ---------------------------

bool ICCProofTool::useDefaultProofProfile()
{
    return d->useProofDefaultProfile->isChecked();
}

//-- Load all settings from file --------------------------------------

void ICCProofTool::slotLoadSettings()
{
    KUrl loadColorManagementFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                                QString( "*" ), kapp->activeWindow(),
                                                QString( i18n("Color Management Settings File to Load")) );
    if( loadColorManagementFile.isEmpty() )
       return;

    QFile file(loadColorManagementFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );

        if ( stream.readLine() != "# Color Management Configuration File" )
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Color Management settings text file.",
                             loadColorManagementFile.fileName()));
           file.close();
           return;
        }

        blockSignals(true);

        d->renderingIntentsCB->setCurrentIndex( stream.readLine().toInt() );
        d->doSoftProofBox->setChecked( (bool)(stream.readLine().toUInt()) );
        d->checkGamutBox->setChecked( (bool)(stream.readLine().toUInt()) );
        d->embeddProfileBox->setChecked( (bool)(stream.readLine().toUInt()) );
        d->BPCBox->setChecked( (bool)(stream.readLine().toUInt()) );
        d->inProfileBG->button( stream.readLine().toInt() )->setChecked(true);
        d->spaceProfileBG->button( stream.readLine().toInt() )->setChecked(true);
        d->proofProfileBG->button( stream.readLine().toInt() )->setChecked(true);
        d->inProfilesPath->setUrl( stream.readLine() );
        d->proofProfilePath->setUrl( stream.readLine() );
        d->spaceProfilePath->setUrl( stream.readLine() );
        d->cInput->setValue( stream.readLine().toInt() );

        for (int i = 0 ; i < 5 ; ++i)
            d->curvesWidget->curves()->curvesChannelReset(i);

        d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
        d->curvesWidget->reset();

        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint disable(-1, -1);
            QPoint p;
            p.setX( stream.readLine().toInt() );
            p.setY( stream.readLine().toInt() );

            if (d->originalImage->sixteenBit() && p != disable)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            d->curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, j, p);
        }

        blockSignals(false);

        for (int i = 0 ; i < 5 ; ++i)
           d->curvesWidget->curves()->curvesCalculateCurve(i);

        d->gboxSettings->histogramBox()->histogram()->reset();
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
    KUrl saveColorManagementFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                                QString( "*" ), kapp->activeWindow(),
                                                QString( i18n("Color Management Settings File to Save")) );
    if( saveColorManagementFile.isEmpty() )
       return;

    QFile file(saveColorManagementFile.path());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Color Management Configuration File\n";
        stream << d->renderingIntentsCB->currentIndex() << "\n";
        stream << d->doSoftProofBox->isChecked() << "\n";
        stream << d->checkGamutBox->isChecked() << "\n";
        stream << d->embeddProfileBox->isChecked() << "\n";
        stream << d->BPCBox->isChecked() << "\n";
        stream << d->inProfileBG->checkedId() << "\n";
        stream << d->spaceProfileBG->checkedId() << "\n";
        stream << d->proofProfileBG->checkedId() << "\n";
        stream << d->inProfilesPath->url().path() << "\n";
        stream << d->proofProfilePath->url().path() << "\n";
        stream << d->spaceProfilePath->url().path() << "\n";
        stream << d->cInput->value() << "\n";

        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint p = d->curvesWidget->curves()->getCurvePoint(ImageHistogram::ValueChannel, j);
            if (d->originalImage->sixteenBit())
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
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

} // namespace DigikamImagesPluginCore
