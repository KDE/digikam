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
#include <qdir.h>
#include <qmap.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kurlrequester.h>
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

#include "iccprofileinfodlg.h"
#include "albumsettings.h"
#include "setupicc.h"

namespace Digikam
{

typedef QMap<QString, QString> ICCfilesPath;

class SetupICCPriv
{
public:

     SetupICCPriv()
     {
        enableColorManagement   = 0;
        bpcAlgorithm            = 0;
        defaultApplyICC         = 0;
        defaultAskICC           = 0;
        inICCFiles_file         = 0;
        workICCFiles_file       = 0;
        proofICCFiles_file      = 0;
        monitorICCFiles_file    = 0;
        defaultPathKU           = 0;
        inProfilesKC            = 0;
        workProfilesKC          = 0;
        proofProfilesKC         = 0;
        monitorProfilesKC       = 0;
        renderingIntentKC       = 0;
        infoWorkProfiles        = 0;
        infoMonitorProfiles     = 0;
        infoInProfiles          = 0;
        infoProofProfiles       = 0;
        ICCPath                 = QMap<QString, QString>();
     }

    QCheckBox       *enableColorManagement;
    QCheckBox       *bpcAlgorithm;
    QCheckBox       *managedView;
    
    QRadioButton    *defaultApplyICC;
    QRadioButton    *defaultAskICC;

    QStringList      inICCFiles_file;
    QStringList      workICCFiles_file;
    QStringList      proofICCFiles_file;
    QStringList      monitorICCFiles_file;

    QPushButton     *infoWorkProfiles;
    QPushButton     *infoMonitorProfiles;
    QPushButton     *infoInProfiles;
    QPushButton     *infoProofProfiles;
    
    KURLRequester   *defaultPathKU;

    KComboBox       *inProfilesKC;
    KComboBox       *workProfilesKC;
    KComboBox       *proofProfilesKC;
    KComboBox       *monitorProfilesKC;
    KComboBox       *renderingIntentKC;

    KDialogBase     *mainDialog;

    ICCfilesPath     ICCPath;
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

    QButtonGroup *behaviour = new QButtonGroup(2, Qt::Vertical, i18n("Behaviour"), colorPolicy);
    
    d->defaultApplyICC = new QRadioButton(behaviour);
    d->defaultApplyICC->setText(i18n("Apply when open an image in Image Editor"));
    QWhatsThis::add( d->defaultApplyICC, i18n("<p>If this option is selected, digiKam applies the "
                     "Workspace default color profile to an image without asking when this has not "
                     "embedded profile or the embedded profile is not the same that the workspace one.</p>"));
    
    d->defaultAskICC = new QRadioButton(behaviour);
    d->defaultAskICC->setText(i18n("Ask when open an image in Image Editor"));
    QWhatsThis::add( d->defaultAskICC, i18n("<p>If this option is selected, digiKam asks to the user "
                     "before it applies the Workspace default color profile to an image which has not "
                     "embedded profile or, if the image has an embbeded profile, this is not the same "
                     "that the workspace one.</p>"));
    
    grid->addMultiCellWidget(d->enableColorManagement, 0, 0, 0, 0);
    grid->addMultiCellWidget(lcmsLogoLabel, 0, 0, 2, 2);
    grid->addMultiCellWidget(behaviour, 1, 1, 0, 2);
    grid->setColStretch(1, 10);

    layout->addWidget(colorPolicy);
    
    // --------------------------------------------------------
    
    QHGroupBox * defaultPath = new QHGroupBox(parent);
    defaultPath->setTitle(i18n("Color Profiles Directory"));
    
    d->defaultPathKU = new KURLRequester(defaultPath);
    d->defaultPathKU->setMode(KFile::Directory);
    QWhatsThis::add( d->defaultPathKU, i18n("<p>Default path to the color profiles folder. "
                     "You must storage your color profiles in this directory.</p>"));
    
    layout->addWidget(defaultPath);

    // --------------------------------------------------------
    
    QGroupBox *profiles = new QGroupBox(0, Qt::Horizontal, i18n("ICC Settings"), parent);
    QGridLayout* grid2  = new QGridLayout( profiles->layout(), 3, 2, KDialog::spacingHint());
    grid2->setColStretch(1, 10);

    QLabel *workProfiles = new QLabel(i18n("Workspace profile: "), profiles);
    d->workProfilesKC    = new KComboBox(false, profiles);
    d->workProfilesKC->setMaximumWidth(300);
    workProfiles->setBuddy(d->workProfilesKC);
    QWhatsThis::add( d->workProfilesKC, i18n("<p>All the images will be converted to the color "
                     "space of this profile, so you must select an apropiate one for edition purpose.</p>"
                     "<p>These color profiles are device independents.</p>"));
    d->infoWorkProfiles = new QPushButton("Info", profiles);
    QWhatsThis::add( d->infoWorkProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected profile.</p>"));

    grid2->addMultiCellWidget(workProfiles, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->workProfilesKC, 0, 0, 1, 1);
    grid2->addMultiCellWidget(d->infoWorkProfiles, 0, 0, 2, 2);

    QLabel *monitorProfiles = new QLabel(i18n("Monitor profile: "), profiles);
    d->monitorProfilesKC    = new KComboBox(false, profiles);
    d->monitorProfilesKC->setMaximumWidth(300);
    monitorProfiles->setBuddy(d->monitorProfilesKC);
    QWhatsThis::add( d->monitorProfilesKC, i18n("<p>You must select the profile for your monitor.</p>"));
    d->infoMonitorProfiles = new QPushButton("Info", profiles);
    QWhatsThis::add( d->infoMonitorProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected profile.</p>"));

    grid2->addMultiCellWidget(monitorProfiles, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->monitorProfilesKC, 1, 1, 1, 1);
    grid2->addMultiCellWidget(d->infoMonitorProfiles, 1, 1, 2, 2);

    QLabel *inProfiles = new QLabel(i18n("Input profile: "), profiles);
    d->inProfilesKC    = new KComboBox(false, profiles);
    d->inProfilesKC->setMaximumWidth(300);
    inProfiles->setBuddy(d->inProfilesKC);
    QWhatsThis::add( d->inProfilesKC, i18n("<p>You must select the profile for your input device "
                     "(scanner, camera, ...)</p>"));
    d->infoInProfiles = new QPushButton("Info", profiles);
    QWhatsThis::add( d->infoInProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected profile.</p>"));

    grid2->addMultiCellWidget(inProfiles, 2, 2, 0, 0);
    grid2->addMultiCellWidget(d->inProfilesKC, 2, 2, 1, 1);
    grid2->addMultiCellWidget(d->infoInProfiles, 2, 2, 2, 2);

    QLabel *proofProfiles = new QLabel(i18n("Soft proof profile: "), profiles);
    d->proofProfilesKC    = new KComboBox(false, profiles);
    d->proofProfilesKC->setMaximumWidth(300);
    proofProfiles->setBuddy(d->proofProfilesKC);
    QWhatsThis::add( d->proofProfilesKC, i18n("<p>You must select the profile for your ouput device "
                     "(usually, your printer). This profile will be used to do a soft proof, so you will "
                     "be able to preview how an image will be rendered in an output device.</p>"));
    d->infoProofProfiles = new QPushButton("Info", profiles);
    QWhatsThis::add( d->infoProofProfiles, i18n("<p>You can use this button to get more detailled "
                     "information about the selected profile.</p>"));

    grid2->addMultiCellWidget(proofProfiles, 3, 3, 0, 0);
    grid2->addMultiCellWidget(d->proofProfilesKC, 3, 3, 1, 1);
    grid2->addMultiCellWidget(d->infoProofProfiles, 3, 3, 2, 2);

    fillCombos();

    layout->addWidget(profiles);

     // --------------------------------------------------------
    
    QVGroupBox * advancedSettingsBox = new QVGroupBox(i18n("Advanced Settings"), parent);

    d->managedView = new QCheckBox(advancedSettingsBox);
    d->managedView->setText(i18n("Use Colormanaged view"));
    QWhatsThis::add( d->managedView, i18n("<p>You have to use this option if " 
                     "you want to use your Monitor Color Profile to show your pictures in "
                     "Image Editor window.</p>"));

    d->bpcAlgorithm = new QCheckBox(advancedSettingsBox);
    d->bpcAlgorithm->setText(i18n("Use Black Point Compensation"));
    QWhatsThis::add( d->bpcAlgorithm, i18n("<p>BPC is a way to make adjustments between the maximum "
                     "black levels of digital files and the black capabilities of various "
                     "digital devices.</p>"));

    QHBox *hbox = new QHBox(advancedSettingsBox);
    QLabel *lablel = new QLabel(hbox);
    lablel->setText(i18n("Rendering Intents:"));

    d->renderingIntentKC = new KComboBox(false, hbox);
    d->renderingIntentKC->insertItem("Perceptual");
    d->renderingIntentKC->insertItem("Relative Colorimetric");
    d->renderingIntentKC->insertItem("Saturation");
    d->renderingIntentKC->insertItem("Absolute Colorimetric");
    QWhatsThis::add( d->renderingIntentKC, i18n("<ul><li>Perceptual intent causes the full gamut of the image to be compressed or expanded to fill the gamut of the destination device, so that gray balance is preserved but colorimetric accuracy may not be preserved.\n"
    "In other words, if certain colors in an image fall outside of the range of colors that the output device can render, the picture intent will cause all the colors in the image to be adjusted so that the every color in the image falls within the range that can be rendered and so that the relationship between colors is preserved as much as possible.\n"
    "This intent is most suitable for display of photographs and images, and is the default intent.</li>"
    "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged.\n"
    "This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, logo colors, ...).</li>"
    "<li>Relative Colorimetric intent is defined such that any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged. Proof intent does not preserve the white point.</li>"
    "<li>Saturarion intent preserves the saturation of colors in the image at the possible expense of hue and lightness.\n"
    "Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to achieve the desired effects.\n"
    "This intent is most suitable for business graphics such as charts, where it is more important that the colors be vivid and contrast well with each other rather than a specific color.</li></ul>"));

    layout->addWidget(advancedSettingsBox);

    // --------------------------------------------------------
    
    connect(lcmsLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processLCMSURL(const QString&)));

    connect(d->enableColorManagement, SIGNAL(toggled(bool)),
            this, SLOT(slotToggledWidgets(bool)));

    connect(d->proofProfilesKC, SIGNAL(highlighted(int)),
            this, SLOT(slotChangeProofProfile(int)));
    
    connect(d->infoProofProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedProof()) );

    connect(d->inProfilesKC, SIGNAL(highlighted(int)),
            this, SLOT(slotChangeInProfile(int)));
    
    connect(d->infoInProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedIn()) );

    connect(d->monitorProfilesKC, SIGNAL(highlighted(int)),
            this, SLOT(slotChangeMonitorProfile(int)));
    
    connect(d->infoMonitorProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedMonitor()) );

    connect(d->workProfilesKC, SIGNAL(highlighted(int)),
            this, SLOT(slotChangeWorkProfile(int)));
            
    connect(d->infoWorkProfiles, SIGNAL(clicked()),
            this, SLOT(slotClickedWork()));

    connect(d->defaultPathKU, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotFillCombos(const QString&)));
            
    connect(d->defaultPathKU, SIGNAL(urlSelected(const QString&)),
            this, SLOT(slotFillCombos(const QString&)));

    // --------------------------------------------------------

    layout->addStretch();

    slotToggledWidgets(false);
    
    readSettings();
    adjustSize();
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
    {
        config->writeEntry("BehaviourICC", true);
    }
    else
    {
        config->writeEntry("BehaviourICC", false);
    }

    config->writePathEntry("DefaultPath", d->defaultPathKU->url());
    config->writeEntry("WorkSpaceProfile", d->workProfilesKC->currentItem());
    config->writeEntry("MonitorProfile", d->monitorProfilesKC->currentItem());
    config->writeEntry("InProfile", d->inProfilesKC->currentItem());
    config->writeEntry("ProofProfile", d->proofProfilesKC->currentItem());
    config->writeEntry("BPCAlgorithm", d->bpcAlgorithm->isChecked());
    config->writeEntry("RenderingIntent", d->renderingIntentKC->currentItem());
    config->writeEntry("ManagedView", d->managedView->isChecked());

    if (d->inProfilesKC->count() != 0)
        config->writePathEntry("InProfileFile", d->inICCFiles_file[d->inProfilesKC->currentItem()]);
    
    if (d->workProfilesKC->count() != 0)
        config->writePathEntry("WorkProfileFile", d->workICCFiles_file[d->workProfilesKC->currentItem()]);
    
    if (d->monitorProfilesKC->count() != 0)
        config->writePathEntry("MonitorProfileFile", d->monitorICCFiles_file[d->monitorProfilesKC->currentItem()]);
    
    if (d->proofProfilesKC->count() != 0)
    {
        config->writePathEntry("ProofProfileFile", d->proofICCFiles_file[d->proofProfilesKC->currentItem()]);
        kdDebug() << "proof: " << d->proofProfilesKC->currentItem() << endl;
        kdDebug() << d->proofICCFiles_file[d->proofProfilesKC->currentItem()] << endl;
    }
}

void SetupICC::readSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");

    d->enableColorManagement->setChecked(config->readBoolEntry("EnableCM", false));
    
    if (!d->enableColorManagement->isChecked())
        return;          // No need to read settings in this case.
    
    slotToggledWidgets(true);

    if (config->readBoolEntry("BehaviourICC"))
    {
        d->defaultApplyICC->setChecked(true);
    }
    else
    {
        d->defaultAskICC->setChecked(true);
    }

    d->defaultPathKU->setURL(config->readPathEntry("DefaultPath"));
    d->workProfilesKC->setCurrentItem(config->readNumEntry("WorkSpaceProfile", 0));
    d->monitorProfilesKC->setCurrentItem(config->readNumEntry("MonitorProfile", 0));
    d->inProfilesKC->setCurrentItem(config->readNumEntry("InProfile", 0));
    d->proofProfilesKC->setCurrentItem(config->readNumEntry("ProofProfile", 0));
    d->bpcAlgorithm->setChecked(config->readBoolEntry("BPCAlgorithm", false));
    d->renderingIntentKC->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
    d->ICCPath["InProfile"]=config->readPathEntry("InProfileFile");
    d->ICCPath["WorkProfile"] = config->readPathEntry("WorkProfileFile");
    d->ICCPath["MonitorProfile"] = config->readPathEntry("MonitorProfileFile");
    d->ICCPath["ProofProfile"] = config->readPathEntry("ProofProfileFile");
    d->managedView->setChecked(config->readBoolEntry("ManagedView", false));
}

void SetupICC::fillCombos()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management");
    
    kdDebug()<< config->readPathEntry("DefaultPath") << endl;
    cmsHPROFILE tmpProfile=0;
    QDir profilesDir(QFile::encodeName(config->readPathEntry("DefaultPath")), "*.icc;*.icm", QDir::Files);

    const QFileInfoList* files = profilesDir.entryInfoList();
    QStringList m_monitorICCFiles_description=0, m_inICCFiles_description=0,
    m_proofICCFiles_description=0, m_workICCFiles_description=0;

    if (files)
    {
        QFileInfoListIterator it(*files);
        QFileInfo *fileInfo;

        while ((fileInfo = it.current()) != 0)
        {
            ++it;
            QString fileName = fileInfo->filePath();
            tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(fileName), "r");
            QString profileDescription = QString((cmsTakeProductDesc(tmpProfile)));
            kdDebug() << "Device class: " << cmsGetDeviceClass(tmpProfile) << endl;

            switch ((int)cmsGetDeviceClass(tmpProfile))
            {
                case icSigInputClass:
                    
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_inICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->inICCFiles_file.append(fileName);
                    break;
                    
                case icSigDisplayClass:
                    
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_monitorICCFiles_description.append(fileName);
                        m_workICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_monitorICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                        m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->monitorICCFiles_file.append(fileName);
                    d->workICCFiles_file.append(fileName);
                    break;
                
                case icSigOutputClass:
                
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_proofICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_proofICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->proofICCFiles_file.append(fileName);
                    break;
                
                case icSigColorSpaceClass:
                
                    if(QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_workICCFiles_description.append(fileName);
                        m_inICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                        m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->workICCFiles_file.append(fileName);
                    d->inICCFiles_file.append(fileName);
                    break;
            }

            cmsCloseProfile(tmpProfile);
        }
    }
    else
    {
        kdDebug() << "No List" << endl;
    }

    d->inProfilesKC->insertStringList(m_inICCFiles_description);
    d->monitorProfilesKC->insertStringList(m_monitorICCFiles_description);
    d->workProfilesKC->insertStringList(m_workICCFiles_description);
    d->proofProfilesKC->insertStringList(m_proofICCFiles_description);
}

void SetupICC::slotToggledWidgets(bool t)
{ 
    d->bpcAlgorithm->setEnabled(t);
    d->bpcAlgorithm->setChecked(t);

    d->managedView->setEnabled(t);
    d->managedView->setChecked(t);
 
    d->defaultApplyICC->setEnabled(t); 
    d->defaultAskICC->setEnabled(t);
    d->defaultAskICC->setChecked(t);
    d->defaultApplyICC->setChecked(t);
    
    d->defaultPathKU->setEnabled(t);
    
    d->inProfilesKC->setEnabled(t);
    d->workProfilesKC->setEnabled(t);
    d->proofProfilesKC->setEnabled(t);
    d->monitorProfilesKC->setEnabled(t);
    d->renderingIntentKC->setEnabled(t);

    d->infoWorkProfiles->setEnabled(t);
    d->infoMonitorProfiles->setEnabled(t);
    d->infoInProfiles->setEnabled(t);
    d->infoProofProfiles->setEnabled(t);

    if (t)
    {
        KConfig* config = kapp->config();
        config->setGroup("Color Management");

        if (config->readBoolEntry("BehaviourICC"))
        {
            d->defaultApplyICC->setChecked(true);
        }
        else
        {
            d->defaultAskICC->setChecked(false);
        }

        d->defaultPathKU->setURL(config->readPathEntry("DefaultPath"));
        d->workProfilesKC->setCurrentItem(config->readNumEntry("WorkSpaceProfile", 0));
        d->monitorProfilesKC->setCurrentItem(config->readNumEntry("MonitorProfile", 0));
        d->inProfilesKC->setCurrentItem(config->readNumEntry("InProfile", 0));
        d->proofProfilesKC->setCurrentItem(config->readNumEntry("ProofProfile", 0));
        d->bpcAlgorithm->setChecked(config->readBoolEntry("BPCAlgorithm", false));
        d->renderingIntentKC->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
        d->ICCPath["InProfile"]=config->readPathEntry("InProfileFile");
        d->ICCPath["WorkProfile"] = config->readPathEntry("WorkProfileFile");
        d->ICCPath["MonitorProfile"] = config->readPathEntry("MonitorProfileFile");
        d->ICCPath["ProofProfile"] = config->readPathEntry("ProofProfileFile");
        d->managedView->setChecked(config->readBoolEntry("ManagedView", false));
    }
}

void SetupICC::slotFillCombos(const QString& url)
{
    cmsHPROFILE tmpProfile=0;

    if (url.isEmpty())
    {
        d->mainDialog->enableButtonOK(false);
        KMessageBox::sorry(this, i18n("<p>You must set a default path to color profiles files.</p>"));
        return;
    }
    d->mainDialog->enableButtonOK(true);
    
    QDir profilesDir(QFile::encodeName(url), "*.icc;*.icm", QDir::Files);

    const QFileInfoList* files = profilesDir.entryInfoList();
    QStringList m_monitorICCFiles_description=0, m_inICCFiles_description=0,
    m_proofICCFiles_description=0, m_workICCFiles_description=0;

    if (files)
    {
        QFileInfoListIterator it(*files);
        QFileInfo *fileInfo;

        while ((fileInfo = it.current()) != 0)
        {
            ++it;
            QString fileName = fileInfo->filePath();
            tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(fileName), "r");
            QString profileDescription = QString((cmsTakeProductDesc(tmpProfile)));

            switch ((int)cmsGetDeviceClass(tmpProfile))
            {
                case icSigInputClass:
                    
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_inICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->inICCFiles_file.append(fileName);
                    break;
                
                case icSigDisplayClass:
                    
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_monitorICCFiles_description.append(fileName);
                        m_workICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_monitorICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                        m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->monitorICCFiles_file.append(fileName);
                    d->workICCFiles_file.append(fileName);
                    break;
                
                case icSigOutputClass:
                    if (QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_proofICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_proofICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    
                    d->proofICCFiles_file.append(fileName);
                    break;
                
                case icSigColorSpaceClass:
                    if(QString(cmsTakeProductDesc(tmpProfile)).isEmpty())
                    {
                        m_workICCFiles_description.append(fileName);
                        m_inICCFiles_description.append(fileName);
                    }
                    else
                    {
                        m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                        m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    }
                    d->workICCFiles_file.append(fileName);
                    d->inICCFiles_file.append(fileName);
                    break;
            }

            cmsCloseProfile(tmpProfile);
        }
    }
    else 
    {
        d->mainDialog->enableButtonOK(false);
        QString message = i18n("<p>Sorry, there is no profiles files in ");
        message.append(url);
        message.append(i18n("</p>"));
        KMessageBox::sorry(this,message);
        return;
    }

    d->inProfilesKC->clear();
    m_inICCFiles_description.remove(m_inICCFiles_description.begin());
    d->inProfilesKC->insertStringList(m_inICCFiles_description);
    d->monitorProfilesKC->clear();
    m_monitorICCFiles_description.remove(m_monitorICCFiles_description.begin());
    d->monitorProfilesKC->insertStringList(m_monitorICCFiles_description);
    d->workProfilesKC->clear();
    m_workICCFiles_description.remove(m_workICCFiles_description.begin());
    d->workProfilesKC->insertStringList(m_workICCFiles_description);
    d->proofProfilesKC->clear();
    m_proofICCFiles_description.remove(m_proofICCFiles_description.begin());
    d->proofProfilesKC->insertStringList(m_proofICCFiles_description);
    d->workICCFiles_file.remove(d->workICCFiles_file.begin());
    d->inICCFiles_file.remove(d->inICCFiles_file.begin());
    d->monitorICCFiles_file.remove(d->monitorICCFiles_file.begin());
    d->proofICCFiles_file.remove(d->proofICCFiles_file.begin());
    d->ICCPath["WorkProfile"] = d->workICCFiles_file[d->workProfilesKC->currentItem()];
    d->ICCPath["InProfile"] = d->inICCFiles_file[d->inProfilesKC->currentItem()];
    d->ICCPath["MonitorProfile"] = d->monitorICCFiles_file[d->monitorProfilesKC->currentItem()];
    d->ICCPath["ProofProfile"] = d->proofICCFiles_file[d->proofProfilesKC->currentItem()];
}

void SetupICC::slotClickedWork()
{
   profileInfo(d->ICCPath["WorkProfile"]);
}

void SetupICC::slotClickedIn()
{
    profileInfo(d->ICCPath["InProfile"]);
}

void SetupICC::slotClickedMonitor()
{
    profileInfo(d->ICCPath["MonitorProfile"]);
}

void SetupICC::slotClickedProof()
{
    profileInfo(d->ICCPath["ProofProfile"]);
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

void SetupICC::slotChangeWorkProfile(int index)
{
    d->ICCPath["WorkProfile"] = d->workICCFiles_file[index];
}

void SetupICC::slotChangeInProfile(int index)
{
    d->ICCPath["InProfile"] = d->inICCFiles_file[index];
}

void SetupICC::slotChangeMonitorProfile(int index)
{
    d->ICCPath["MonitorProfile"] = d->monitorICCFiles_file[index];
}

void SetupICC::slotChangeProofProfile(int index)
{
    d->ICCPath["ProofProfile"] = d->proofICCFiles_file[index];
}

}  // namespace Digikam

#include "setupicc.moc"
