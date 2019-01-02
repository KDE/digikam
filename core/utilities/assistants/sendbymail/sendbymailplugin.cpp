/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : plugin to email items.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "sendbymailplugin.h"

// Qt includes

#include <QPointer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mailwizard.h"

namespace Digikam
{

SendByMailPlugin::SendByMailPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

QString SendByMailPlugin::name() const
{
    return i18n("Send by Email");
}

QString SendByMailPlugin::id() const
{
    return QLatin1String("SendByMail");
}

QString SendByMailPlugin::version() const
{
    return QLatin1String("1.0");
}

QString SendByMailPlugin::resume() const
{
    return i18n("A Tool to Send Items by Email.");
}

QString SendByMailPlugin::description() const
{
    return i18n("This tool permit to backprocess items (as resize) before to send by email."
                "Items to process can be selected one by one or by group through a selection of albums"
                "Different mail client application can be used to process files on the network.");
}

QList<DPluginAuthor> SendByMailPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"));
}

void SendByMailPlugin::setup()
{
    DPluginAction* const ac = new DPluginAction(this);
    ac->setIcon(QIcon::fromTheme(QLatin1String("mail-send")));
    ac->setText(i18nc("@action", "Send by Mail..."));
    ac->setActionName(QLatin1String("sendbymail"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSendByMail()));

    addAction(ac);
}

void SendByMailPlugin::slotSendByMail()
{
    QPointer<MailWizard> wzrd = new MailWizard(0, infoIface());
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
