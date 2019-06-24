/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : Plasma Address Book contacts interface
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "akonadiiface.h"

// Qt includes

#include <QApplication>
#include <QAction>
#include <QString>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

#if defined(Q_OS_DARWIN) && defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wundef"
#endif

#include <kjob.h>
#include <AkonadiCore/Item>
#include <Akonadi/Contact/ContactSearchJob>
#include <KContacts/Addressee>

#if defined(Q_OS_DARWIN) && defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

// TODO: Port from KABC::AddressBook to libakonadi-kontact. For instance using Akonadi::ContactSearchJob.
// See http://techbase.kde.org/Development/AkonadiPorting/AddressBook

AkonadiIface::AkonadiIface(QMenu* const parent)
    : QObject(parent)
{
    m_parent  = parent;
    m_ABCmenu = 0;

    m_ABCmenu = new QMenu(m_parent);

    QAction* const abcAction = m_ABCmenu->menuAction();
    abcAction->setIcon(QIcon::fromTheme(QLatin1String("tag-addressbook")));
    abcAction->setText(i18n("Create Tag From Address Book"));
    m_parent->addMenu(m_ABCmenu);

    QAction* const nothingFound = m_ABCmenu->addAction(i18n("No address book entries found"));
    nothingFound->setEnabled(false);

    Akonadi::ContactSearchJob* const job = new Akonadi::ContactSearchJob();

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotABCSearchResult(KJob*)));
}

AkonadiIface::~AkonadiIface()
{
    delete m_ABCmenu;
}

void AkonadiIface::slotABCSearchResult(KJob* job)
{
    if (job->error())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Akonadi search was not successful";
        return;
    }

    Akonadi::ContactSearchJob* const searchJob = qobject_cast<Akonadi::ContactSearchJob*>(job);
    const KContacts::Addressee::List contacts  = searchJob->contacts();

    if (contacts.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No contacts in Akonadi";
        return;
    }

    QStringList names;

    foreach(const KContacts::Addressee& addr, contacts)
    {
        if (!addr.realName().isNull())
        {
            names.append(addr.realName());
        }
    }

    names.removeDuplicates();
    names.sort();

    if (names.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No names in the address book";
        return;
    }

    m_ABCmenu->clear();

    foreach(const QString& name, names)
    {
        m_ABCmenu->addAction(name);
    }

    connect(m_ABCmenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotABCMenuTriggered(QAction*)));
}

void AkonadiIface::slotABCMenuTriggered(QAction* action)
{
    QString name = action->iconText();
    emit signalContactTriggered(name);
}

} // namespace Digikam
