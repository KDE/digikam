/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-22
 * Description : an abstract parse object class
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

#include "parseobject.h"
#include "parseobject.moc"

// Qt includes

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QString>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

class ParseObjectPriv
{
public:

    ParseObjectPriv() :
        buttonRegistered(false),
        menuRegistered(false),
        useTokenMenu(true)
    {}

    bool         buttonRegistered;
    bool         menuRegistered;
    bool         useTokenMenu;

    QString      description;
    QIcon        icon;
    QRegExp      regExp;

    TokenList    tokens;
};

ParseObject::ParseObject(const QString& name, const QIcon& icon)
           : QObject(0), d(new ParseObjectPriv)
{
    setObjectName(name);
    d->icon = icon;
}

ParseObject::~ParseObject()
{
    foreach (Token* token, d->tokens)
    {
        delete token;
    }
    d->tokens.clear();

    delete d;
}

void ParseObject::setDescription(const QString& desc)
{
    d->description = desc;
}

QString ParseObject::description() const
{
    return d->description;
}

QRegExp ParseObject::regExp() const
{
    return d->regExp;
}

void ParseObject::setRegExp(const QString& regExp)
{
    d->regExp = QRegExp(regExp);
}

QPushButton* ParseObject::createButton(const QString& name, const QIcon& icon)
{
    const int maxHeight = 28;

    QPushButton* button = new QPushButton;
    button->setText(name);
    button->setIcon(icon);
    button->setMinimumHeight(maxHeight);
    button->setMaximumHeight(maxHeight);

    return button;
}

QPushButton* ParseObject::registerButton(QWidget* parent)
{
    QPushButton* button = 0;
    button = createButton(objectName(), d->icon);

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

QAction* ParseObject::registerMenu(QMenu* parent)
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
        action->setText(objectName());
        action->setIcon(d->icon);
        d->menuRegistered = true;
    }

    return action;
}

bool ParseObject::addTokenDescription(const QString& id, const QString& name, const QString& description)
{
    if (id.isEmpty() || name.isEmpty() || description.isEmpty())
    {
        return false;
    }

    Token* token = new Token(id, name, description);
    if (!token)
    {
        return false;
    }

    connect(token, SIGNAL(signalTokenTriggered(const QString&)),
            this, SLOT(slotTokenTriggered(const QString&)));

    d->tokens << token;
    return true;
}

void ParseObject::setUseTokenMenu(bool value)
{
    d->useTokenMenu = value;
}

bool ParseObject::useTokenMenu() const
{
    return d->useTokenMenu;
}

TokenList ParseObject::tokens() const
{
    return d->tokens;
}

void ParseObject::slotTokenTriggered(const QString& token)
{
    emit signalTokenTriggered(token);
}

bool ParseObject::tokenAtPosition(ParseResults& results, int pos)
{
    int start;
    int length;
    return tokenAtPosition(results, pos, start, length);
}

bool ParseObject::tokenAtPosition(ParseResults& results, int pos, int& start, int& length)
{
    bool found = false;

    ParseResults::ResultsKey key = results.keyAtApproximatePosition(pos);
    start  = key.first;
    length = key.second;

    if ((pos >= start) && (pos <= start + length))
    {
        found = true;
    }
    return found;
}

bool ParseObject::isValid() const
{
    return (!d->tokens.isEmpty() && !d->regExp.isEmpty() && d->regExp.isValid());
}

} // namespace Digikam
