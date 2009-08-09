/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-24
 * Description : Color management setup tab.
 *
 * Copyright (C) 2005-2007 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupicc.h"
#include "setupicc.moc"

// Qt includes

#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>
#include <QMap>
#include <QDir>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes

#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpagedialog.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kurlrequester.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/squeezedcombobox.h>

// Local includes

#include "iccprofileinfodlg.h"
#include "iccsettingscontainer.h"
#include "albumsettings.h"

using namespace KDcrawIface;

namespace Digikam
{

class SetupICCPriv
{
public:

     SetupICCPriv()
     {
        enableColorManagement = 0;
        bpcAlgorithm          = 0;
        managedView           = 0;
        defaultApplyICC       = 0;
        defaultAskICC         = 0;
        defaultDoNotApplyICC  = 0;
        defaultPathKU         = 0;
        inProfilesKC          = 0;
        workProfilesKC        = 0;
        proofProfilesKC       = 0;
        monitorProfilesKC     = 0;
        renderingIntentKC     = 0;
        infoWorkProfiles      = 0;
        infoMonitorProfiles   = 0;
        infoInProfiles        = 0;
        infoProofProfiles     = 0;
        workspaceGB           = 0;
        openingGB             = 0;
        profilesGB            = 0;
        advancedSettingsGB    = 0;
        monitorIcon           = 0;
        monitorProfiles       = 0;
        iccPathsRead          = false;
     }

    QLabel                 *monitorIcon;
    QLabel                 *monitorProfiles;

    QCheckBox              *enableColorManagement;
    QCheckBox              *bpcAlgorithm;
    QCheckBox              *managedView;

    QRadioButton           *defaultApplyICC;
    QRadioButton           *defaultAskICC;
    QRadioButton           *defaultDoNotApplyICC;

    QPushButton            *infoWorkProfiles;
    QPushButton            *infoMonitorProfiles;
    QPushButton            *infoInProfiles;
    QPushButton            *infoProofProfiles;

    QGroupBox              *workspaceGB;
    QGroupBox              *openingGB;
    QGroupBox              *profilesGB;
    QGroupBox              *advancedSettingsGB;

    // Maps to store profile descriptions and profile file path
    QMap<QString, QString>  inICCPath;
    QMap<QString, QString>  workICCPath;
    QMap<QString, QString>  proofICCPath;
    QMap<QString, QString>  monitorICCPath;

    KUrlRequester          *defaultPathKU;

    KComboBox              *renderingIntentKC;

    KPageDialog            *mainDialog;

    SqueezedComboBox       *inProfilesKC;
    SqueezedComboBox       *workProfilesKC;
    SqueezedComboBox       *proofProfilesKC;
    SqueezedComboBox       *monitorProfilesKC;

    bool                    iccPathsRead;
};

SetupICC::SetupICC(QWidget* parent, KPageDialog* dialog )
        : QScrollArea(parent), d(new SetupICCPriv)
{
    d->mainDialog = dialog;

    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QWidget *colorPolicy  = new QWidget;
    QGridLayout* grid      = new QGridLayout(colorPolicy);

    d->enableColorManagement = new QCheckBox(colorPolicy);
    d->enableColorManagement->setText(i18n("Enable Color Management"));
    d->enableColorManagement->setWhatsThis( i18n("<ul><li>Checked: Color Management is enabled</li>"
                                                 "<li>Unchecked: Color Management is "
                                                 "disabled</li></ul>"));

    KUrlLabel *lcmsLogoLabel = new KUrlLabel(colorPolicy);
    lcmsLogoLabel->setText(QString());
    lcmsLogoLabel->setUrl("http://www.littlecms.com");
    lcmsLogoLabel->setPixmap( QPixmap( KStandardDirs::locate("data", "digikam/data/logo-lcms.png" ) ));
    lcmsLogoLabel->setToolTip(i18n("Visit Little CMS project website"));

    grid->addWidget(d->enableColorManagement, 0, 0, 1, 1);
    grid->addWidget(lcmsLogoLabel,            0, 2, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);

    // --------------------------------------------------------

    d->workspaceGB       = new QGroupBox(i18n("Working Color Space"), colorPolicy);
    QHBoxLayout *hboxWS  = new QHBoxLayout(d->workspaceGB);

    /*QLabel *workIcon     = new QLabel(d->profilesGB);
    workIcon->setPixmap(SmallIcon("input-tablet"));
    QLabel *workProfiles = new QLabel(i18n("Working Space:"), d->profilesGB);
    workProfiles->setBuddy(d->workProfilesKC);*/
    d->workProfilesKC    = new SqueezedComboBox(d->workspaceGB);
    d->workProfilesKC->setWhatsThis( i18n("<p>This is the color space all the images will be converted to when opened "
                                          "(if you choose to convert) and the profile that will be embedded when saving. "
                                          "Good and safe choices are <b>Adobe RGB (1998)</b> and <b>sRGB IEC61966-2.1</b>"));

    d->infoWorkProfiles = new QPushButton(d->workspaceGB);
    d->infoWorkProfiles->setIcon(SmallIcon("documentinfo"));
    d->infoWorkProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected workspace profile.</p>"));

    hboxWS->addWidget(d->workProfilesKC, 10);
    hboxWS->addWidget(d->infoWorkProfiles);

    // --------------------------------------------------------

    d->openingGB                   = new QGroupBox(i18n("Behavior on Profile Mismatch"), colorPolicy);
    QVBoxLayout *vlayOGB           = new QVBoxLayout(d->openingGB);
    QButtonGroup *behaviourOptions = new QButtonGroup(d->openingGB);

    QLabel *behaviorIcon = new QLabel;
    behaviorIcon->setPixmap(SmallIcon("editimage", KIconLoader::SizeSmallMedium));
    QLabel *behaviorLabel = new QLabel(i18n("When the profile of an image that is opened in the image editor "
                                            "does not match the working color space:"));
    behaviorLabel->setWordWrap(true);

    QHBoxLayout *hboxBL = new QHBoxLayout;
    hboxBL->addWidget(behaviorIcon);
    hboxBL->addWidget(behaviorLabel, 10);

    d->defaultAskICC = new QRadioButton(d->openingGB);
    d->defaultAskICC->setText(i18n("Ask when opening the image"));
    d->defaultAskICC->setWhatsThis( i18n("<p>If this option is enabled, digiKam asks the user "
                     "before it applies the Workspace default color profile to an image which has no "
                     "embedded profile or, if the image has an embedded profile, when it is not the same "
                     "as the workspace profile.</p>"));
    behaviourOptions->addButton(d->defaultAskICC);

    d->defaultApplyICC = new QRadioButton(d->openingGB);
    d->defaultApplyICC->setText(i18n("Convert the image to the working color space"));
    d->defaultApplyICC->setWhatsThis( i18n("<p>If this option is enabled, digiKam applies the "
                     "working color profile to an image, without prompting you about missing "
                     "embedded profiles or embedded profiles different from the workspace "
                     "profile.</p>"));
    behaviourOptions->addButton(d->defaultApplyICC);

    d->defaultDoNotApplyICC = new QRadioButton(d->openingGB);
    d->defaultDoNotApplyICC->setText(i18n("Leave the image as is without conversion"));
    d->defaultDoNotApplyICC->setWhatsThis( i18n("<p>If this option is enabled, digiKam does not apply the "
                     "workspace color profile to an image, without prompting you about missing "
                     "embedded profiles or embedded profiles different from the workspace "
                     "profile.</p>"));
    behaviourOptions->addButton(d->defaultDoNotApplyICC);

    vlayOGB->addLayout(hboxBL);
    vlayOGB->addWidget(d->defaultAskICC);
    vlayOGB->addWidget(d->defaultApplyICC);
    vlayOGB->addWidget(d->defaultDoNotApplyICC);

    // --------------------------------------------------------

    d->profilesGB      = new QGroupBox(i18n("ICC Profiles Settings"), panel);
    QGridLayout* grid2 = new QGridLayout(d->profilesGB);

    d->managedView = new QCheckBox(d->profilesGB);
    d->managedView->setText(i18n("Use color managed view (warning: slow)"));
    d->managedView->setWhatsThis( i18n("<p>Turn on this option if "
                     "you want to use your <b>Monitor Color Profile</b> to show your pictures in "
                     "the Image Editor window with a color correction adapted to your monitor. "
                     "Warning: this option can take a while to render "
                     "pictures on the screen, especially with a slow computer.</p>"));

    d->monitorIcon       = new QLabel(d->profilesGB);
    d->monitorIcon->setPixmap(SmallIcon("video-display"));
    d->monitorProfiles   = new QLabel(i18n("Monitor:"), d->profilesGB);
    d->monitorProfilesKC = new SqueezedComboBox(d->profilesGB);
    d->monitorProfiles->setBuddy(d->monitorProfilesKC);
    d->monitorProfilesKC->setWhatsThis( i18n("<p>Select the color profile for your monitor here. "
                     "You need to enable the <b>Use color managed view</b> option to use this "
                     "profile.</p>"));

    d->infoMonitorProfiles = new QPushButton(d->profilesGB);
    d->infoMonitorProfiles->setIcon(SmallIcon("documentinfo"));
    d->infoMonitorProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected monitor profile.</p>"));

    QLabel *inIcon     = new QLabel(d->profilesGB);
    inIcon->setPixmap(SmallIcon("camera-photo"));
    QLabel *inProfiles = new QLabel(i18n("Input:"), d->profilesGB);
    d->inProfilesKC    = new SqueezedComboBox(d->profilesGB);
    inProfiles->setBuddy(d->inProfilesKC);
    d->inProfilesKC->setWhatsThis( i18n("<p>Select a default profile for your input device "
                     "(usually your camera or your scanner)</p>"));

    d->infoInProfiles = new QPushButton(d->profilesGB);
    d->infoInProfiles->setIcon(SmallIcon("documentinfo"));
    d->infoInProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected input profile.</p>"));

    QLabel *proofIcon     = new QLabel(d->profilesGB);
    proofIcon->setPixmap(SmallIcon("printer"));
    QLabel *proofProfiles = new QLabel(i18n("Soft proof:"), d->profilesGB);
    d->proofProfilesKC    = new SqueezedComboBox(d->profilesGB);
    proofProfiles->setBuddy(d->proofProfilesKC);
    d->proofProfilesKC->setWhatsThis( i18n("<p>You must select the profile for your output device "
                     "(usually, your printer). This profile will be used to do a soft proof, so you will "
                     "be able to preview how an image will be rendered via an output device.</p>"));

    d->infoProofProfiles = new QPushButton(d->profilesGB);
    d->infoProofProfiles->setIcon(SmallIcon("documentinfo"));
    d->infoProofProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected soft proof profile.</p>"));

    QLabel *defaultPathLabel = new QLabel(i18n("Folder with additional color profiles:"));
    d->defaultPathKU = new KUrlRequester(d->profilesGB);
    d->defaultPathKU->lineEdit()->setReadOnly(true);
    d->defaultPathKU->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);
    d->defaultPathKU->setWhatsThis( i18n("<p>DigiKam searches ICC profiles in default system folders "
                                         "and ships itself a few selected profiles. "
                                         "Store all your additional color profiles in the directory set here.</p>"));

    grid2->addWidget(d->managedView,         0, 0, 1, 4);
    grid2->addWidget(d->monitorIcon,         1, 0, 1, 1);
    grid2->addWidget(d->monitorProfiles,     1, 1, 1, 1);
    grid2->addWidget(d->monitorProfilesKC,   1, 2, 1, 1);
    grid2->addWidget(d->infoMonitorProfiles, 1, 3, 1, 1);
    grid2->addWidget(inIcon,                 2, 0, 1, 1);
    grid2->addWidget(inProfiles,             2, 1, 1, 1);
    grid2->addWidget(d->inProfilesKC,        2, 2, 1, 1);
    grid2->addWidget(d->infoInProfiles,      2, 3, 1, 1);
    grid2->addWidget(proofIcon,              3, 0, 1, 1);
    grid2->addWidget(proofProfiles,          3, 1, 1, 1);
    grid2->addWidget(d->proofProfilesKC,     3, 2, 1, 1);
    grid2->addWidget(d->infoProofProfiles,   3, 3, 1, 1);
    grid2->addWidget(defaultPathLabel,       4, 0, 1, 4);
    grid2->addWidget(d->defaultPathKU,       5, 0, 1, 4);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(0);
    grid2->setColumnStretch(2, 10);

    // --------------------------------------------------------

    d->advancedSettingsGB = new QGroupBox(i18n("Advanced Settings"), panel);
    QGridLayout* grid3    = new QGridLayout(d->advancedSettingsGB);

    d->bpcAlgorithm = new QCheckBox(d->advancedSettingsGB);
    d->bpcAlgorithm->setText(i18n("Use black point compensation"));
    d->bpcAlgorithm->setWhatsThis( i18n("<p><b>Black Point Compensation</b> is a way to make "
                     "adjustments between the maximum "
                     "black levels of digital files and the black capabilities of various "
                     "digital devices.</p>"));

    QLabel *lablel = new QLabel(d->advancedSettingsGB);
    lablel->setText(i18n("Rendering Intents:"));

    d->renderingIntentKC = new KComboBox(false, d->advancedSettingsGB);
    d->renderingIntentKC->addItem("Perceptual", IccTransform::Perceptual);
    d->renderingIntentKC->addItem("Relative Colorimetric", IccTransform::RelativeColorimetric);
    d->renderingIntentKC->addItem("Saturation", IccTransform::Saturation);
    d->renderingIntentKC->addItem("Absolute Colorimetric", IccTransform::AbsoluteColorimetric);
    d->renderingIntentKC->setWhatsThis( i18n("<ul><li><p><b>Perceptual intent</b> causes the full gamut of the image to be "
                     "compressed or expanded to fill the gamut of the destination device, so that gray balance is "
                     "preserved but colorimetric accuracy may not be preserved.</p>"
                     "<p>In other words, if certain colors in an image fall outside of the range of colors that the output "
                     "device can render, the image intent will cause all the colors in the image to be adjusted so that "
                     "the every color in the image falls within the range that can be rendered and so that the relationship "
                     "between colors is preserved as much as possible.</p>"
                     "<p>This intent is most suitable for display of photographs and images, and is the default intent.</p></li>"
                     "<li><p><b>Absolute Colorimetric intent</b> causes any colors that fall outside the range that the output device "
                     "can render to be adjusted to the closest color that can be rendered, while all other colors are "
                     "left unchanged.</p>"
                     "<p>This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, "
                     "logo colors, ....)</p></li>"
                     "<li><p><b>Relative Colorimetric intent</b> is defined such that any colors that fall outside the range that the "
                     "output device can render are adjusted to the closest color that can be rendered, while all other colors "
                     "are left unchanged. Proof intent does not preserve the white point.</p></li>"
                     "<li><p><b>Saturation intent</b> preserves the saturation of colors in the image at the possible expense of "
                     "hue and lightness.</p>"
                     "<p>Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to "
                     "achieve the desired effects.</p>"
                     "<p>This intent is most suitable for business graphics such as charts, where it is more important that the "
                     "colors be vivid and contrast well with each other rather than a specific color.</p></li></ul>"));

    grid3->addWidget(d->bpcAlgorithm,      0, 0, 1, 2);
    grid3->addWidget(lablel,               1, 0, 1, 1);
    grid3->addWidget(d->renderingIntentKC, 1, 1, 1, 1);
    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(0);

    layout->addWidget(colorPolicy);
    layout->addWidget(d->workspaceGB);
    layout->addWidget(d->openingGB);
    layout->addWidget(d->profilesGB);
    layout->addWidget(d->advancedSettingsGB);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->enableColorManagement, SIGNAL(toggled(bool)),
            this, SLOT(slotToggledEnabled()));

    connect(d->managedView, SIGNAL(toggled(bool)),
            this, SLOT(slotToggledManagedView()));

    connect(lcmsLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processLcmsUrl(const QString&)));

    connect(d->infoProofProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedProof()) );

    connect(d->infoInProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedIn()) );

    connect(d->infoMonitorProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedMonitor()) );

    connect(d->infoWorkProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedWork()));

    connect(d->defaultPathKU, SIGNAL(urlSelected(const KUrl&)),
            this, SLOT(slotUrlChanged()));

    // --------------------------------------------------------

    adjustSize();

    readSettings();
    slotToggledEnabled();
}

SetupICC::~SetupICC()
{
    delete d;
}

void SetupICC::processLcmsUrl(const QString& url)
{
    KToolInvocation::self()->invokeBrowser(url);
}

void SetupICC::applySettings()
{
    ICCSettingsContainer settings;
    settings.enableCM = d->enableColorManagement->isChecked();

    if (d->defaultApplyICC->isChecked())
        settings.onProfileMismatch = ICCSettingsContainer::Convert;
    else if (d->defaultDoNotApplyICC->isChecked())
        settings.onProfileMismatch = ICCSettingsContainer::Leave;
    else
        settings.onProfileMismatch = ICCSettingsContainer::Ask;

    settings.iccFolder = d->defaultPathKU->url().path();
    settings.useBPC =  d->bpcAlgorithm->isChecked();
    settings.renderingIntent = d->renderingIntentKC->itemData(d->renderingIntentKC->currentIndex()).toInt();
    settings.useManagedView = d->managedView->isChecked();

    settings.defaultInputProfile = d->inICCPath.value(d->inProfilesKC->itemHighlighted());
    settings.workspaceProfile = d->workICCPath.value(d->workProfilesKC->itemHighlighted());
    settings.monitorProfile = d->monitorICCPath.value(d->monitorProfilesKC->itemHighlighted());
    settings.defaultProofProfile = d->proofICCPath.value(d->proofProfilesKC->itemHighlighted());

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Color Management"));
    settings.writeToConfig(group);
}

static void setCurrentIndexFromUserData(QComboBox *box, const QVariant& userData)
{
    if (userData.isNull())
    {
        box->setCurrentIndex(-1);
        return;
    }

    const int size = box->count();
    for (int i=0; i<size; i++)
    {
        if (box->itemData(i) == userData)
        {
            box->setCurrentIndex(i);
            return;
        }
    }
    box->setCurrentIndex(-1);
}

void SetupICC::readSettings(bool restore)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Color Management"));
    ICCSettingsContainer settings;
    settings.readFromConfig(group);

    if (!restore)
        d->enableColorManagement->setChecked(settings.enableCM);

    d->bpcAlgorithm->setChecked(settings.useBPC);
    setCurrentIndexFromUserData(d->renderingIntentKC, settings.renderingIntent);
    d->managedView->setChecked(settings.useManagedView);

    if (settings.onProfileMismatch == ICCSettingsContainer::Convert)
        d->defaultApplyICC->setChecked(true);
    else if (settings.onProfileMismatch == ICCSettingsContainer::Leave)
        d->defaultDoNotApplyICC->setChecked(true);
    else
        d->defaultAskICC->setChecked(true);

    d->defaultPathKU->setUrl(settings.iccFolder);
    fillCombos(false);

    setCurrentIndexFromUserData(d->workProfilesKC, settings.workspaceProfile);
    setCurrentIndexFromUserData(d->monitorProfilesKC, settings.monitorProfile);
    setCurrentIndexFromUserData(d->inProfilesKC, settings.defaultInputProfile);
    setCurrentIndexFromUserData(d->proofProfilesKC, settings.defaultProofProfile);
}

void SetupICC::slotUrlChanged()
{
    d->iccPathsRead = false;
    fillCombos(true);
}

void SetupICC::fillCombos(bool report)
{
    if (d->iccPathsRead)
        return;

    if (!d->enableColorManagement->isChecked())
        return;

    d->inProfilesKC->clear();
    d->monitorProfilesKC->clear();
    d->workProfilesKC->clear();
    d->proofProfilesKC->clear();
    d->inICCPath.clear();
    d->workICCPath.clear();
    d->proofICCPath.clear();
    d->monitorICCPath.clear();

    QString extraPath = d->defaultPathKU->url().path();

    QList<IccProfile> profiles;
    // get system paths, e.g. /usr/share/color/icc
    QStringList paths = IccProfile::defaultSearchPaths();
    // add user-specified path
    paths << extraPath;
    // check search directories
    profiles << scanDirectories(paths);
    // load profiles that come with libkdcraw
    profiles << IccProfile::defaultProfiles();

    if ( profiles.isEmpty() )
    {
        if (report)
        {
            QString message = i18n("No ICC profiles files found.");
            KMessageBox::sorry(this, message);
        }

        kDebug(50003) << "No ICC profile files found!!!";
        d->mainDialog->enableButtonOk(false);
        return;
    }

    parseProfiles(profiles);

    QMap<QString, QString>::const_iterator it;
    for (it = d->monitorICCPath.constBegin(); it != d->monitorICCPath.constEnd(); ++it)
        d->monitorProfilesKC->addSqueezedItem(it.key(), it.value());
    for (it = d->inICCPath.constBegin(); it != d->inICCPath.constEnd(); ++it)
        d->inProfilesKC->addSqueezedItem(it.key(), it.value());
    for (it = d->proofICCPath.constBegin(); it != d->proofICCPath.constEnd(); ++it)
        d->proofProfilesKC->addSqueezedItem(it.key(), it.value());
    for (it = d->workICCPath.constBegin(); it != d->workICCPath.constEnd(); ++it)
        d->workProfilesKC->addSqueezedItem(it.key(), it.value());

    if (d->monitorICCPath.keys().isEmpty())
    {
        d->managedView->setEnabled(false);
        d->managedView->setChecked(false);
    }
    else
    {
        d->managedView->setEnabled(true);
    }

    if (d->workICCPath.isEmpty())
    {
        // If there is no workspace ICC profiles available,
        // the CM is broken and cannot be used.
        d->mainDialog->enableButtonOk(false);
        return;
    }

    d->iccPathsRead = true;
    d->mainDialog->enableButtonOk(true);
}

QList<IccProfile> SetupICC::scanDirectories(const QStringList& dirs)
{
    QList<IccProfile> profiles;

    QStringList filters;
    filters << "*.icc" << "*.icm";
    kDebug() << dirs;
    foreach (const QString &dirPath, dirs)
    {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;
        scanDirectory(dir.path(), filters, &profiles);
    }

    return profiles;
}

void SetupICC::scanDirectory(const QString& path, const QStringList& filter, QList<IccProfile> *profiles)
{
    QDir dir(path);
    QFileInfoList infos;
    infos << dir.entryInfoList(filter, QDir::Files | QDir::Readable);
    infos << dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &info, infos)
    {
        if (info.isFile())
        {
            //kDebug(50003) << info.filePath() << (info.exists() && info.isReadable());
            *profiles << IccProfile(info.filePath());
        }
        else if (info.isDir())
        {
            scanDirectory(info.filePath(), filter, profiles);
        }
    }
}

void SetupICC::parseProfiles(const QList<IccProfile>& profiles)
{
    foreach (IccProfile profile, profiles)
    {
        QString filePath = profile.filePath();

        if (!profile.open())
        {
            kError(50003) << "Cannot open profile" << filePath;
            /*
            QString message = i18n("<p>The following profile is invalid:</p><p><b>%1</b></p>"
            "<p>To avoid this message, remove the profile from the color profiles repository.</p>"
            "<p>Do you want digiKam to do this for you?</p>", message);
            if (KMessageBox::warningYesNo(this, message, i18n("Invalid Profile")) == 3)
            {
                if (QFile::remove(filePath))
                {
                    KMessageBox::information(this,  i18n("Invalid color profile has been removed."));
                }
                else
                {
                    KMessageBox::information(this, i18n("<p>digiKam has failed to remove the invalid color profile.</p>"
                    "<p>Remove it manually.</p>"));
                }
            }
            */
            continue;
        }

        QFileInfo info(filePath);
        QString fileName = info.fileName();

        QString description = profile.description();
        if (description.isEmpty())
            description = fileName;
        else
            description = i18nc("<Profile Description> (<File Name>)", "%1 (%2)", description, fileName);

        switch (profile.type())
        {
            case IccProfile::Input:
                d->inICCPath.insert(description, filePath);
                kDebug(50003) << "Input ICC profile" << filePath;
                break;
            case IccProfile::Display:
                d->monitorICCPath.insert(description, filePath);
                d->workICCPath.insert(description, filePath);
                kDebug(50003) << "Display ICC profile" << filePath;
                break;
            case IccProfile::Output:
                d->proofICCPath.insert(description, filePath);
                kDebug(50003) << "Output ICC profile" << filePath;
                break;
            case IccProfile::ColorSpace:
                d->inICCPath.insert(description, filePath);
                d->workICCPath.insert(description, filePath);
                kDebug(50003) << "ColorSpace ICC profile" << filePath;
                break;
            case IccProfile::DeviceLink:
            case IccProfile::Abstract:
            case IccProfile::NamedColor:
                kDebug(50003) << "ICC profile of unused profile type:" << filePath;
                break;
            default:
                kDebug(50003) << "Invalid ICC profile" << filePath;
                break;
        }
    }
}

void SetupICC::setWidgetsEnabled(bool enabled)
{
    d->workspaceGB->setEnabled(enabled);
    d->openingGB->setEnabled(enabled);
    d->profilesGB->setEnabled(enabled);
    d->advancedSettingsGB->setEnabled(enabled);
}

void SetupICC::slotToggledEnabled()
{
    bool enabled = d->enableColorManagement->isChecked();

    setWidgetsEnabled(enabled);

    if (enabled)
    {
        readSettings(true);
        slotToggledManagedView();
    }
    else
    {
        d->mainDialog->enableButtonOk(true);
    }
}

void SetupICC::slotToggledManagedView()
{
    bool enabled = d->managedView->isChecked();
    d->monitorIcon->setEnabled(enabled);
    d->monitorProfiles->setEnabled(enabled);
    d->monitorProfilesKC->setEnabled(enabled);
    d->infoMonitorProfiles->setEnabled(enabled);
}

void SetupICC::slotClickedWork()
{
    if (!d->workProfilesKC->itemHighlighted().isEmpty())
       profileInfo(*(d->workICCPath.find(d->workProfilesKC->itemHighlighted())));
}

void SetupICC::slotClickedIn()
{
    if (!d->inProfilesKC->itemHighlighted().isEmpty())
       profileInfo(*(d->inICCPath.find(d->inProfilesKC->itemHighlighted())));
}

void SetupICC::slotClickedMonitor()
{
    if (!d->monitorProfilesKC->itemHighlighted().isEmpty())
       profileInfo(*(d->monitorICCPath.find(d->monitorProfilesKC->itemHighlighted())));
}

void SetupICC::slotClickedProof()
{
    if (!d->proofProfilesKC->itemHighlighted().isEmpty())
       profileInfo(*(d->proofICCPath.find(d->proofProfilesKC->itemHighlighted())));
}

void SetupICC::profileInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(this, i18n("No profile is selected."), i18n("Profile Error"));
        return;
    }

    ICCProfileInfoDlg infoDlg(this, profile);
    infoDlg.exec();
}

bool SetupICC::iccRepositoryIsValid()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Color Management"));

    // If color management is disable, no need to check anymore.
    if (!group.readEntry("EnableCM", false))
        return true;


    // can at least libkdcraw profiles be opened?
    if (IccProfile::sRGB().open())
        return true;

    // To be valid, the ICC profiles repository must exist and be readable.
    QString extraPath = group.readEntry("DefaultPath", QString());
    QFileInfo info(extraPath);
    if (info.isDir() && info.exists() && info.isReadable())
        return true;
    QStringList paths = IccProfile::defaultSearchPaths();
    return !paths.isEmpty();
}

}  // namespace Digikam
