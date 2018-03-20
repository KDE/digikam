/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : an abstract rule class
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

#include "rule.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QString>
#include <QIcon>
#include <QApplication>
#include <QStyle>

namespace Digikam
{

class Rule::Private
{
public:

    Private() :
        useTokenMenu(false)
    {}

    bool         useTokenMenu;

    QString      description;
    QString      iconName;
    QRegExp      regExp;

    TokenList    tokens;
};

Rule::Rule(const QString& name)
    : QObject(0),
      d(new Private)
{
    setObjectName(name);
}

Rule::Rule(const QString& name, const QString& icon)
    : QObject(0),
      d(new Private)
{
    setObjectName(name);
    setIcon(icon);
}

Rule::~Rule()
{
    qDeleteAll(d->tokens);
    d->tokens.clear();

    delete d;
}

void Rule::setIcon(const QString& iconName)
{
    d->iconName = iconName;
}

QPixmap Rule::icon(Rule::IconType type) const
{
    QPixmap icon;

    switch (type)
    {
        case Dialog:
            icon = QIcon::fromTheme(d->iconName).pixmap(48);
            break;

        default:
            icon = QIcon::fromTheme(d->iconName).pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
            break;
    }

    return icon;
}

void Rule::setDescription(const QString& desc)
{
    d->description = desc;
}

QString Rule::description() const
{
    return d->description;
}

QRegExp& Rule::regExp() const
{
    return d->regExp;
}

void Rule::setRegExp(const QRegExp& regExp)
{
    d->regExp = regExp;
}

QPushButton* Rule::createButton(const QString& name, const QIcon& icon)
{
    const int maxHeight = 28;

    QPushButton* const button = new QPushButton;
    button->setText(name);
    button->setIcon(icon);
    button->setMinimumHeight(maxHeight);
    button->setMaximumHeight(maxHeight);

    return button;
}

QPushButton* Rule::registerButton(QWidget* parent)
{
    QPushButton* button = createButton(objectName(), icon());

    QList<QAction*> actions;

    if (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* const menu = new QMenu(button);

        foreach(Token* const token, d->tokens)
        {
            actions << token->action();
        }

        menu->addActions(actions);
        button->setMenu(menu);
    }
    else if (!d->tokens.isEmpty())
    {
        Token* const token = d->tokens.first();

        connect(button, SIGNAL(clicked()),
                token, SLOT(slotTriggered()));

    }

    button->setParent(parent);

    return button;
}

QAction* Rule::registerMenu(QMenu* parent)
{
    QAction* action = 0;

    if (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* const menu = new QMenu(parent);
        QList<QAction*> actions;

        foreach(Token* const token, d->tokens)
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

bool Rule::addToken(const QString& id, const QString& description, const QString& actionName)
{
    if (id.isEmpty() || description.isEmpty())
    {
        return false;
    }

    Token* const token = new Token(id, description);

    if (!actionName.isEmpty())
    {
        token->action()->setText(actionName);
    }

    connect(token, SIGNAL(signalTokenTriggered(QString)),
            this, SLOT(slotTokenTriggered(QString)));

    d->tokens << token;
    return true;
}

void Rule::setUseTokenMenu(bool value)
{
    d->useTokenMenu = value;
}

bool Rule::useTokenMenu() const
{
    return d->useTokenMenu;
}

TokenList& Rule::tokens() const
{
    return d->tokens;
}

void Rule::slotTokenTriggered(const QString& token)
{
    emit signalTokenTriggered(token);
}

bool Rule::isValid() const
{
    return (!d->tokens.isEmpty() && !d->regExp.isEmpty() && d->regExp.isValid());
}

void Rule::reset()
{
}

QString Rule::escapeToken(const QString& token)
{
    QString escaped = token;

    // replace special characters for renaming options
    escaped.replace(QLatin1Char('['), QLatin1String("\\["));
    escaped.replace(QLatin1Char(']'), QLatin1String("\\]"));

    // replace special characters for modifiers
    escaped.replace(QLatin1Char('{'), QLatin1String("\\{"));
    escaped.replace(QLatin1Char('}'), QLatin1String("\\}"));

    return escaped;
}

ParseResults Rule::parse(ParseSettings &settings)
{
    ParseResults parsedResults;
    const QRegExp& reg         = regExp();
    const QString& parseString = settings.parseString;

    int pos = 0;

    while (pos > -1)
    {
        pos = reg.indexIn(parseString, pos);

        if (pos > -1)
        {
            QString result = parseOperation(settings);

            ParseResults::ResultsKey   k(pos, reg.cap(0).count());
            ParseResults::ResultsValue v(reg.cap(0), result);
            parsedResults.addEntry(k, v);
            pos += reg.matchedLength();
        }
    }

    return parsedResults;
}

} // namespace Digikam
