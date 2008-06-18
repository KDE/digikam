/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : dialog displayed at the first digiKam run
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include <QLabel>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kseparator.h>

// Local includes.

#include "ddebug.h"
#include "version.h"
#include "digikamfirstrun.h"
#include "digikamfirstrun.moc"

namespace Digikam
{

DigikamFirstRun::DigikamFirstRun(QWidget* parent)
               : KDialog(parent)
{
    setModal(true);
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setCaption(i18n("First Run"));
    setHelp("firstrundialog.anchor", "digikam");

    QWidget *widget = new QWidget(this);
    setMainWidget(widget);

    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    QLabel *textLabel2   = new QLabel(widget);
    textLabel2->setText(i18n( "<b>Setting-Up First Local Root Album Path</b>"));

    KSeparator *line1    = new KSeparator(Qt::Horizontal, widget);
    QGridLayout *grid    = new QGridLayout();
    m_pixLabel           = new QLabel(widget);
    m_pixLabel->setAlignment(Qt::AlignTop);
    m_pixLabel->setPixmap(QPixmap(KStandardDirs::locate("data",
                                  "digikam/data/logo-digikam.png"))
                          .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_path = new KUrlRequester(widget);
    m_path->setMode(KFile::LocalOnly | KFile::Directory);
    m_path->setUrl(QDir::homePath() + i18nc("This is a path name so you should "
                   "include the slash in the translation", "/Pictures"));
    m_path->setMode(KFile::Directory | KFile::LocalOnly);

    QLabel *textLabel1 = new QLabel(widget);
    textLabel1->setAlignment(Qt::AlignVCenter);
    textLabel1->setWordWrap(true);
    textLabel1->setText(i18n("<p>digiKam use root album paths to store your photo albums "
                             "created in <b>My Albums</b> view from left side-bar. "
                             "Below, please select which folder you would like "
                             "digiKam to use as first root album path from your local "
                             "file system.</p>" 
                             "<p><b>Note: you can set other root album paths later using "
                             "digiKam settings panel. Removable media and shared file system are "
                             "supported.</b></p>") );

    grid->addWidget(m_pixLabel, 0, 0, 2, 1);
    grid->addWidget(textLabel1, 0, 1, 1, 1);
    grid->addWidget(m_path,     1, 1, 1, 1);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    vlayout->addWidget(textLabel2);
    vlayout->addWidget(line1);
    vlayout->addLayout(grid);
    vlayout->addStretch(10);
    vlayout->setMargin(0);
    vlayout->setSpacing(KDialog::spacingHint());

    widget->setMinimumSize(450, sizeHint().height());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));
}

DigikamFirstRun::~DigikamFirstRun()
{
}

void DigikamFirstRun::slotOk()
{
    QString albumLibraryFolder = m_path->url().path();

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
                                        "Would you like digiKam to create it?</qt>",
                                        albumLibraryFolder),
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
                                    "<p><b>%1</b></p></qt>", albumLibraryFolder),
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
    group.writeEntry("Database File Path", albumLibraryFolder);

    config->sync();

    KDialog::accept();
}

}  // namespace Digikam
