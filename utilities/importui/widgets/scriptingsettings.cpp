/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-20
 * Description : scripting settings for camera interface.
 *
 * Copyright (C) 2012 by Petri Damstén <damu@iki.fi>
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

#include "scriptingsettings.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "dlayoutbox.h"
#include "dfileselector.h"
#include "digikam_debug.h"
#include "tooltipdialog.h"

namespace Digikam
{

class ScriptingSettings::Private
{
public:

    Private()
        : scriptLabel(0),
          script(0),
          tooltipDialog(0),
          tooltipToggleButton(0)
    {
    }

    QLabel*        scriptLabel;
    DFileSelector* script;
    TooltipDialog* tooltipDialog;
    QToolButton*   tooltipToggleButton;
};

ScriptingSettings::ScriptingSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->tooltipDialog = new TooltipDialog(this);
    d->tooltipDialog->setTooltip(i18n("<p>These expressions may be used to customize the command line:</p>"
                                      "<p><b>%file</b>: full path of the imported file</p>"
                                      "<p><b>%filename</b>: file name of the imported file</p>"
                                      "<p><b>%path</b>: path of the imported file</p>"
                                      "<p><b>%orgfilename</b>: original file name</p>"
                                      "<p><b>%orgpath</b>: original path</p>"
                                      "<p>If there are no expressions full path is added to the command.<p>"
                                     ));
    d->tooltipDialog->resize(650, 530);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    d->scriptLabel          = new QLabel(i18n("Execute script for image:"), this);
    DHBox* const hbox       = new DHBox(this);
    d->script               = new DFileSelector(hbox);
    d->script->setFileDlgMode(DFileDialog::ExistingFile);
    d->script->lineEdit()->setPlaceholderText(i18n("No script selected"));
    d->tooltipToggleButton  = new QToolButton(hbox);
    d->tooltipToggleButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->tooltipToggleButton->setToolTip(i18n("Show a list of all available options"));

    vlay->addWidget(d->scriptLabel);
    vlay->addWidget(hbox);
    vlay->addStretch();
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    setWhatsThis(i18n("Set here the script that is executed for every imported image."));

    // ---------------------------------------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(clicked(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));
}

ScriptingSettings::~ScriptingSettings()
{
    delete d;
}

void ScriptingSettings::readSettings(KConfigGroup& group)
{
    d->script->setFileDlgPath(group.readEntry("Script", QString()));
}

void ScriptingSettings::saveSettings(KConfigGroup& group)
{
    group.writeEntry("Script", d->script->fileDlgPath());
}

void ScriptingSettings::settings(DownloadSettings* const settings) const
{
    settings->script = d->script->fileDlgPath();
}

void ScriptingSettings::slotToolTipButtonToggled(bool /*checked*/)
{
    if (!d->tooltipDialog->isVisible())
    {
        d->tooltipDialog->show();
    }

    d->tooltipDialog->raise();
}

} // namespace Digikam
