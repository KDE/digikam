/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : dialog displayed at the first digiKam run
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ Includes.

#include <iostream>

// Qt includes.

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QDir>
#include <QFileInfo>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

// Local includes.

#include "ddebug.h"
#include "version.h"
#include "firstrun.h"
#include "digikamfirstrun.h"
#include "digikamfirstrun.moc"

namespace Digikam
{

using namespace std;

DigikamFirstRun::DigikamFirstRun(QWidget* parent)
               : KDialog(parent)
{
    setModal(true);
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setCaption(i18n( "Album Library Path" ));
    setHelp("firstrundialog.anchor", "digikam");
    
    m_ui = new FirstRunWidget(this);
    setMainWidget(m_ui);

    m_ui->m_path->setUrl(QDir::homePath() + 
                         i18nc("This is a path name so you should "
                               "include the slash in the translation", "/Pictures"));
    m_ui->m_path->setMode(KFile::Directory | KFile::LocalOnly);

    m_ui->m_pixLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_ui->setMinimumSize(450, m_ui->sizeHint().height());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));
}

DigikamFirstRun::~DigikamFirstRun()
{
}

void DigikamFirstRun::slotOk()
{
    QString albumLibraryFolder = m_ui->m_path->url().path();
    
    if (albumLibraryFolder.isEmpty())
    {
        KMessageBox::sorry(this, i18n("You must select a folder for digiKam to "
                                      "use as the Album Library folder."));
        return;
    }

    if (!albumLibraryFolder.startsWith("/"))
    {
        albumLibraryFolder.prepend(QDir::homePath());
    }

    if (KUrl(albumLibraryFolder).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutFragment))
    {
        KMessageBox::sorry(this, i18n("digiKam cannot use your home folder as "
                                      "the Album Library folder."));
        return;
    }

    QDir targetPath(albumLibraryFolder);
    
    if (!targetPath.exists())
    {
        int rc = KMessageBox::questionYesNo(this,
                                   i18n("<qt>The folder you selected does not exist: "

                                        "<p><b>%1</b></p>"
                                        "Would you like digiKam to create it?</qt>"
                                        ,albumLibraryFolder),
                                   i18n("Create Folder?"));

        if (rc == KMessageBox::No)
        {
            return;
        }

        if (!targetPath.mkdir(albumLibraryFolder))
        {
            KMessageBox::sorry(this,
                               i18n("<qt>digiKam could not create the folder shown below. "
                                    "Please select a different location."
                                    "<p><b>%1</b></p></qt>").arg(albumLibraryFolder),
                               i18n("Create Folder Failed"));
            return;
        }
    }

    QFileInfo path(albumLibraryFolder);
    
    if (!path.isWritable()) 
    {
        KMessageBox::information(this, i18n("No write access for this path.\n"
                                            "Warning: the comment and tag features "
                                            "will not work."));
        return;
    }

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("General Settings");
    group.writeEntry("Version", digikam_version);

    group = config->group("Album Settings");
    group.writeEntry("Album Path", albumLibraryFolder);

    config->sync();

    KDialog::accept();

    QString ErrorMsg, URL;

    if (KToolInvocation::startServiceByDesktopName("digikam", URL , &ErrorMsg) > 0)
    {
        DError() << ErrorMsg << endl;
        KMessageBox::sorry(this, i18n("Cannot restart digiKam automatically.\n"
                                      "Please restart digiKam manually."));
    }
}

}  // namespace Digikam
