/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mailwizard.h"

namespace DigikamGenericSendByMailPlugin
{

SendByMailPlugin::SendByMailPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

SendByMailPlugin::~SendByMailPlugin()
{
}

QString SendByMailPlugin::name() const
{
    return i18n("Send by Email");
}

QString SendByMailPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SendByMailPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("mail-send"));
}

QString SendByMailPlugin::description() const
{
    return i18n("A tool to send images by E-mail");
}

QString SendByMailPlugin::details() const
{
    return i18n("<p>This tool permit to back-process items (as resize) before to send by e-mail.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Different mail client application can be used to process files on the network.</p>");
}

QList<DPluginAuthor> SendByMailPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Michael Hoechstetter"),
                             QString::fromUtf8("michael dot hoechstetter at gmx dot de"),
                             QString::fromUtf8("(C) 2006"))
            << DPluginAuthor(QString::fromUtf8("Tom Albers"),
                             QString::fromUtf8("tomalbers at kde dot nl"),
                             QString::fromUtf8("(C) 2007"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"),
                             i18n("Author and Maintainer"))
            ;
}

void SendByMailPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Send by Mail..."));
    ac->setObjectName(QLatin1String("sendbymail"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSendByMail()));

    addAction(ac);
}

void SendByMailPlugin::slotSendByMail()
{
    QPointer<MailWizard> wzrd = new MailWizard(nullptr, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace DigikamGenericSendByMailPlugin
