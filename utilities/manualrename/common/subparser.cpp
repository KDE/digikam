/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract subparser class
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

#include "subparser.h"
#include "subparser.moc"

// Qt includes

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QWidget>

// KDE includes

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

namespace Digikam
{

class SubParserPriv
{
public:

    SubParserPriv()
    {
        buttonRegistered  = false;
        menuRegistered    = false;
        useTokenMenu      = true;
    }

    bool      buttonRegistered;
    bool      menuRegistered;
    bool      useTokenMenu;

    QString   name;
    QIcon     icon;

    TokenList tokens;
};

SubParser::SubParser(const QString& name, const QIcon& icon)
      : QObject(0), d(new SubParserPriv)
{
    d->name = name;
    d->icon = icon;
}

SubParser::~SubParser()
{
    foreach (Token* token, d->tokens)
    {
        delete token;
    }

    d->tokens.clear();
}

QPushButton* SubParser::createButton(const QString& name, const QIcon& icon)
{
    const int maxHeight = 28;

    QPushButton* button = new QPushButton;
    button->setText(name);
    button->setIcon(icon);
    button->setMinimumHeight(maxHeight);
    button->setMaximumHeight(maxHeight);

    return button;
}

QPushButton* SubParser::registerButton(QWidget* parent)
{
    QPushButton* button = 0;
    button = createButton(d->name, d->icon);

    QList<QAction*> actions;

    if (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* menu = new QMenu(button);

        foreach (Token* token, d->tokens)
        {
            actions << token->action();
        }

        menu->addActions(actions);
        button->setMenu(menu);
    }
    else
    {
        Token* token = d->tokens.first();
        connect(button, SIGNAL(clicked()),
                token, SLOT(slotTriggered()));

    }
    button->setParent(parent);

    d->buttonRegistered = button ? true : false;
    return button;
}

QAction* SubParser::registerMenu(QMenu* parent)
{
    QAction* action = 0;
    QList<QAction*> actions;

    if (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* menu = new QMenu(parent);

        foreach (Token* token, d->tokens)
        {
            actions << token->action();
        }

        menu->addActions(actions);
        action = parent->addMenu(menu);
    }
    else
    {
        action = d->tokens.first()->action();
        parent->insertAction(0, action);
    }

    if (action)
    {
        action->setText(d->name);
        action->setIcon(d->icon);
        d->menuRegistered = true;
    }

    return action;
}

bool SubParser::addToken(const QString& id, const QString& name, const QString& description)
{
    if (id.isEmpty() || name.isEmpty() || description.isEmpty())
        return false;

    Token* token = new Token(id, name, description);
    if (!token)
        return false;

    connect(token, SIGNAL(signalTokenTriggered(const QString&)),
            this, SLOT(slotTokenTriggered(const QString&)));

    d->tokens << token;
    return true;
}

void SubParser::useTokenMenu(bool value)
{
    d->useTokenMenu = value;
}

QList<Token*> SubParser::tokens() const
{
    return d->tokens;
}

void SubParser::slotTokenTriggered(const QString& token)
{
    emit signalTokenTriggered(token);
}

bool SubParser::stringIsValid(const QString& str)
{
    QRegExp invalidString("^\\s*$");
    if (str.isEmpty() || invalidString.exactMatch(str))
        return false;
    return true;
}

void SubParser::parse(const QString& parseString, const ParseInformation& info, ParseResults& results)
{
    if (!stringIsValid(parseString))
        return;

    parseOperation(parseString, info, results);
}

} // namespace Digikam
