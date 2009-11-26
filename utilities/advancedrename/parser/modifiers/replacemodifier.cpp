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
    caseSensitive->setChecked(true);

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(caseSensitive, 0, 0, 1,-1);
    mainLayout->addWidget(srcLabel,      1, 0, 1, 1);
    mainLayout->addWidget(source,        1, 1, 1, 1);
    mainLayout->addWidget(dstLabel,      2, 0, 1, 1);
    mainLayout->addWidget(destination,   2, 1, 1, 1);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(3, 10);
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
    setUseTokenMenu(false);

    addTokenDescription(QString("{r:\"|old|\", \"|new|\"}"),  i18nc("Replace text", "Replace"),
                                                              i18n("Replace text"));

    addTokenDescription(QString("{ri:\"|old|\", \"|new|\"}"), i18nc("Replace text (case insensitive", "Replace (case insensitive)"),
                                                              i18n("Replace text (case insensitive)"));

    QRegExp reg("\\{r(i)?:\"(.+)\",\"(.*)\"\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

void ReplaceModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString tmp;

    QPointer<ReplaceDialog> dlg = new ReplaceDialog(this);
    if (dlg->exec() == KDialog::Accepted)
    {
        QString oldStr = dlg->source->text();
        QString newStr = dlg->destination->text();
        if (!oldStr.isEmpty())
        {
            if (dlg->caseSensitive->isChecked())
            {
                tmp = QString("{r:\"%1\",\"%2\"}").arg(oldStr).arg(newStr);
            }
            else
            {
                tmp = QString("{ri:\"%1\",\"%2\"}").arg(oldStr).arg(newStr);
            }
        }
    }
    delete dlg;

    emit signalTokenTriggered(tmp);
}

QString ReplaceModifier::modifyOperation(const QString& parseString, const QString& result)
{
    QRegExp reg = regExp();
    int pos     = 0;
    pos         = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        QString original    = reg.cap(2);
        QString replacement = reg.cap(3);
        QString _result     = result;

        Qt::CaseSensitivity caseType = (!reg.cap(1).isEmpty() && reg.cap(1).count() == 1)
                                       ? Qt::CaseInsensitive
                                       : Qt::CaseSensitive;
        _result.replace(original, replacement, caseType);
        return _result;
    }
    return QString();
}

} // namespace Digikam
