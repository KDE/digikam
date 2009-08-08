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

// lcms includes

#include <lcms.h>
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// LibKDcraw includes

#include <libkdcraw/squeezedcombobox.h>

// Local includes

#include "iccprofileinfodlg.h"
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
        behaviourGB           = 0;
        defaultPathGB         = 0;
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

    QPushButton            *infoWorkProfiles;
    QPushButton            *infoMonitorProfiles;
    QPushButton            *infoInProfiles;
    QPushButton            *infoProofProfiles;

    QGroupBox              *behaviourGB;
    QGroupBox              *defaultPathGB;
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

    QGroupBox *colorPolicy = new QGroupBox(i18n("Color Management Policy"), panel);
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

    d->behaviourGB                 = new QGroupBox(i18n("Behavior"), colorPolicy);
    QVBoxLayout *vlay3             = new QVBoxLayout(d->behaviourGB);
    QButtonGroup *behaviourOptions = new QButtonGroup(d->behaviourGB);

    d->defaultApplyICC = new QRadioButton(d->behaviourGB);
    d->defaultApplyICC->setText(i18n("Apply when opening an image in the Image Editor"));
    d->defaultApplyICC->setWhatsThis( i18n("<p>If this option is enabled, digiKam applies the "
                     "Workspace default color profile to an image, without prompting you about missing "
                     "embedded profiles or embedded profiles different from the workspace "
                     "profile.</p>"));
    behaviourOptions->addButton(d->defaultApplyICC);

    d->defaultAskICC = new QRadioButton(d->behaviourGB);
    d->defaultAskICC->setText(i18n("Ask when opening an image in the Image Editor"));
    d->defaultAskICC->setWhatsThis( i18n("<p>If this option is enabled, digiKam asks the user "
                     "before it applies the Workspace default color profile to an image which has no "
                     "embedded profile or, if the image has an embedded profile, when it is not the same "
                     "as the workspace profile.</p>"));
    behaviourOptions->addButton(d->defaultAskICC);

    vlay3->addWidget(d->defaultApplyICC);
    vlay3->addWidget(d->defaultAskICC);
    vlay3->setMargin(KDialog::spacingHint());
    vlay3->setSpacing(0);

    grid->addWidget(d->enableColorManagement, 0, 0, 1, 1);
    grid->addWidget(lcmsLogoLabel,            0, 2, 1, 1);
    grid->addWidget(d->behaviourGB,           1, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);

    // --------------------------------------------------------

    d->defaultPathGB  = new QGroupBox(panel);
    QVBoxLayout *vlay = new QVBoxLayout(d->defaultPathGB);

    d->defaultPathGB->setTitle(i18n("Additional Color Profiles Directory"));

    d->defaultPathKU = new KUrlRequester(d->defaultPathGB);
    d->defaultPathKU->lineEdit()->setReadOnly(true);
    d->defaultPathKU->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);
    d->defaultPathKU->setWhatsThis( i18n("<p>DigiKam searches ICC profiles in default system folders "
                                         "and ships itself a few selected profiles. "
                                         "Store all your additional color profiles in the directory set here.</p>"));

    vlay->addWidget(d->defaultPathKU);
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(0);

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
    d->infoMonitorProfiles->setIcon(SmallIcon("document-properties"));
    d->infoMonitorProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected monitor profile.</p>"));

    QLabel *workIcon     = new QLabel(d->profilesGB);
    workIcon->setPixmap(SmallIcon("input-tablet"));
    QLabel *workProfiles = new QLabel(i18n("Workspace:"), d->profilesGB);
    d->workProfilesKC    = new SqueezedComboBox(d->profilesGB);
    workProfiles->setBuddy(d->workProfilesKC);
    d->workProfilesKC->setWhatsThis( i18n("<p>All the images will be converted to the color "
                     "space of this profile, so you must select a profile appropriate for editing.</p>"
                     "<p>These color profiles are device independent.</p>"));

    d->infoWorkProfiles = new QPushButton(d->profilesGB);
    d->infoWorkProfiles->setIcon(SmallIcon("document-properties"));
    d->infoWorkProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected workspace profile.</p>"));

    QLabel *inIcon     = new QLabel(d->profilesGB);
    inIcon->setPixmap(SmallIcon("camera-photo"));
    QLabel *inProfiles = new QLabel(i18n("Input:"), d->profilesGB);
    d->inProfilesKC    = new SqueezedComboBox(d->profilesGB);
    inProfiles->setBuddy(d->inProfilesKC);
    d->inProfilesKC->setWhatsThis( i18n("<p>You must select the profile for your input device "
                     "(usually, your camera, scanner...)</p>"));

    d->infoInProfiles = new QPushButton(d->profilesGB);
    d->infoInProfiles->setIcon(SmallIcon("document-properties"));
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
    d->infoProofProfiles->setIcon(SmallIcon("document-properties"));
    d->infoProofProfiles->setWhatsThis( i18n("<p>You can use this button to get more detailed "
                     "information about the selected soft proof profile.</p>"));

    grid2->addWidget(d->managedView,         0, 0, 1, 4);
    grid2->addWidget(d->monitorIcon,         1, 0, 1, 1);
    grid2->addWidget(d->monitorProfiles,     1, 1, 1, 1);
    grid2->addWidget(d->monitorProfilesKC,   1, 2, 1, 1);
    grid2->addWidget(d->infoMonitorProfiles, 1, 3, 1, 1);
    grid2->addWidget(workIcon,               2, 0, 1, 1);
    grid2->addWidget(workProfiles,           2, 1, 1, 1);
    grid2->addWidget(d->workProfilesKC,      2, 2, 1, 1);
    grid2->addWidget(d->infoWorkProfiles,    2, 3, 1, 1);
    grid2->addWidget(inIcon,                 3, 0, 1, 1);
    grid2->addWidget(inProfiles,             3, 1, 1, 1);
    grid2->addWidget(d->inProfilesKC,        3, 2, 1, 1);
    grid2->addWidget(d->infoInProfiles,      3, 3, 1, 1);
    grid2->addWidget(proofIcon,              4, 0, 1, 1);
    grid2->addWidget(proofProfiles,          4, 1, 1, 1);
    grid2->addWidget(d->proofProfilesKC,     4, 2, 1, 1);
    grid2->addWidget(d->infoProofProfiles,   4, 3, 1, 1);
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
    d->renderingIntentKC->insertItem(0, "Perceptual");
    d->renderingIntentKC->insertItem(1, "Relative Colorimetric");
    d->renderingIntentKC->insertItem(2, "Saturation");
    d->renderingIntentKC->insertItem(3, "Absolute Colorimetric");
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
    layout->addWidget(d->defaultPathGB);
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Color Management"));

    group.writeEntry("EnableCM", d->enableColorManagement->isChecked());

    if (!d->enableColorManagement->isChecked())
        return;          // No need to write settings in this case.

    if (d->defaultApplyICC->isChecked())
        group.writeEntry("BehaviourICC", true);
    else
        group.writeEntry("BehaviourICC", false);

    group.writeEntry("DefaultPath", d->defaultPathKU->url().path());
    group.writeEntry("BPCAlgorithm", d->bpcAlgorithm->isChecked());
    group.writeEntry("RenderingIntent", d->renderingIntentKC->currentIndex());
    group.writeEntry("ManagedView", d->managedView->isChecked());

    group.writePathEntry("InProfileFile", d->inICCPath.value(d->inProfilesKC->itemHighlighted()));
    group.writePathEntry("WorkProfileFile", d->workICCPath.value(d->workProfilesKC->itemHighlighted()));
    group.writePathEntry("MonitorProfileFile", d->monitorICCPath.value(d->monitorProfilesKC->itemHighlighted()));
    group.writePathEntry("ProofProfileFile", d->proofICCPath.value(d->proofProfilesKC->itemHighlighted()));
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

    if (!restore)
        d->enableColorManagement->setChecked(group.readEntry("EnableCM", false));

    d->bpcAlgorithm->setChecked(group.readEntry("BPCAlgorithm", false));
    d->renderingIntentKC->setCurrentIndex(group.readEntry("RenderingIntent", 0));
    d->managedView->setChecked(group.readEntry("ManagedView", false));

    if (group.readEntry("BehaviourICC", false))
        d->defaultApplyICC->setChecked(true);
    else
        d->defaultAskICC->setChecked(true);

    d->defaultPathKU->setUrl(group.readEntry("DefaultPath", QString()));
    fillCombos(false);

    setCurrentIndexFromUserData(d->inProfilesKC, group.readPathEntry("InProfileFile", QString()));
    setCurrentIndexFromUserData(d->workProfilesKC, group.readPathEntry("WorkProfileFile", IccProfile::sRGB().filePath()));
    setCurrentIndexFromUserData(d->monitorProfilesKC, group.readPathEntry("MonitorProfileFile", IccProfile::sRGB().filePath()));
    setCurrentIndexFromUserData(d->proofProfilesKC, group.readPathEntry("ProofProfileFile", QString()));
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
            case icSigColorSpaceClass:
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
    d->behaviourGB->setEnabled(enabled);
    d->defaultPathGB->setEnabled(enabled);
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
