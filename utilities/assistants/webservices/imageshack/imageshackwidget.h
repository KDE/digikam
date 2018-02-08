/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMAGESHACK_WIDGET_H
#define IMAGESHACK_WIDGET_H

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "dprogresswdg.h"
#include "wssettingswidget.h"
#include "dinfointerface.h"

namespace Digikam
{

class ImageShackSession;

class ImageShackWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit ImageShackWidget(QWidget* const parent,
                              ImageShackSession* const session,
                              DInfoInterface* const iface,
                              const QString& toolName);
    ~ImageShackWidget();

Q_SIGNALS:

    void signalReloadGalleries();

private:

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotGetGalleries(const QStringList& gTexts, const QStringList& gNames);
    void slotReloadGalleries();

private:

    class Private;
    Private* const d;

    friend class ImageShackWindow;
};

}  // namespace Digikam

#endif // IMAGESHACK_WIDGET_H
