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

#include <QGridLayout>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

namespace Digikam
{

class ReplaceDialogPriv
{
public:

    ReplaceDialogPriv()
    {
        source      = 0;
        destination = 0;
    }

    KLineEdit* source;
    KLineEdit* destination;
};

ReplaceDialog::ReplaceDialog()
             : KDialog(0), d(new ReplaceDialogPriv)
{
    QString replace = i18nc("Replace text", "Replace");

    setCaption(replace);

    QLabel* srcLabel = new QLabel(replace + ':');
    d->source        = new KLineEdit(this);

    QLabel* dstLabel = new QLabel(i18nc("Replace text with", "With:"));
    d->destination   = new KLineEdit(this);

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(srcLabel,       0, 0);
    mainLayout->addWidget(d->source,      0, 1);
    mainLayout->addWidget(dstLabel,       1, 0);
    mainLayout->addWidget(d->destination, 1, 1);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(2, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    d->source->setFocus();
}

ReplaceDialog::~ReplaceDialog()
{
    delete d;
}

QString ReplaceDialog::source() const
{
    return d->source->text();
}

QString ReplaceDialog::destination() const
{
    return d->destination->text();
}

// --------------------------------------------------------

ReplaceModifier::ReplaceModifier()
               : Modifier(i18nc("Replace text", "Replace..."), i18n("Replace text"),
                          SmallIcon("document-edit"))
{
    addTokenDescription(QString("{r:\"|old|\", \"|new|\"}"),  i18n("Replace"), description());
    addTokenDescription(QString("{ri:\"|old|\", \"|new|\"}"), i18n("Replace (case insensitive)"),
                                                              i18n("Replace text (case insensitive)"));

    setRegExp("\\{r(i)?:\"(.+)\",\"(.*)\"\\}");
}

void ReplaceModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString tmp;

    QPointer<ReplaceDialog> dlg = new ReplaceDialog;
    if (dlg->exec() == KDialog::Accepted)
    {
        QString oldStr = dlg->source();
        QString newStr = dlg->destination();
        if (!oldStr.isEmpty())
        {
            tmp = QString("{r:\"%1\",\"%2\"}").arg(oldStr).arg(newStr);
        }
    }
    delete dlg;

    emit signalTokenTriggered(tmp);
}

QString ReplaceModifier::modifyOperation(const QString& parseString, const QString& result)
{
    QRegExp reg = regExp();

    int pos = 0;
    pos     = reg.indexIn(parseString, pos);
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
