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

namespace Digikam
{

SetupICC::SetupICC(QWidget* parent )
        : QWidget(parent)
{
//     QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint());

   // --------------------------------------------------------

   QVGroupBox *colorPolicy = new QVGroupBox(parent);
   colorPolicy->setTitle(i18n("Color Management Policy"));

   m_enableColorManagement = new QCheckBox(colorPolicy);
   m_enableColorManagement->setText(i18n("Enable Color Management"));
   QWhatsThis::add( m_enableColorManagement, i18n("<ul><li>Checked: Color Management is enabled</li>"
                                                  "<li>Unchecked: Color Management is disabled</li></ul>"));

   QButtonGroup *behaviour = new QButtonGroup(2, Qt::Vertical, i18n("Behaviour"),colorPolicy);

   m_defaultApplyICC = new QRadioButton(behaviour);
   m_defaultApplyICC->setText(i18n("Apply when open an image in Image Editor"));
   QWhatsThis::add( m_defaultApplyICC, i18n("<p>If this option is selected, Digikam applies the Workspace default color profile to an image without asking when this has not embedded profile or the embedded profile is not the same that the workspace one.</p>"));

   m_defaultAskICC = new QRadioButton(behaviour);
   m_defaultAskICC->setText(i18n("Ask when open an image in Image Editor"));
   QWhatsThis::add( m_defaultAskICC, i18n("<p>If this option is selected, Digikam asks to the user before it applies the Workspace default color profile to an image which has not embedded profile or, if the image has an embbeded profile, this is not the same that the workspace one.</p>"));

   layout->addWidget(colorPolicy);

   // --------------------------------------------------------
   
   QHGroupBox * defaultPath = new QHGroupBox(parent);
   defaultPath->setTitle(i18n("Color Profiles Directory"));

   m_defaultPath = new KURLRequester(defaultPath);
   m_defaultPath->setMode(KFile::Directory);
   QWhatsThis::add( m_defaultPath, i18n("<p>Default path to the color profiles folder.\nYou must storage your color profiles in this directory.</p>"));

   connect(m_defaultPath, SIGNAL(textChanged(const QString&)), this, SLOT(slotFillCombos(const QString&)));
   connect(m_defaultPath, SIGNAL(urlSelected(const QString&)), this, SLOT(slotFillCombos(const QString&)));

   layout->addWidget(defaultPath);

    // --------------------------------------------------------
    
    QVGroupBox *profiles = new QVGroupBox(parent);
    profiles->setTitle(i18n("ICC Settings"));

    QHBox *workProfilesSettings = new QHBox(profiles);
    workProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *workProfiles = new QLabel(i18n("Workspace profile: "), workProfilesSettings);
    m_workProfiles = new KComboBox(false, workProfilesSettings);
    workProfiles->setBuddy(m_workProfiles);
    QWhatsThis::add( m_workProfiles, i18n("<p>All the images will be converted to the color space of this profile, so you must select an apropiate one for edition purpose.</p><p> These color profiles are device independents.</p>"));
    QPushButton *infoWorkProfiles = new QPushButton("Info", workProfilesSettings);
    infoWorkProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoWorkProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(m_workProfiles, SIGNAL(highlighted(int)), this, SLOT(slotChangeWorkProfile(int)));
    connect(infoWorkProfiles, SIGNAL(clicked()), this, SLOT(slotClickedWork()));

    QHBox *monitorProfilesSettings = new QHBox(profiles);
    monitorProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *monitorProfiles = new QLabel(i18n("Monitor profile: "), monitorProfilesSettings);
    m_monitorProfiles = new KComboBox(false, monitorProfilesSettings);
    monitorProfiles->setBuddy(m_monitorProfiles);
    QWhatsThis::add( m_monitorProfiles, i18n("<p>You must select the profile for your monitor.</p>"));
    QPushButton *infoMonitorProfiles = new QPushButton("Info", monitorProfilesSettings);
    infoMonitorProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoMonitorProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(m_monitorProfiles, SIGNAL(highlighted(int)), this, SLOT(slotChangeMonitorProfile(int)));
    connect(infoMonitorProfiles, SIGNAL(clicked()), this, SLOT(slotClickedMonitor()) );

    QHBox *inProfilesSettings = new QHBox(profiles);
    inProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *inProfiles = new QLabel(i18n("Input profile: "), inProfilesSettings);
    m_inProfiles = new KComboBox(false, inProfilesSettings);
    inProfiles->setBuddy(m_inProfiles);
    QWhatsThis::add( m_inProfiles, i18n("<p>You must select the profile for your input device (scanner, camera, ...)</p>"));
    QPushButton *infoInProfiles = new QPushButton("Info", inProfilesSettings);
    infoInProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoInProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(m_inProfiles, SIGNAL(highlighted(int)), this, SLOT(slotChangeInProfile(int)));
    connect(infoInProfiles, SIGNAL(clicked()), this, SLOT(slotClickedIn()) );

    QHBox *proofProfilesSettings = new QHBox(profiles);
    proofProfilesSettings->setSpacing(KDialog::spacingHint());

    QLabel *proofProfiles = new QLabel(i18n("Soft proof profile: "), proofProfilesSettings);
    m_proofProfiles = new KComboBox(false, proofProfilesSettings);
    proofProfiles->setBuddy(m_proofProfiles);
    QWhatsThis::add( m_proofProfiles, i18n("<p>You must select the profile for your ouput device (usually, your printer). This profile will be used to do a soft proof, so you will be able to preview how an image will be rendered in an output device.</p>"));
    QPushButton *infoProofProfiles = new QPushButton("Info", proofProfilesSettings);
    infoProofProfiles->setMaximumWidth(60);
    QWhatsThis::add( infoProofProfiles, i18n("<p>You can use this button to get more detailled information about the selected profile.</p>"));
    connect(m_proofProfiles, SIGNAL(highlighted(int)), this, SLOT(slotChangeProofProfile(int)));
    connect(infoProofProfiles, SIGNAL(clicked()), this, SLOT(slotClickedProof()) );

    fillCombos();

    layout->addWidget(profiles);

    // --------------------------------------------------------
    
    QVGroupBox * bpc = new QVGroupBox(i18n("BPC Algorithm"), parent);

    m_bpcAlgorithm = new QCheckBox(bpc);
    m_bpcAlgorithm->setText(i18n("Use Black Point Compensation"));
    QWhatsThis::add( m_bpcAlgorithm, i18n("<p>BPC is a way to make adjustments between the maximum black levels of digital files and the black capabilities of various digital devices.</p>"));
    
    layout->addWidget(bpc);
    
    // --------------------------------------------------------
    
    QHGroupBox *intents = new QHGroupBox(i18n("Rendering Intents"), parent);

    m_renderingIntent = new KComboBox(false, intents);
    m_renderingIntent->insertItem("Perceptual");
    m_renderingIntent->insertItem("Absolute Colorimetric");
    m_renderingIntent->insertItem("Relative Colorimetric");
    m_renderingIntent->insertItem("Saturation");
    QWhatsThis::add( m_renderingIntent, i18n("<ul><li>Perceptual intent causes the full gamut of the image to be compressed or expanded to fill the gamut of the destination device, so that gray balance is preserved but colorimetric accuracy may not be preserved.\n"
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
    connect(m_enableColorManagement, SIGNAL(toggled(bool)), this, SLOT(slotToggledWidgets(bool)));

    layout->addStretch();

    slotToggledWidgets(false);
    
    readSettings();
    adjustSize();

//    mainLayout->addWidget(this);
}

SetupICC::~SetupICC()
{
}

void SetupICC::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");
    config->writeEntry("EnableCM", m_enableColorManagement->isChecked());
    
    if (!m_enableColorManagement->isChecked())
        return;          // No need to write settings in this case.

    if (m_defaultApplyICC->isChecked())
    {
        config->writeEntry("BehaviourICC", true);
    }
    else
    {
        config->writeEntry("BehaviourICC", false);
    }
//     config->writeEntry("ApplyICC", m_defaultApplyICC->isChecked());
//     config->writeEntry("AskICC", m_defaultAskICC->isChecked());
    if (m_defaultPath->url().isEmpty())
    {
        QString message = QString(i18n("<p>You must set a default path to color profiles files.</p>"));
        message.append(i18n("<p>This settings will not be written.</p>"));
        KMessageBox::error(this, message );
        config->writeEntry("EnableCM", false);
        config->sync();
        return;
    }
    config->writeEntry("DefaultPath", m_defaultPath->url());
    config->writeEntry("WorkSpaceProfile", m_workProfiles->currentItem());
    config->writeEntry("MonitorProfile", m_monitorProfiles->currentItem());
    config->writeEntry("InProfile", m_inProfiles->currentItem());
    config->writeEntry("ProofProfile", m_proofProfiles->currentItem());
    config->writeEntry("BPCAlgorithm", m_bpcAlgorithm->isChecked());
    config->writeEntry("RenderingIntent", m_renderingIntent->currentItem());
    config->writeEntry("InProfileFile", m_inICCFiles_file[m_inProfiles->currentItem()]);
    config->writeEntry("WorkProfileFile", m_workICCFiles_file[m_workProfiles->currentItem()]);
    config->writeEntry("MonitorProfileFile", m_monitorICCFiles_file[m_monitorProfiles->currentItem()]);
    config->writeEntry("ProofProfileFile", m_proofICCFiles_file[m_proofProfiles->currentItem()]);

    config->sync();
}

void SetupICC::readSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");

    m_enableColorManagement->setChecked(config->readBoolEntry("EnableCM", false));
    
    if (!m_enableColorManagement->isChecked())
        return;          // No need to read settings in this case.
    
    slotToggledWidgets(true);

    if (config->readBoolEntry("BehaviourICC"))
    {
        m_defaultApplyICC->setChecked(true);
    }
    else
    {
        m_defaultAskICC->setChecked(true);
    }
//     m_defaultApplyICC->setChecked(config->readBoolEntry("ApplyICC", false));
//     m_defaultAskICC->setChecked(config->readBoolEntry("AskICC", false));
    m_defaultPath->setURL(config->readPathEntry("DefaultPath"));
    m_workProfiles->setCurrentItem(config->readNumEntry("WorkSpaceProfile", 0));
    m_monitorProfiles->setCurrentItem(config->readNumEntry("MonitorProfile", 0));
    m_inProfiles->setCurrentItem(config->readNumEntry("InProfile", 0));
    m_proofProfiles->setCurrentItem(config->readNumEntry("ProofProfile", 0));
    m_bpcAlgorithm->setChecked(config->readBoolEntry("BPCAlgorithm", false));
    m_renderingIntent->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
    m_ICCfilesPath["InProfile"]=config->readPathEntry("InProfileFile");
    m_ICCfilesPath["WorkProfile"] = config->readPathEntry("WorkProfileFile");
    m_ICCfilesPath["MonitorProfile"] = config->readPathEntry("MonitorProfileFile");
    m_ICCfilesPath["ProofProfile"] = config->readPathEntry("ProofProfileFile");
    kdDebug() << "Profile Path: " << m_ICCfilesPath["InProfile"] << endl;
    kdDebug() << "Profile Path: " << m_ICCfilesPath["WorkProfile"] << endl;
    kdDebug() << "Profile Path: " << m_ICCfilesPath["MonitorProfile"] << endl;
    kdDebug() << "Profile Path: " << m_ICCfilesPath["ProofProfile"] << endl;
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
                    m_inICCFiles_file.append(fileName);
                    break;
                case icSigDisplayClass:
                    m_monitorICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_monitorICCFiles_file.append(fileName);
                    m_workICCFiles_file.append(fileName);
                    break;
                case icSigOutputClass:
                    m_proofICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_proofICCFiles_file.append(fileName);
                    break;
                case icSigColorSpaceClass:
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_workICCFiles_file.append(fileName);
                    m_inICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_inICCFiles_file.append(fileName);
                    break;
            }

            cmsCloseProfile(tmpProfile);
        }
    }
    else
    {
      kdDebug() << "no list" << endl;
    }
    m_inProfiles->insertStringList(m_inICCFiles_description);
    m_monitorProfiles->insertStringList(m_monitorICCFiles_description);
    m_workProfiles->insertStringList(m_workICCFiles_description);
    m_proofProfiles->insertStringList(m_proofICCFiles_description);
}

void SetupICC::slotToggledWidgets(bool t)
{ 
    m_bpcAlgorithm->setEnabled(t); 
 
    m_defaultApplyICC->setEnabled(t); 
    m_defaultAskICC->setEnabled(t);
    m_defaultApplyICC->setChecked(t);
    
    m_defaultPath->setEnabled(t); 
    
    m_inProfiles->setEnabled(t); 
    m_workProfiles->setEnabled(t); 
    m_proofProfiles->setEnabled(t); 
    m_monitorProfiles->setEnabled(t); 
    m_renderingIntent->setEnabled(t);

    if (t)
    {
        KConfig* config = kapp->config();

        config->setGroup("Color Management");
        if (config->readBoolEntry("BehaviourICC"))
        {
            m_defaultApplyICC->setChecked(true);
        }
        else
        {
            m_defaultAskICC->setChecked(false);
        }
    //     m_defaultApplyICC->setChecked(config->readBoolEntry("ApplyICC", false));
    //     m_defaultAskICC->setChecked(config->readBoolEntry("AskICC", false));
        m_defaultPath->setURL(config->readPathEntry("DefaultPath"));
        m_workProfiles->setCurrentItem(config->readNumEntry("WorkSpaceProfile", 0));
        m_monitorProfiles->setCurrentItem(config->readNumEntry("MonitorProfile", 0));
        m_inProfiles->setCurrentItem(config->readNumEntry("InProfile", 0));
        m_proofProfiles->setCurrentItem(config->readNumEntry("ProofProfile", 0));
        m_bpcAlgorithm->setChecked(config->readBoolEntry("BPCAlgorithm", false));
        m_renderingIntent->setCurrentItem(config->readNumEntry("RenderingIntent", 0));
        m_ICCfilesPath["InProfile"]=config->readPathEntry("InProfileFile");
        m_ICCfilesPath["WorkProfile"] = config->readPathEntry("WorkProfileFile");
        m_ICCfilesPath["MonitorProfile"] = config->readPathEntry("MonitorProfileFile");
        m_ICCfilesPath["ProofProfile"] = config->readPathEntry("ProofProfileFile");
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
        m_inICCFiles_file.clear();
        m_monitorICCFiles_description.clear();
        m_monitorICCFiles_file.clear();
        m_workICCFiles_description.clear();
        m_workICCFiles_file.clear();
        m_proofICCFiles_description.clear();
        m_proofICCFiles_file.clear();

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
                    m_inICCFiles_file.append(fileName);
                    break;
                case icSigDisplayClass:
                    m_monitorICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_monitorICCFiles_file.append(fileName);
                    m_workICCFiles_file.append(fileName);
                    break;
                case icSigOutputClass:
                    m_proofICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_proofICCFiles_file.append(fileName);
                    break;
                case icSigColorSpaceClass:
                    m_workICCFiles_description.append(QString(cmsTakeProductDesc(tmpProfile)));
                    m_workICCFiles_file.append(fileName);
                    break;
            }

            cmsCloseProfile(tmpProfile);
        }
    }
    else
    {
      kdDebug() << "no list" << endl;
    }
    m_inProfiles->clear();
    m_inProfiles->insertStringList(m_inICCFiles_description);
    m_monitorProfiles->clear();
    m_monitorProfiles->insertStringList(m_monitorICCFiles_description);
    m_workProfiles->clear();
    m_workProfiles->insertStringList(m_workICCFiles_description);
    m_proofProfiles->clear();
    m_proofProfiles->insertStringList(m_proofICCFiles_description);
    m_ICCfilesPath["WorkProfile"] = m_workICCFiles_file[m_workProfiles->currentItem()];
    m_ICCfilesPath["InProfile"] = m_inICCFiles_file[m_inProfiles->currentItem()];
    m_ICCfilesPath["MonitorProfile"] = m_monitorICCFiles_file[m_monitorProfiles->currentItem()];
    m_ICCfilesPath["ProofProfile"] = m_proofICCFiles_file[m_proofProfiles->currentItem()];
//     kdDebug() << "Current Profle: " << m_ICCfilesPath["WorkProfile"] << endl;
}

void SetupICC::slotClickedWork()
{
   profileInfo(m_ICCfilesPath["WorkProfile"]);
}

void SetupICC::slotClickedIn()
{
    profileInfo(m_ICCfilesPath["InProfile"]);
}

void SetupICC::slotClickedMonitor()
{
    profileInfo(m_ICCfilesPath["MonitorProfile"]);
}

void SetupICC::slotClickedProof()
{
    profileInfo(m_ICCfilesPath["ProofProfile"]);
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
    m_ICCfilesPath["WorkProfile"] = m_workICCFiles_file[index];
}

void SetupICC::slotChangeInProfile(int index)
{
    m_ICCfilesPath["InProfile"] = m_inICCFiles_file[index];
}

void SetupICC::slotChangeMonitorProfile(int index)
{
    m_ICCfilesPath["MonitorProfile"] = m_monitorICCFiles_file[index];
}

void SetupICC::slotChangeProofProfile(int index)
{
    m_ICCfilesPath["ProofProfile"] = m_proofICCFiles_file[index];
}

}  // namespace Digikam

#include "setupicc.moc"
