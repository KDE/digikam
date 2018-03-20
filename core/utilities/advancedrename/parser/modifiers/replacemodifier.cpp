/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "replacemodifier.h"

// Qt includes

#include <QCheckBox>
#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ruledialog.h"
#include "ui_replacemodifierdialogwidget.h"

namespace Digikam
{

ReplaceDialog::ReplaceDialog(Rule* const parent)
    : RuleDialog(parent),
      ui(new Ui::ReplaceModifierDialogWidget())
{
    QWidget* const mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);
    setSettingsWidget(mainWidget);
    ui->source->setFocus();
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

// --------------------------------------------------------

ReplaceModifier::ReplaceModifier()
    : Modifier(i18nc("Replace text", "Replace..."), i18n("Replace text in a renaming option"),
               QLatin1String("document-edit"))
{
    addToken(QLatin1String("{replace:\"||old||\", \"||new||\",||options||}"),
             i18n("Replace text (||options||: ||r|| = regular expression, ||i|| = ignore case)"));

    QRegExp reg(QLatin1String("\\{replace(:\"(.*)\",\"(.*)\"(,(r|ri|ir|i))?)\\}"));
    reg.setMinimal(true);
    setRegExp(reg);
}

void ReplaceModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<ReplaceDialog> dlg = new ReplaceDialog(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        QString oldStr = dlg->ui->source->text();
        QString newStr = dlg->ui->destination->text();

        if (!oldStr.isEmpty())
        {
            QString options;

            if (dlg->ui->isRegExp->isChecked())
            {
                options.append(QLatin1Char('r'));
            }

            if (!dlg->ui->caseSensitive->isChecked())
            {
                options.append(QLatin1Char('i'));
            }

            if (!options.isEmpty())
            {
                options.prepend(QLatin1Char(','));
            }

            result = QString::fromUtf8("{replace:\"%1\",\"%2\"%3}").arg(oldStr).arg(newStr).arg(options);
        }
    }

    delete dlg;

    emit signalTokenTriggered(result);
}

QString ReplaceModifier::parseOperation(ParseSettings& settings)
{
    const QRegExp& reg  = regExp();
    QString original    = reg.cap(2);
    QString replacement = reg.cap(3);
    QString result      = settings.str2Modify;
    QString options     = reg.cap(5);
    Qt::CaseSensitivity caseType = (!options.isEmpty() && options.contains(QLatin1Char('i')))
                                   ? Qt::CaseInsensitive
                                   : Qt::CaseSensitive;

    QRegExp ro(original);
    ro.setCaseSensitivity(caseType);

    if (!options.isEmpty() && options.contains(QLatin1Char('r')))
    {
        result.replace(ro, replacement);
    }
    else
    {
        result.replace(original, replacement, caseType);
    }

    return result;
}

} // namespace Digikam
