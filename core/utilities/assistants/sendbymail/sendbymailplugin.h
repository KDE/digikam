/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : plugin to email items.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_SENDBYMAIL_PLUGIN_H
#define DIGIKAM_SENDBYMAIL_PLUGIN_H

// Local includes

#include "dplugin.h"

namespace Digikam
{

class SendByMailPlugin : public DPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.digikam.SendByMail")
    Q_INTERFACES(Digikam::DPlugin)

public:

    explicit SendByMailPlugin(QObject* const parent = 0);

    QString name() const                 override;
    QString id() const                   override;
    QString version() const              override;
    QString description() const          override;
    QList<DPluginAuthor> authors() const override;

    void setup();

private Q_SLOTS:

    void slotSendByMail();
};

} // namespace Digikam

#endif // DIGIKAM_SENDBYMAIL_PLUGIN_H
