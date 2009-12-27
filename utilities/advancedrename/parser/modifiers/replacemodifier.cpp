/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
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

#include "replacemodifier.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

// Local includes

#include "parseobjectdialog.h"

namespace Digikam
{

ReplaceDialog::ReplaceDialog(ParseObject* parent)
             : ParseObjectDialog(parent),
               source(0), destination(0), caseSensitive(0)
{
    QString replace  = i18nc("Replace text", "Replace");

    QLabel* srcLabel = new QLabel(replace + ':');
    source           = new KLineEdit(this);

    QLabel* dstLabel = new QLabel(i18nc("Replace text with", "With:"));
    destination      = new KLineEdit(this);

    caseSensitive    = new QCheckBox(i18n("Case sensitive"));
    caseSensitive->setChecked(false);

    isRegExp         = new QCheckBox(i18n("Regular Expression"));
    isRegExp->setChecked(false);

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(caseSensitive, 0, 0, 1,-1);
    mainLayout->addWidget(isRegExp,      1, 0, 1,-1);
    mainLayout->addWidget(srcLabel,      2, 0, 1, 1);
    mainLayout->addWidget(source,        2, 1, 1, 1);
    mainLayout->addWidget(dstLabel,      3, 0, 1, 1);
    mainLayout->addWidget(destination,   3, 1, 1, 1);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(4, 10);
    mainWidget->setLayout(mainLayout);

    setSettingsWidget(mainWidget);

    source->setFocus();
}

ReplaceDialog::~ReplaceDialog()
{
}

// --------------------------------------------------------

ReplaceModifier::ReplaceModifier()
               : Modifier(i18nc("Replace text", "Replace..."), i18n("Replace text in a renaming option"),
                          SmallIcon("document-edit"))
{
    addToken("{replace:\"||old||\", \"||new||\",||options||}",
             i18n("Replace text (||options||: ||r|| = regular expression, ||i|| = ignore case)"));

    QRegExp reg("\\{replace:\"(.*)\",\"(.*)\"(,(r|ri|ir|i))?\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

void ReplaceModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<ReplaceDialog> dlg = new ReplaceDialog(this);
    if (dlg->exec() == KDialog::Accepted)
    {
        QString oldStr = dlg->source->text();
        QString newStr = dlg->destination->text();
        if (!oldStr.isEmpty())
        {
            QString options;

            if (dlg->isRegExp->isChecked())
            {
                options.append('r');
            }

            if (!dlg->caseSensitive->isChecked())
            {
                options.append('i');
            }

            if (!options.isEmpty())
            {
                options.prepend(',');
            }

            result = QString("{replace:\"%1\",\"%2\"%3}").arg(oldStr).arg(newStr).arg(options);
        }
    }
    delete dlg;

    emit signalTokenTriggered(result);
}

QString ReplaceModifier::modifyOperation(const ParseSettings& settings, const QString& str2Modify)
{
    Q_UNUSED(settings);

    const QRegExp& reg  = regExp();
    QString original    = reg.cap(1);
    QString replacement = reg.cap(2);
    QString result      = str2Modify;
    QString options     = reg.cap(4);
    Qt::CaseSensitivity caseType = (!options.isEmpty() && options.contains('i'))
                                   ? Qt::CaseInsensitive
                                   : Qt::CaseSensitive;

    QRegExp ro(original);
    ro.setCaseSensitivity(caseType);

    if (!options.isEmpty() && options.contains('r'))
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
