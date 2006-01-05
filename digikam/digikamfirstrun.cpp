/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-02-01
 * Description :
 *
 * Copyright 2003-2004 by Renchi Raju
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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>
#include <qdir.h>
#include <qfileinfo.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes.

#include "digikamfirstrun.h"
#include "version.h"
#include "firstrunUI.h"

namespace Digikam
{

using namespace std;

DigikamFirstRun::DigikamFirstRun( KConfig* config,
                                  QWidget* parent,
                                  const char* name, bool modal,
                                  WFlags fl )
    : KDialogBase( parent, name, modal, i18n( "Album Library Path" ), Help|Ok|Cancel, Ok, true )

{
    setHelp("firstrundialog.anchor", "digikam");
    setWFlags(fl);
    config_ = config;
    ui = new DigikamFirstFirstRunWidget(this);
    setMainWidget(ui);
    ui->path->setURL(QDir::homeDirPath() + i18n("This is a path name so you should include the slash in the translation","/Pictures"));
    ui->path->setMode(KFile::Directory | KFile::LocalOnly);

    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    ui->pixLabel->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));
    ui->setMinimumSize(450, ui->sizeHint().height());
}

DigikamFirstRun::~DigikamFirstRun()
{
}

void DigikamFirstRun::slotOk()
{
    QString albumLibraryFolder = ui->path->url();
    if (albumLibraryFolder.isEmpty())
    {
        KMessageBox::sorry(this, i18n("You must select a folder for Digikam to use as the Album Library folder."));
        return;
    }

    if (!albumLibraryFolder.startsWith("/"))
    {
        albumLibraryFolder.prepend(QDir::homeDirPath());
    }

    if (KURL(albumLibraryFolder).equals(KURL(QDir::homeDirPath()), true))
    {
        KMessageBox::sorry(this, i18n("Digikam cannot use your home folder as the Album Library folder."));
        return;
    }

    QDir targetPath(albumLibraryFolder);
    if (!targetPath.exists())
    {
        int rc = KMessageBox::questionYesNo(this,
                                   i18n("<qt>The folder you selected does not exist: "

                                        "<p><b>%1</b></p>"
                                        "Would you like Digikam to make it for you now?</qt>")
                                        .arg(albumLibraryFolder),
                                   i18n("Create Folder?"));

        if (rc == KMessageBox::No)
        {
            return;
        }

        if (!targetPath.mkdir(albumLibraryFolder))
        {
            KMessageBox::sorry(this,
                               i18n("<qt>Digikam could not create the folder shown below. "
                                    "Please select a different location."
                                    "<p><b>%1</b></p></qt>").arg(albumLibraryFolder),
                               i18n("Create Folder Failed"));
            return;
        }
    }

    QFileInfo path(albumLibraryFolder);
    if (!path.isWritable()) {
        KMessageBox::information(this, i18n("No write access for this path.\n"
                                         "Warning: the comments and tag features will not work."));
        return;
    }

    config_->setGroup("General Settings");
    config_->writeEntry("Version", digikam_version);

    config_->setGroup("Album Settings");
    config_->writePathEntry("Album Path", albumLibraryFolder);
    config_->sync();

    QDialog::accept();

    QString ErrorMsg, URL;

    if (kapp->startServiceByDesktopName("digikam", URL , &ErrorMsg) > 0)
    {
        kdError() << ErrorMsg << endl;
        KMessageBox::sorry(this, i18n("Cannot restart Digikam automatically.\n"
                                   "Please restart Digikam manually."));
    }
}

}  // namespace Digikam

#include "digikamfirstrun.moc"
