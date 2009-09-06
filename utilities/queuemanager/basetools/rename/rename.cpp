/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-06
 * Description : batch tool for renaming files.
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "rename.h"
#include "rename.moc"

// Qt includes

#include <QWidget>
#include <QVBoxLayout>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kdebug.h>

// Local includes

#include "dimgimagefilters.h"
#include "manualrenamewidget.h"
#include "parser.h"

using namespace Digikam::ManualRename;
namespace Digikam
{

class RenamePriv
{

public:

    RenamePriv()
    {
        renamingWidget = 0;
    }
    ManualRenameWidget* renamingWidget;
};

Rename::Rename(QObject* parent)
      : BatchTool("Rename", BaseTool, parent), d(new RenamePriv)
{
    setToolTitle(i18n("Rename"));
    setToolDescription(i18n("A tool for renaming files"));
    setToolIcon(KIcon(SmallIcon("insert-text")));

    // --------------------------------------------------------

    d->renamingWidget       = new ManualRenameWidget;
    QWidget* mainWidget     = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(d->renamingWidget);
    mainLayout->addStretch(10);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(0);
    mainWidget->setLayout(mainLayout);

    setSettingsWidget(mainWidget);

    // --------------------------------------------------------

    connect(d->renamingWidget, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotSettingsChanged()));
}

Rename::~Rename()
{
    delete d;
}

BatchToolSettings Rename::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("Text", QString());
    return settings;
}

void Rename::assignSettings2Widget()
{
}

void Rename::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("text", d->renamingWidget->text());
    setSettings(settings);
}

bool Rename::toolOperations()
{
    return true;
}

QString Rename::outputBaseName() const
{
    QString baseName;
    ParseInformation info = parseInformation();

    if (!info.isEmpty())
        baseName = d->renamingWidget->parse(info);

    return baseName;
}

}  // namespace Digikam
