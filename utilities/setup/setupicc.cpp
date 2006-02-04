/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
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

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes.

#include "albumsettings.h"
#include "setupicc.h"

// Others

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

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
        ICCPath                 = QMap<QString, QString>();
     }

    QCheckBox       *enableColorManagement;
    QCheckBox       *bpcAlgorithm;
    
    QRadioButton    *defaultApplyICC;
    QRadioButton    *defaultAskICC;

    QStringList      inICCFiles_file;
    QStringList      workICCFiles_file;
    QStringList      proofICCFiles_file;
    QStringList      monitorICCFiles_file;

    KURLRequester   *defaultPathKU;

    KComboBox       *inProfilesKC;
    KComboBox       *workProfilesKC;
    KComboBox       *proofProfilesKC;
    KComboBox       *monitorProfilesKC;
    KComboBox       *renderingIntentKC;

    ICCfilesPath     ICCPath;
};

SetupICC::SetupICC(QWidget* parent )
        : QWidget(parent)
{
//     QVBoxLayout *mainLayout = new QVBoxLayout(parent);
    d = new SetupICCPriv();

    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint());

   // --------------------------------------------------------

   QVGroupBox *colorPolicy = new QVGroupBox(parent);
   colorPolicy->setTitle(i18n("Color Management Policy"));

   d->enableColorManagement = new QCheckBox(colorPolicy);
   d->enableColorManagement->setText(i18n("Enable Color Management"));
   QWhatsThis::add( d->enableColorManagement, i18n("<ul><li>Checked: Color Management is enabled</li>"
                                                  "<li>Unchecked: Color Management is disabled</li></ul>"));

   QButtonGroup *behaviour = new QButtonGroup(2, Qt::Vertical, i18n("Behaviour"),colorPolicy);

   d->defaultApplyICC = new QRadioButton(behaviour);
   d->defaultApplyICC->setText(i18n("Apply when open an image in Image Editor"));
   QWhatsThis::add( d->defaultApplyICC, i18n("<p>If this option is selected, Digikam applies the Workspace default color profile to an image without asking when this has not embedded profile or the embedded profile is not the same that the workspace one.</p>"));

   d->defaultAskICC = new QRadioButton(behaviour);
   d->defaultAskICC->setText(i18n("Ask when open an image in Image Editor"));
   QWhatsThis::add( d->defaultAskICC, i18n("<p>If this option is selected, Digikam asks to the user before it applies the Workspace default color profile to an image which has not embedded profile or, if the image has an embbeded profile, this is not the same that the workspace one.</p>"));

   layout->addWidget(colorPolicy);

   // --------------------------------------------------------
   
   QHGroupBox * defaultPath = new QHGroupBox(parent);
   defaultPath->setTitle(i18n("Color Profiles Directory"));

   d->defaultPathKU = new KURLRequester(defaultPath);
   d->defaultPathKU->setMode(KFile::Directory);
   QWhatsThis::add( d->defaultPathKU, i18n("<p>Default path to the color profiles folder.\nYou must storage your color profiles in this directory.</p>"));

   connect(d->defaultPathKU, SIGNAL(textChanged(const QString&)), this, SLOT(slotFillCombos(const QString&)));
   connect(d->defaultPathKU, SIGNAL(urlSelected(const QString&)), this, SLOT(slotFillCombos(const QString&)));

   layout->addWidget(defaultPath);

    // --------------------------------------------------------
    
    QVGroupBox *profiles = new QVGroupBox(parent);
    profiles->setTitle(i18n("ICC Settings"));

    QHBox *workProfilesSettings = new QHBox(profiles);
    workProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *workProfiles = new QLabel(i18n("Workspace profile: "), workProfilesSettings);
    d->workProfilesKC = new KComboBox(false, workProfilesSettings);
    workProfiles->setBuddy(d->workProfilesKC);
    QWhatsThis::add( d->workProfilesKC, i18n("<p>All the images will be converted to the color space of this profile, so you must select an apropiate one for edition purpose.</p><p> These color profiles are device independents.</p>"));
    QPushButton *infoWorkProfiles = new QPushButton("Info", workProfilesSettings);
    infoWorkProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoWorkProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(d->workProfilesKC, SIGNAL(highlighted(int)), this, SLOT(slotChangeWorkProfile(int)));
    connect(infoWorkProfiles, SIGNAL(clicked()), this, SLOT(slotClickedWork()));

    QHBox *monitorProfilesSettings = new QHBox(profiles);
    monitorProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *monitorProfiles = new QLabel(i18n("Monitor profile: "), monitorProfilesSettings);
    d->monitorProfilesKC = new KComboBox(false, monitorProfilesSettings);
    monitorProfiles->setBuddy(d->monitorProfilesKC);
    QWhatsThis::add( d->monitorProfilesKC, i18n("<p>You must select the profile for your monitor.</p>"));
    QPushButton *infoMonitorProfiles = new QPushButton("Info", monitorProfilesSettings);
    infoMonitorProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoMonitorProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(d->monitorProfilesKC, SIGNAL(highlighted(int)), this, SLOT(slotChangeMonitorProfile(int)));
    connect(infoMonitorProfiles, SIGNAL(clicked()), this, SLOT(slotClickedMonitor()) );

    QHBox *inProfilesSettings = new QHBox(profiles);
    inProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *inProfiles = new QLabel(i18n("Input profile: "), inProfilesSettings);
    d->inProfilesKC = new KComboBox(false, inProfilesSettings);
    inProfiles->setBuddy(d->inProfilesKC);
    QWhatsThis::add( d->inProfilesKC, i18n("<p>You must select the profile for your input device (scanner, camera, ...)</p>"));
    QPushButton *infoInProfiles = new QPushButton("Info", inProfilesSettings);
    infoInProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoInProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(d->inProfilesKC, SIGNAL(highlighted(int)), this, SLOT(slotChangeInProfile(int)));
    connect(infoInProfiles, SIGNAL(clicked()), this, SLOT(slotClickedIn()) );

    QHBox *proofProfilesSettings = new QHBox(profiles);
    proofProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *proofProfiles = new QLabel(i18n("Soft proof profile: "), proofProfilesSettings);
    d->proofProfilesKC = new KComboBox(false, proofProfilesSettings);
    proofProfiles->setBuddy(d->proofProfilesKC);
    QWhatsThis::add( d->proofProfilesKC, i18n("<p>You must select the profile for your ouput device (usually, your printer). This profile will be used to do a soft proof, so you will be able to preview how an image will be rendered in an output device.</p>"));
    QPushButton *infoProofProfiles = new QPushButton("Info", proofProfilesSettings);
    infoProofProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoProofProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(d->proofProfilesKC, SIGNAL(highlighted(int)), this, SLOT(slotChangeProofProfile(int)));
    connect(infoProofProfiles, SIGNAL(clicked()), this, SLOT(slotClickedProof()) );

    fillCombos();

    layout->addWidget(profiles);

    // --------------------------------------------------------
    
    QVGroupBox * bpc = new QVGroupBox(i18n("BPC Algorithm"), parent);

    d->bpcAlgorithm = new QCheckBox(bpc);
    d->bpcAlgorithm->setText(i18n("Use Black Point Compensation"));
    QWhatsThis::add( d->bpcAlgorithm, i18n("<p>BPC is a way to make adjustments between the maximum black levels of digital files and the black capabilities of various digital devices.</p>"));
    
    layout->addWidget(bpc);
    
    // --------------------------------------------------------
    
    QHGroupBox *intents = new QHGroupBox(i18n("Rendering Intents"), parent);

    d->renderingIntentKC = new KComboBox(false, intents);
    d->renderingIntentKC->insertItem("Perceptual");
    d->renderingIntentKC->insertItem("Absolute Colorimetric");
    d->renderingIntentKC->insertItem("Relative Colorimetric");
    d->renderingIntentKC->insertItem("Saturation");
    QWhatsThis::add( d->renderingIntentKC, i18n("<ul><li>Perceptual intent causes the full gamut of the image to be compressed or expanded to fill the gamut of the destination device, so that gray balance is preserved but colorimetric accuracy may not be preserved.\n"
    "In other words, if certain colors in an image fall outside of the range of colors that the output device can render, the picture intent will cause all the colors in the image to be adjusted so that the every color in the image falls within the range that can be rendered and so that the relationship between colors is preserved as much as possible.\n"
    "This intent is most suitable for display of photographs and images, and is the default intent.</li>"
    "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged.\n"
    "This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, logo colors, ...).</li>"
    "<li>Relative Colorimetric intent is defined such that any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged. Proof intent does not preserve the white point.</li>"
    "<li>Saturarion intent preserves the saturation of colors in the image at the possible expense of hue and lightness.\n"
    "Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to achieve the desired effects.\n"
    "This intent is most suitable for business graphics such as charts, where it is more important that the colors be vivid and contrast well with each other rather than a specific color.</li></ul>"));

    layout->addWidget(intents);
    
    // --------------------------------------------------------
    connect(d->enableColorManagement, SIGNAL(toggled(bool)), this, SLOT(slotToggledWidgets(bool)));

    layout->addStretch();

    slotToggledWidgets(false);
    
    readSettings();
    adjustSize();

//    mainLayout->addWidget(this);
}

SetupICC::~SetupICC()
{
    delete d;
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
//     config->writeEntry("ApplyICC", d->defaultApplyICC->isChecked());
//     config->writeEntry("AskICC", d->defaultAskICC->isChecked());
    if (d->defaultPathKU->url().isEmpty())
    {
        QString message = QString(i18n("<p>You must set a default path to color profiles files.</p>"));
        message.append(i18n("<p>This settings will not be written.</p>"));
        KMessageBox::error(this, message );
        config->writeEntry("EnableCM", false);
        config->sync();
        return;
    }
    config->writePathEntry("DefaultPath", d->defaultPathKU->url());
    config->writeEntry("WorkSpaceProfile", d->workProfilesKC->currentItem());
    config->writeEntry("MonitorProfile", d->monitorProfilesKC->currentItem());
    config->writeEntry("InProfile", d->inProfilesKC->currentItem());
    config->writeEntry("ProofProfile", d->proofProfilesKC->currentItem());
    config->writeEntry("BPCAlgorithm", d->bpcAlgorithm->isChecked());
    config->writeEntry("RenderingIntent", d->renderingIntentKC->currentItem());
    config->writePathEntry("InProfileFile", d->inICCFiles_file[d->inProfilesKC->currentItem()]);
    config->writePathEntry("WorkProfileFile", d->workICCFiles_file[d->workProfilesKC->currentItem()]);
    config->writePathEntry("MonitorProfileFile", d->monitorICCFiles_file[d->monitorProfilesKC->currentItem()]);
    config->writePathEntry("ProofProfileFile", d->proofICCFiles_file[d->proofProfilesKC->currentItem()]);

    config->sync();
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
//     d->defaultApplyICC->setChecked(config->readBoolEntry("ApplyICC", false));
//     d->defaultAskICC->setChecked(config->readBoolEntry("AskICC", false));
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
    kdDebug() << "Profile Path: " << d->ICCPath["InProfile"] << endl;
    kdDebug() << "Profile Path: " << d->ICCPath["WorkProfile"] << endl;
    kdDebug() << "Profile Path: " << d->ICCPath["MonitorProfile"] << endl;
    kdDebug() << "Profile Path: " << d->ICCPath["ProofProfile"] << endl;
}

void SetupICC::fillCombos()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");
    
    kdDebug()<< config->readPathEntry("DefaultPath") << endl;
    cmsHPROFILE tmpProfile=0;
    QDir profilesDir(QFile::encodeName(config->readPathEntry("DefaultPath")), "*.icc;*.icm", QDir::Files);


    if (!profilesDir.isReadable())
    {
//         KMessageBox::error(this, i18n("You don't have read permission for this directory"), i18n("Permission error"));
        return;
    }

    const QFileInfoList* files = profilesDir.entryInfoList();
    QStringList m_monitorICCFiles_description=0, m_inICCFiles_description=0, m_proofICCFiles_description=0, m_workICCFiles_description=0;

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
                    m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->inICCFiles_file.append(fileName);
                    break;
                case icSigDisplayClass:
                    m_monitorICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->monitorICCFiles_file.append(fileName);
                    d->workICCFiles_file.append(fileName);
                    break;
                case icSigOutputClass:
                    m_proofICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->proofICCFiles_file.append(fileName);
                    break;
                case icSigColorSpaceClass:
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->workICCFiles_file.append(fileName);
                    m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->inICCFiles_file.append(fileName);
                    break;
            }

            cmsCloseProfile(tmpProfile);
        }
    }
    else
    {
      kdDebug() << "no list" << endl;
    }
    d->inProfilesKC->insertStringList(m_inICCFiles_description);
    d->monitorProfilesKC->insertStringList(m_monitorICCFiles_description);
    d->workProfilesKC->insertStringList(m_workICCFiles_description);
    d->proofProfilesKC->insertStringList(m_proofICCFiles_description);
}

void SetupICC::slotToggledWidgets(bool t)
{ 
    d->bpcAlgorithm->setEnabled(t);
 
    d->defaultApplyICC->setEnabled(t); 
    d->defaultAskICC->setEnabled(t);
    d->defaultApplyICC->setChecked(t);
    
    d->defaultPathKU->setEnabled(t);
    
    d->inProfilesKC->setEnabled(t);
    d->workProfilesKC->setEnabled(t);
    d->proofProfilesKC->setEnabled(t);
    d->monitorProfilesKC->setEnabled(t);
    d->renderingIntentKC->setEnabled(t);

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
    //     d->defaultApplyICC->setChecked(config->readBoolEntry("ApplyICC", false));
    //     d->defaultAskICC->setChecked(config->readBoolEntry("AskICC", false));
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
    }
}

void SetupICC::slotFillCombos(const QString& url)
{
    cmsHPROFILE tmpProfile=0;
    QDir profilesDir(QFile::encodeName(url), "*.icc;*.icm", QDir::Files);


    if (!profilesDir.isReadable())
    {
//         KMessageBox::error(this, i18n("You don't have read permission for this directory"), i18n("Permission error"));
        return;
    }

    const QFileInfoList* files = profilesDir.entryInfoList();
    QStringList m_monitorICCFiles_description=0, m_inICCFiles_description=0, m_proofICCFiles_description=0, m_workICCFiles_description=0;

    if (files)
    {
        QFileInfoListIterator it(*files);
        QFileInfo *fileInfo;

        m_inICCFiles_description.clear();
        d->inICCFiles_file.clear();
        m_monitorICCFiles_description.clear();
        d->monitorICCFiles_file.clear();
        m_workICCFiles_description.clear();
        d->workICCFiles_file.clear();
        m_proofICCFiles_description.clear();
        d->proofICCFiles_file.clear();

        while ((fileInfo = it.current()) != 0)
        {
            ++it;
            QString fileName = fileInfo->filePath();
            tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(fileName), "r");
            QString profileDescription = QString((cmsTakeProductDesc(tmpProfile)));
//             kdDebug() << "Device class: " << cmsGetDeviceClass(tmpProfile) << endl;

            switch ((int)cmsGetDeviceClass(tmpProfile))
            {
                case icSigInputClass:
                    m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->inICCFiles_file.append(fileName);
                    break;
                case icSigDisplayClass:
                    m_monitorICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->monitorICCFiles_file.append(fileName);
                    d->workICCFiles_file.append(fileName);
                    break;
                case icSigOutputClass:
                    m_proofICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->proofICCFiles_file.append(fileName);
                    break;
                case icSigColorSpaceClass:
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    d->workICCFiles_file.append(fileName);
                    break;
            }

            cmsCloseProfile(tmpProfile);
        }
    }
    else
    {
      kdDebug() << "no list" << endl;
    }
    d->inProfilesKC->clear();
    d->inProfilesKC->insertStringList(m_inICCFiles_description);
    d->monitorProfilesKC->clear();
    d->monitorProfilesKC->insertStringList(m_monitorICCFiles_description);
    d->workProfilesKC->clear();
    d->workProfilesKC->insertStringList(m_workICCFiles_description);
    d->proofProfilesKC->clear();
    d->proofProfilesKC->insertStringList(m_proofICCFiles_description);
    d->ICCPath["WorkProfile"] = d->workICCFiles_file[d->workProfilesKC->currentItem()];
    d->ICCPath["InProfile"] = d->inICCFiles_file[d->inProfilesKC->currentItem()];
    d->ICCPath["MonitorProfile"] = d->monitorICCFiles_file[d->monitorProfilesKC->currentItem()];
    d->ICCPath["ProofProfile"] = d->proofICCFiles_file[d->proofProfilesKC->currentItem()];
//     kdDebug() << "Current Profle: " << d->ICCPath["WorkProfile"] << endl;
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
