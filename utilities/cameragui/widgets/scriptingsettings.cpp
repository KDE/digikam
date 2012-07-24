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

#include "scriptingsettings.moc"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>

// KDE includes

#include <kdialog.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <khbox.h>
#include <kdebug.h>

// Local includes

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
    KUrlRequester* script;
    TooltipDialog* tooltipDialog;
    QToolButton*   tooltipToggleButton;
};

ScriptingSettings::ScriptingSettings(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
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

    QVBoxLayout* vlay      = new QVBoxLayout(this);
    d->scriptLabel         = new QLabel(i18n("Execute script for image:"), this);
    KHBox* hbox            = new KHBox(this);
    d->script              = new KUrlRequester(hbox);
    KFile::Modes mode      = KFile::File | KFile::ExistingOnly | KFile::LocalOnly;
    d->script->setMode(mode);
    d->script->setClickMessage(i18n("No script selected"));
    d->tooltipToggleButton = new QToolButton(hbox);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));
    d->tooltipToggleButton->setToolTip(i18n("Show a list of all available options"));

    vlay->addWidget(d->scriptLabel);
    vlay->addWidget(hbox);
    vlay->addStretch();
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

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
    d->script->setText(group.readEntry("Script", QString()));
}

void ScriptingSettings::saveSettings(KConfigGroup& group)
{
    group.writeEntry("Script", d->script->text());
}

void ScriptingSettings::settings(DownloadSettings* const settings) const
{
    settings->script = d->script->text();
}

void ScriptingSettings::slotToolTipButtonToggled(bool /*checked*/)
{
    if (!d->tooltipDialog->isVisible())
    {
        d->tooltipDialog->show();
    }

    d->tooltipDialog->raise();
}

}  // namespace Digikam
