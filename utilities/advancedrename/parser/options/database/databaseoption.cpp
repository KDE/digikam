/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : an option to provide database information to the parser
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

#include "databaseoption.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "imagecomments.h"
#include "dbkeyselector.h"

#include "defaultcommentkey.h"
#include "dimensionkey.h"
#include "filesizekey.h"
#include "formatkey.h"
#include "mediatypekey.h"
#include "ratingkey.h"

namespace Digikam
{

DatabaseOptionDialog::DatabaseOptionDialog(Parseable* parent)
                    : ParseableDialog(parent),
                      dbkeySelectorView(0), separatorLineEdit(0)
{
    QWidget* mainWidget  = new QWidget(this);
    dbkeySelectorView    = new DbKeySelectorView(this);
    QLabel *customLabel  = new QLabel(i18n("Keyword separator:"));
    separatorLineEdit    = new KLineEdit(this);
    separatorLineEdit->setText("_");

    // --------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(customLabel,       0, 0, 1, 1);
    mainLayout->addWidget(separatorLineEdit, 0, 1, 1, 1);
    mainLayout->addWidget(dbkeySelectorView, 1, 0, 1,-1);
    mainWidget->setLayout(mainLayout);

    // --------------------------------------------------------

    setSettingsWidget(mainWidget);
    resize(450, 450);
}

DatabaseOptionDialog::~DatabaseOptionDialog()
{
}

// --------------------------------------------------------

DatabaseOption::DatabaseOption()
              : Option(i18n("Database..."), i18n("Add information from the database"), SmallIcon("server-database"))
{
    addToken("[db:||key||]", i18n("Add database information"));
    QRegExp reg("\\[db(:(.*))\\]");
    reg.setMinimal(true);
    setRegExp(reg);

    registerKeys();
}

DatabaseOption::~DatabaseOption()
{
    unregisterKeys();
}

void DatabaseOption::registerKeys()
{
    addDbOptionKey(new DefaultCommentKey());
    addDbOptionKey(new DimensionKey());
    addDbOptionKey(new FileSizeKey());
    addDbOptionKey(new FormatKey());
    addDbOptionKey(new MediaTypeKey());
    addDbOptionKey(new MediaTypeKey(true));
    addDbOptionKey(new RatingKey());
}

void DatabaseOption::unregisterKeys()
{
    foreach (DbOptionKey* key, m_map)
    {
        if (key) delete key;
    }
    m_map.clear();
}

void DatabaseOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QStringList keys;
    QPointer<DatabaseOptionDialog> dlg = new DatabaseOptionDialog(this);
    dlg->dbkeySelectorView->setKeysMap(m_map);

    if (dlg->exec() == KDialog::Accepted)
    {
        QStringList checkedKeys = dlg->dbkeySelectorView->checkedKeysList();

        foreach (const QString& key, checkedKeys)
        {
            QString keyStr = QString("[db:%1]").arg(key);
            keys << keyStr;
        }
    }

    if (!keys.isEmpty())
    {
        QString tokenStr = keys.join(dlg->separatorLineEdit->text());
        emit signalTokenTriggered(tokenStr);
    }

    delete dlg;
}

QString DatabaseOption::parseOperation(ParseSettings& settings)
{
    const QRegExp& reg  = regExp();
    QString keyword     = reg.cap(2);

    return parseDatabase(keyword, settings);
}

QString DatabaseOption::parseDatabase(const QString& keyword, ParseSettings& settings)
{
    if (settings.fileUrl.isEmpty() || keyword.isEmpty())
    {
        return QString();
    }

    DbOptionKey* dbkey = 0;
    dbkey = m_map.value(keyword);
    if (!dbkey) return QString();

    return dbkey->getValue(settings);
}

void DatabaseOption::addDbOptionKey(DbOptionKey* key)
{
    if (!key || key->name.isEmpty() || key->description.isEmpty())
    {
        return;
    }
    m_map.insert(key->name, key);
}

} // namespace Digikam
