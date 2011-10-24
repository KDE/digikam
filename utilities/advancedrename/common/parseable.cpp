/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : an abstract parseable class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "parseable.moc"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QString>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

class ParseablePriv
{
public:

    ParseablePriv() :
        useTokenMenu(false)
    {}

    bool         useTokenMenu;

    QString      description;
    QString      iconName;
    QRegExp      regExp;

    TokenList    tokens;
};

Parseable::Parseable(const QString& name)
    : QObject(0), d(new ParseablePriv)
{
    setObjectName(name);
}

Parseable::Parseable(const QString& name, const QString& icon)
    : QObject(0), d(new ParseablePriv)
{
    setObjectName(name);
    setIcon(icon);
}

Parseable::~Parseable()
{
    qDeleteAll(d->tokens);
    d->tokens.clear();

    delete d;
}

void Parseable::setIcon(const QString& iconName)
{
    d->iconName = iconName;
}

QPixmap Parseable::icon(Parseable::IconType type) const
{
    QPixmap icon;

    switch (type)
    {
        case Dialog:
            icon = DesktopIcon(d->iconName);
            break;
        default:
            icon = SmallIcon(d->iconName);
            break;
    }

    return icon;
}

void Parseable::setDescription(const QString& desc)
{
    d->description = desc;
}

QString Parseable::description() const
{
    return d->description;
}

QRegExp& Parseable::regExp() const
{
    return d->regExp;
}

void Parseable::setRegExp(const QRegExp& regExp)
{
    d->regExp = regExp;
}

QPushButton* Parseable::createButton(const QString& name, const QIcon& icon)
{
    const int maxHeight = 28;

    QPushButton* button = new QPushButton;
    button->setText(name);
    button->setIcon(icon);
    button->setMinimumHeight(maxHeight);
    button->setMaximumHeight(maxHeight);

    return button;
}

QPushButton* Parseable::registerButton(QWidget* parent)
{
    QPushButton* button = 0;
    button = createButton(objectName(), icon());

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
    else if (!d->tokens.isEmpty())
    {
        Token* token = d->tokens.first();
        connect(button, SIGNAL(clicked()),
                token, SLOT(slotTriggered()));

    }

    button->setParent(parent);

    return button;
}

QAction* Parseable::registerMenu(QMenu* parent)
{
    QAction* action = 0;

    if (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* menu = new QMenu(parent);
        QList<QAction*> actions;

        foreach (Token* token, d->tokens)
        {
            actions << token->action();
        }

        menu->addActions(actions);
        action = parent->addMenu(menu);
    }
    else if (!d->tokens.isEmpty())
    {
        action = d->tokens.first()->action();
        parent->insertAction(0, action);
    }

    if (action)
    {
        action->setText(objectName());
        action->setIcon(icon());
    }

    return action;
}

bool Parseable::addToken(const QString& id, const QString& description, const QString& actionName)
{
    if (id.isEmpty() || description.isEmpty())
    {
        return false;
    }

    Token* token = new Token(id, description);

    if (!token)
    {
        return false;
    }

    if (!actionName.isEmpty())
    {
        token->action()->setText(actionName);
    }

    connect(token, SIGNAL(signalTokenTriggered(QString)),
            this, SLOT(slotTokenTriggered(QString)));

    d->tokens << token;
    return true;
}

void Parseable::setUseTokenMenu(bool value)
{
    d->useTokenMenu = value;
}

bool Parseable::useTokenMenu() const
{
    return d->useTokenMenu;
}

TokenList Parseable::tokens() const
{
    return d->tokens;
}

void Parseable::slotTokenTriggered(const QString& token)
{
    emit signalTokenTriggered(token);
}

bool Parseable::tokenAtPosition(ParseResults& results, int pos)
{
    int start;
    int length;
    return tokenAtPosition(results, pos, start, length);
}

bool Parseable::tokenAtPosition(ParseResults& results, int pos, int& start, int& length)
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

bool Parseable::isValid() const
{
    return (!d->tokens.isEmpty() && !d->regExp.isEmpty() && d->regExp.isValid());
}

void Parseable::reset()
{
}

QString Parseable::escapeToken(const QString& token)
{
    QString escaped = token;

    // replace special characters for renaming options
    escaped.replace('[', "\\[");
    escaped.replace(']', "\\]");

    // replace special characters for modifiers
    escaped.replace('{', "\\{");
    escaped.replace('}', "\\}");

    return escaped;
}

} // namespace Digikam
