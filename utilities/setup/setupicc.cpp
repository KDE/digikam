/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 *         F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-11-24
 * Description : ICC profils setup tab.
 * 
 * Copyright 2005-2006 by Gilles Caulier and F.J. Cruz
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

// QT includes.

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>

// lcms includes.

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// Local includes.

#include "squeezedcombobox.h"
#include "iccprofileinfodlg.h"
#include "albumsettings.h"
#include "setupicc.h"

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
        cmToolInRawLoading    = 0;
        behaviourGB           = 0;
        defaultPathGB         = 0;
        profilesGB            = 0;
        advancedSettingsGB    = 0;
     }

    QCheckBox              *enableColorManagement;
    QCheckBox              *bpcAlgorithm;
    QCheckBox              *managedView;
    QCheckBox              *cmToolInRawLoading;
    
    QRadioButton           *defaultApplyICC;
    QRadioButton           *defaultAskICC;

    QPushButton            *infoWorkProfiles;
    QPushButton            *infoMonitorProfiles;
    QPushButton            *infoInProfiles;
    QPushButton            *infoProofProfiles;

    QVGroupBox             *behaviourGB;
    QHGroupBox             *defaultPathGB;
    QGroupBox              *profilesGB;
    QVGroupBox             *advancedSettingsGB;

    // Maps to store profile descriptions and profile file path
    QMap<QString, QString>  inICCPath;
    QMap<QString, QString>  workICCPath;
    QMap<QString, QString>  proofICCPath;
    QMap<QString, QString>  monitorICCPath;
    
    KURLRequester          *defaultPathKU;

    KComboBox              *renderingIntentKC;

    KDialogBase            *mainDialog;

    SqueezedComboBox       *inProfilesKC;
    SqueezedComboBox       *workProfilesKC;
    SqueezedComboBox       *proofProfilesKC;
    SqueezedComboBox       *monitorProfilesKC;
};

SetupICC::SetupICC(QWidget* parent, KDialogBase* dialog )
        : QWidget(parent)
{
    d = new SetupICCPriv();
    d->mainDialog = dialog;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint());

    // --------------------------------------------------------
    
    QGroupBox *colorPolicy = new QGroupBox(0, Qt::Horizontal, i18n("Color Management Policy"), parent);
    QGridLayout* grid = new QGridLayout( colorPolicy->layout(), 1, 2, KDialog::spacingHint());
    
    d->enableColorManagement = new QCheckBox(colorPolicy);
    d->enableColorManagement->setText(i18n("Enable Color Management"));
    QWhatsThis::add( d->enableColorManagement, i18n("<ul><li>Checked: Color Management is enabled</li>"
                                                    "<li>Unchecked: Color Management is disabled</li></ul>"));
    
    KURLLabel *lcmsLogoLabel = new KURLLabel(colorPolicy);
    lcmsLogoLabel->setText(QString::null);
    lcmsLogoLabel->setURL("http://www.littlecms.com");
    KGlobal::dirs()->addResourceType("lcmslogo", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("lcmslogo", "lcmslogo.png");
    lcmsLogoLabel->setPixmap( QPixmap( directory + "lcmslogo.png" ) );
    QToolTip::add(lcmsLogoLabel, i18n("Visit Little CMS project website"));

    d->behaviourGB = new QVGroupBox(i18n("Behaviour"), colorPolicy);
    QButtonGroup *behaviourOptions = new QButtonGroup(2, Qt::Vertical, d->behaviourGB);
    behaviourOptions->setFrameStyle( QFrame::NoFrame );
    behaviourOptions->setInsideMargin(0); 

    d->defaultApplyICC = new QRadioButton(behaviourOptions);
    d->defaultApplyICC->setText(i18n("Apply when open an image in Image Editor"));
    QWhatsThis::add( d->defaultApplyICC, i18n("<p>If this option is selected, digiKam applies the "
                     "Workspace default color profile to an image without asking when this has not "
                     "embedded profile or the embedded profile is not the same that the workspace one.</p>"));
    
    d->defaultAskICC = new QRadioButton(behaviourOptions);
    d->defaultAskICC->setText(i18n("Ask when open an image in Image Editor"));
    QWhatsThis::add( d->defaultAskICC, i18n("<p>If this option is selected, digiKam asks to the user "
                     "before it applies the Workspace default color profile to an image which has not "
                     "embedded profile or, if the image has an embbeded profile, this is not the same "
                     "that the workspace one.</p>"));

    QHBox *hbox   = new QHBox(d->behaviourGB);
    QLabel *space = new QLabel(hbox);
    space->setFixedWidth(15);
    d->cmToolInRawLoading = new QCheckBox(hbox);
    d->cmToolInRawLoading->setText(i18n("Launch Color Management plugin with RAW files"));
    QWhatsThis::add( d->cmToolInRawLoading, i18n("Enable this option if you want to lauch the color "
                     "management image plugin when a RAW file is loaded in editor."));

    grid->addMultiCellWidget(d->enableColorManagement, 0, 0, 0, 0);
    grid->addMultiCellWidget(lcmsLogoLabel, 0, 0, 2, 2);
    grid->addMultiCellWidget(d->behaviourGB, 1, 1, 0, 2);
    grid->setColStretch(1, 10);

    layout->addWidget(colorPolicy);
    
    // --------------------------------------------------------
    
    d->defaultPathGB = new QHGroupBox(parent);
    d->defaultPathGB->setTitle(i18n("Color Profiles Directory"));
    
    d->defaultPathKU = new KURLRequester(d->defaultPathGB);
    d->defaultPathKU->lineEdit()->setReadOnly(true);
    d->defaultPathKU->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);    
    QWhatsThis::add( d->defaultPathKU, i18n("<p>Default path to the color profiles folder. "
                     "You must store all your color profiles in this directory.</p>"));
    
    layout->addWidget(d->defaultPathGB);

    // --------------------------------------------------------
    
    d->profilesGB = new QGroupBox(0, Qt::Horizontal, i18n("ICC Profiles Settings"), parent);
    QGridLayout* grid2  = new QGridLayout( d->profilesGB->layout(), 3, 3, KDialog::spacingHint());
    grid2->setColStretch(2, 10);

    QLabel *workIcon     = new QLabel(d->profilesGB);
    workIcon->setPixmap(SmallIcon("tablet"));
    QLabel *workProfiles = new QLabel(i18n("Workspace:"), d->profilesGB);
    d->workProfilesKC    = new SqueezedComboBox(d->profilesGB);
    workProfiles->setBuddy(d->workProfilesKC);
    QWhatsThis::add( d->workProfilesKC, i18n("<p>All the images will be converted to the color "
                     "space of this profile, so you must select an apropiate one for edition purpose.</p>"
                     "<p>These color profiles are device independents.</p>"));
    d->infoWorkProfiles = new QPushButton(i18n("Info..."), d->profilesGB);
    QWhatsThis::add( d->infoWorkProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected workspace profile.</p>"));

    grid2->addMultiCellWidget(workIcon, 0, 0, 0, 0);
    grid2->addMultiCellWidget(workProfiles, 0, 0, 1, 1);
    grid2->addMultiCellWidget(d->workProfilesKC, 0, 0, 2, 2);
    grid2->addMultiCellWidget(d->infoWorkProfiles, 0, 0, 3, 3);

    QLabel *monitorIcon     = new QLabel(d->profilesGB);
    monitorIcon->setPixmap(SmallIcon("tv"));
    QLabel *monitorProfiles = new QLabel(i18n("Monitor:"), d->profilesGB);
    d->monitorProfilesKC    = new SqueezedComboBox(d->profilesGB);
    monitorProfiles->setBuddy(d->monitorProfilesKC);
    QWhatsThis::add( d->monitorProfilesKC, i18n("<p>You must select the profile for your monitor. "
                     "You need to toogle on <b>Use color managed view</b> option from "
                     "the Advanced Settings pannel to use this profile.</p>"));
    d->infoMonitorProfiles = new QPushButton(i18n("Info..."), d->profilesGB);
    QWhatsThis::add( d->infoMonitorProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected monitor profile.</p>"));
    
    grid2->addMultiCellWidget(monitorIcon, 1, 1, 0, 0);
    grid2->addMultiCellWidget(monitorProfiles, 1, 1, 1, 1);
    grid2->addMultiCellWidget(d->monitorProfilesKC, 1, 1, 2, 2);
    grid2->addMultiCellWidget(d->infoMonitorProfiles, 1, 1, 3, 3);

    QLabel *inIcon     = new QLabel(d->profilesGB);
    inIcon->setPixmap(SmallIcon("camera"));
    QLabel *inProfiles = new QLabel(i18n("Input:"), d->profilesGB);
    d->inProfilesKC    = new SqueezedComboBox(d->profilesGB);
    inProfiles->setBuddy(d->inProfilesKC);
    QWhatsThis::add( d->inProfilesKC, i18n("<p>You must select the profile for your input device "
                     "(usually, your camera, scanner...)</p>"));
    d->infoInProfiles = new QPushButton(i18n("Info..."), d->profilesGB);
    QWhatsThis::add( d->infoInProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected input profile.</p>"));
    
    grid2->addMultiCellWidget(inIcon, 2, 2, 0, 0);
    grid2->addMultiCellWidget(inProfiles, 2, 2, 1, 1);
    grid2->addMultiCellWidget(d->inProfilesKC, 2, 2, 2, 2);
    grid2->addMultiCellWidget(d->infoInProfiles, 2, 2, 3, 3);

    QLabel *proofIcon     = new QLabel(d->profilesGB);
    proofIcon->setPixmap(SmallIcon("printer1"));
    QLabel *proofProfiles = new QLabel(i18n("Soft proof:"), d->profilesGB);
    d->proofProfilesKC    = new SqueezedComboBox(d->profilesGB);
    proofProfiles->setBuddy(d->proofProfilesKC);
    QWhatsThis::add( d->proofProfilesKC, i18n("<p>You must select the profile for your ouput device "
                     "(usually, your printer). This profile will be used to do a soft proof, so you will "
                     "be able to preview how an image will be rendered in an output device.</p>"));
    d->infoProofProfiles = new QPushButton(i18n("Info..."), d->profilesGB);
    QWhatsThis::add( d->infoProofProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected soft proof profile.</p>"));
    
    grid2->addMultiCellWidget(proofIcon, 3, 3, 0, 0);
    grid2->addMultiCellWidget(proofProfiles, 3, 3, 1, 1);
    grid2->addMultiCellWidget(d->proofProfilesKC, 3, 3, 2, 2);
    grid2->addMultiCellWidget(d->infoProofProfiles, 3, 3, 3, 3);

    layout->addWidget(d->profilesGB);

     // --------------------------------------------------------
    
    d->advancedSettingsGB = new QVGroupBox(i18n("Advanced Settings"), parent);

    d->managedView = new QCheckBox(d->advancedSettingsGB);
    d->managedView->setText(i18n("Use color managed view (warning: slow)"));
    QWhatsThis::add( d->managedView, i18n("<p>You have to use this option if " 
                     "you want to use your <b>Monitor Color Profile</b> to show your pictures in "
                     "Image Editor window. Warning : this option can take a while to render "
                     "pictures on the screen, especially with slow computer.</p>"));

    d->bpcAlgorithm = new QCheckBox(d->advancedSettingsGB);
    d->bpcAlgorithm->setText(i18n("Use black point compensation"));
    QWhatsThis::add( d->bpcAlgorithm, i18n("<p><b>Black Point Compensation</b> is a way to make "
                     "adjustments between the maximum "
                     "black levels of digital files and the black capabilities of various "
                     "digital devices.</p>"));

    QHBox *hbox2 = new QHBox(d->advancedSettingsGB);
    QLabel *lablel = new QLabel(hbox2);
    lablel->setText(i18n("Rendering Intents:"));

    d->renderingIntentKC = new KComboBox(false, hbox2);
    d->renderingIntentKC->insertItem("Perceptual");
    d->renderingIntentKC->insertItem("Relative Colorimetric");
    d->renderingIntentKC->insertItem("Saturation");
    d->renderingIntentKC->insertItem("Absolute Colorimetric");
    QWhatsThis::add( d->renderingIntentKC, i18n("<ul><li><p><b>Perceptual intent</b> causes the full gamut of the image to be "
                     "compressed or expanded to fill the gamut of the destination device, so that gray balance is "
                     "preserved but colorimetric accuracy may not be preserved.</p>"
                     "<p>In other words, if certain colors in an image fall outside of the range of colors that the output "
                     "device can render, the picture intent will cause all the colors in the image to be adjusted so that "
                     "the every color in the image falls within the range that can be rendered and so that the relationship "
                     "between colors is preserved as much as possible.</p>"
                     "<p>This intent is most suitable for display of photographs and images, and is the default intent.</p></li>"
                     "<li><p><b>Absolute Colorimetric intent</b> causes any colors that fall outside the range that the output device "
                     "can render are adjusted to the closest color that can be rendered, while all other colors are "
                     "left unchanged.</p>"
                     "<p>This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, "
                     "logo colors, ...).</p></li>"
                     "<li><p><b>Relative Colorimetric intent</b> is defined such that any colors that fall outside the range that the "
                     "output device can render are adjusted to the closest color that can be rendered, while all other colors "
                     "are left unchanged. Proof intent does not preserve the white point.</p></li>"
                     "<li><p><b>Saturation intent</b> preserves the saturation of colors in the image at the possible expense of "
                     "hue and lightness.</p>"
                     "<p>Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to "
                     "achieve the desired effects.</p>"
                     "<p>This intent is most suitable for business graphics such as charts, where it is more important that the "
                     "colors be vivid and contrast well with each other rather than a specific color.</p></li></ul>"));

    layout->addWidget(d->advancedSettingsGB);
    layout->addStretch();
    
    // --------------------------------------------------------
    
    connect(d->managedView, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleManagedView(bool)));
                    
    connect(lcmsLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processLCMSURL(const QString&)));

    connect(d->enableColorManagement, SIGNAL(toggled(bool)),
            this, SLOT(slotToggledWidgets(bool)));

    connect(d->infoProofProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedProof()) );

    connect(d->infoInProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedIn()) );

    connect(d->infoMonitorProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedMonitor()) );

    connect(d->infoWorkProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedWork()));
            
    connect(d->defaultPathKU, SIGNAL(urlSelected(const QString&)),
            this, SLOT(slotFillCombos(const QString&)));

    connect(d->defaultAskICC, SIGNAL(toggled(bool)),
            d->cmToolInRawLoading, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    adjustSize();
    readSettings();
}

SetupICC::~SetupICC()
{
    delete d;
}

void SetupICC::processLCMSURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void SetupICC::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management");

    config->writeEntry("EnableCM", d->enableColorManagement->isChecked());
    
    if (!d->enableColorManagement->isChecked())
        return;          // No need to write settings in this case.

    if (d->defaultApplyICC->isChecked())
        config->writeEntry("BehaviourICC", true);
    else
        config->writeEntry("BehaviourICC", false);

    config->writeEntry("CMInRawLoading", d->cmToolInRawLoading->isChecked());
    config->writePathEntry("DefaultPath", d->defaultPathKU->url());
    config->writeEntry("WorkSpaceProfile", d->workProfilesKC->currentItem());
    config->writeEntry("MonitorProfile", d->monitorProfilesKC->currentItem());
    config->writeEntry("InProfile", d->inProfilesKC->currentItem());
    config->writeEntry("ProofProfile", d->proofProfilesKC->currentItem());
    config->writeEntry("BPCAlgorithm", d->bpcAlgorithm->isChecked());
    config->writeEntry("RenderingIntent", d->renderingIntentKC->currentItem());
    config->writeEntry("ManagedView", d->managedView->isChecked());

    config->writePathEntry("InProfileFile", 
            *(d->inICCPath.find(d->inProfilesKC->currentText())));
    config->writePathEntry("WorkProfileFile", 
            *(d->workICCPath.find(d->workProfilesKC->currentText())));
    config->writePathEntry("MonitorProfileFile",
            *(d->monitorICCPath.find(d->monitorProfilesKC->currentText())));
    config->writePathEntry("ProofProfileFile", 
            *(d->proofICCPath.find(d->proofProfilesKC->currentText())));
}

void SetupICC::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management");

    d->defaultPathKU->setURL(config->readPathEntry("DefaultPath", QString::null));
    d->enableColorManagement->setChecked(config->readBoolEntry("EnableCM", false));
    d->bpcAlgorithm->setChecked(config->readBoolEntry("BPCAlgorithm", false));
    d->renderingIntentKC->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
    d->managedView->setChecked(config->readBoolEntry("ManagedView", false));
    
    if (config->readBoolEntry("BehaviourICC"))
        d->defaultApplyICC->setChecked(true);
    else
        d->defaultAskICC->setChecked(true);

    d->cmToolInRawLoading->setChecked(config->readBoolEntry("CMInRawLoading", false));

    slotToggledWidgets(d->enableColorManagement->isChecked());
    slotToggleManagedView(d->managedView->isChecked());
    fillCombos(d->defaultPathKU->url(), false);

    d->workProfilesKC->setCurrentItem(config->readNumEntry("WorkSpaceProfile", 0));
    d->monitorProfilesKC->setCurrentItem(config->readNumEntry("MonitorProfile", 0));
    d->inProfilesKC->setCurrentItem(config->readNumEntry("InProfile", 0));
    d->proofProfilesKC->setCurrentItem(config->readNumEntry("ProofProfile", 0));
}

void SetupICC::restoreSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management");

    d->defaultPathKU->setURL(config->readPathEntry("DefaultPath", QString::null));
    d->bpcAlgorithm->setChecked(config->readBoolEntry("BPCAlgorithm", false));
    d->renderingIntentKC->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
    d->managedView->setChecked(config->readBoolEntry("ManagedView", false));

    if (config->readBoolEntry("BehaviourICC"))
        d->defaultApplyICC->setChecked(true);
    else
        d->defaultAskICC->setChecked(true);
 
    d->cmToolInRawLoading->setChecked(config->readBoolEntry("CMInRawLoading", false));

    fillCombos(d->defaultPathKU->url(), false);

    d->workProfilesKC->setCurrentItem(config->readNumEntry("WorkSpaceProfile", 0));
    d->monitorProfilesKC->setCurrentItem(config->readNumEntry("MonitorProfile", 0));
    d->inProfilesKC->setCurrentItem(config->readNumEntry("InProfile", 0));
    d->proofProfilesKC->setCurrentItem(config->readNumEntry("ProofProfile", 0));
}

void SetupICC::slotFillCombos(const QString& path)
{
    fillCombos(path, true);
}

void SetupICC::fillCombos(const QString& path, bool report)
{
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
    QDir dir(path);

    if (path.isEmpty() || !dir.exists() || !dir.isReadable())
    {
        if (report)
            KMessageBox::sorry(this, i18n("<p>You must set a correct default "
                                          "path for your ICC color profiles files.</p>"));
    
        d->mainDialog->enableButtonOK(false);
        return;
    }    
    d->mainDialog->enableButtonOK(true);

    // Look the ICC profile path repository set by user.
    QDir userProfilesDir(QFile::encodeName(path), "*.icc;*.icm", QDir::Files);
    const QFileInfoList* usersFiles = userProfilesDir.entryInfoList();
    kdDebug() << "Scanning ICC profiles from user repository: " << path << endl;

    if ( !parseProfilesfromDir(usersFiles) )
    {
        if (report)
        {
            QString message = i18n("<p>Sorry, there is no ICC profiles files in ");
            message.append(path);
            message.append(i18n("</p>"));
            KMessageBox::sorry(this, message);
        }
        
        kdDebug() << "No ICC profile files found!!!" << endl;
        d->mainDialog->enableButtonOK(false);
        return;
    }

    // Look the ICC color-space profile path include with digiKam dist.
    KGlobal::dirs()->addResourceType("profiles", KGlobal::dirs()->kde_default("data") + "digikam/profiles");
    QString digiKamProfilesPath = KGlobal::dirs()->findResourceDir("profiles", "srgb.icm");
    QDir digiKamProfilesDir(QFile::encodeName(digiKamProfilesPath), "*.icc;*.icm", QDir::Files);
    const QFileInfoList* digiKamFiles = digiKamProfilesDir.entryInfoList();
    kdDebug() << "Scanning ICC profiles include with digiKam: " << digiKamProfilesPath << endl;
    parseProfilesfromDir(digiKamFiles);

    d->inProfilesKC->insertStringList(d->inICCPath.keys(), 0);
    
    d->monitorProfilesKC->insertStringList(d->monitorICCPath.keys(), 0);
    if (d->monitorICCPath.keys().isEmpty())
    {
        d->managedView->setEnabled(false);
        d->managedView->setChecked(false);
    }
    else
    {
        d->managedView->setEnabled(true);
    }
    
    d->workProfilesKC->insertStringList(d->workICCPath.keys(), 0);
    
    d->proofProfilesKC->insertStringList(d->proofICCPath.keys(), 0);
}

bool SetupICC::parseProfilesfromDir(const QFileInfoList* files)
{
    cmsHPROFILE tmpProfile=0;
    bool findIccFiles=false;

    if (files)
    {
        QFileInfoListIterator it(*files);
        QFileInfo *fileInfo;

        while ((fileInfo = it.current()) != 0)
        {
            QString fileName = fileInfo->filePath();
            tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(fileName), "r");
            QString profileDescription = QString((cmsTakeProductDesc(tmpProfile)));

            switch ((int)cmsGetDeviceClass(tmpProfile))
            {
                case icSigInputClass:
                    
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                        d->inICCPath.insert(fileName, fileName);
                    else
                        d->inICCPath.insert(QString(cmsTakeProductDesc(tmpProfile)), fileName);
                    
                    kdDebug() << "ICC file: " << fileName << " ==> Input device class (" 
                              << cmsGetDeviceClass(tmpProfile) << ")" << endl;
                    findIccFiles = true;
                    break;
                
                case icSigDisplayClass:
                    
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        d->monitorICCPath.insert(fileName, fileName);
                        d->workICCPath.insert(fileName, fileName);
                    }
                    else
                    {
                        d->monitorICCPath.insert(QString(cmsTakeProductDesc(tmpProfile)), fileName);
                        d->workICCPath.insert(QString(cmsTakeProductDesc(tmpProfile)), fileName);
                    }

                    kdDebug() << "ICC file: " << fileName << " ==> Monitor device class (" 
                              << cmsGetDeviceClass(tmpProfile) << ")" << endl;
                    findIccFiles = true;
                    break;
                
                case icSigOutputClass:
                
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                        d->proofICCPath.insert(fileName, fileName);
                    else
                        d->proofICCPath.insert(QString(cmsTakeProductDesc(tmpProfile)), fileName);

                    kdDebug() << "ICC file: " << fileName << " ==> Output device class (" 
                              << cmsGetDeviceClass(tmpProfile) << ")" << endl;
                    findIccFiles = true;
                    break;
                
                case icSigColorSpaceClass:
                
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        d->inICCPath.insert(fileName, fileName);
                        d->workICCPath.insert(fileName, fileName);
                    }
                    else
                    {
                        d->inICCPath.insert(QString(cmsTakeProductDesc(tmpProfile)), fileName);
                        d->workICCPath.insert(QString(cmsTakeProductDesc(tmpProfile)), fileName);
                    }

                    kdDebug() << "ICC file: " << fileName << " ==> WorkingSpace device class (" 
                              << cmsGetDeviceClass(tmpProfile) << ")" << endl;
                    findIccFiles = true;
                    break;
            
                default:
                    kdDebug() << "ICC file: " << fileName << " ==> UNKNOW device class (" 
                              << cmsGetDeviceClass(tmpProfile) << ")" << endl;
                    break;
            }

            cmsCloseProfile(tmpProfile);
            ++it;
        }
    }
    
    return findIccFiles;
}

void SetupICC::slotToggledWidgets(bool t)
{ 
    d->behaviourGB->setEnabled(t); 
    d->defaultPathGB->setEnabled(t); 
    d->profilesGB->setEnabled(t);
    d->advancedSettingsGB->setEnabled(t);

    if (t)
        restoreSettings();
    else
        d->mainDialog->enableButtonOK(true);
}

void SetupICC::slotClickedWork()
{
    profileInfo(*(d->workICCPath.find(d->workProfilesKC->currentText())));
}

void SetupICC::slotClickedIn()
{
    profileInfo(*(d->inICCPath.find(d->inProfilesKC->currentText())));
}

void SetupICC::slotClickedMonitor()
{
    profileInfo(*(d->monitorICCPath.find(d->monitorProfilesKC->currentText())));
}

void SetupICC::slotClickedProof()
{
    profileInfo(*(d->proofICCPath.find(d->proofProfilesKC->currentText())));
}

void SetupICC::profileInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(this, i18n("Sorry, there is not any selected profile"), i18n("Profile Error"));
        return;
    }

    ICCProfileInfoDlg infoDlg(this, profile);
    infoDlg.exec();
}

void SetupICC::slotToggleManagedView(bool b)
{
    d->monitorProfilesKC->setEnabled(b);
    d->infoMonitorProfiles->setEnabled(b);
}
            
}  // namespace Digikam

#include "setupicc.moc"
