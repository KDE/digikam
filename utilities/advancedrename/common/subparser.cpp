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
#include <kdebug.h>

// Local includes

#include "lowercasemodifier.h"
#include "uppercasemodifier.h"
#include "firstletterofeachworduppercasemodifier.h"
#include "trimmedmodifier.h"
#include "rangemodifier.h"
#include "replacemodifier.h"

namespace Digikam
{

class SubParserPriv
{
public:

    SubParserPriv()
    {
        buttonRegistered = false;
        menuRegistered   = false;
        useTokenMenu     = true;
        useModifiers     = true;
    }

    bool         buttonRegistered;
    bool         menuRegistered;
    bool         useTokenMenu;
    bool         useModifiers;

    QString      name;
    QIcon        icon;

    TokenList    tokens;
    ParseResults parseResults;
    ParseResults modifierResults;
    ModifierList modifiers;
};

SubParser::SubParser(const QString& name, const QIcon& icon)
      : QObject(0), d(new SubParserPriv)
{
    d->name = name;
    d->icon = icon;

    registerModifier(new LowerCaseModifier());
    registerModifier(new UpperCaseModifier());
    registerModifier(new FirstLetterEachWordUpperCaseModifier());
    registerModifier(new TrimmedModifier());
    registerModifier(new RangeModifier());
    registerModifier(new ReplaceModifier());
}

SubParser::~SubParser()
{
    foreach (Token* token, d->tokens)
    {
        delete token;
    }

    foreach (Modifier* modifier, d->modifiers)
    {
        delete modifier;
    }

    d->modifiers.clear();
    d->tokens.clear();
}

void SubParser::registerModifier(Modifier* modifier)
{
    if (!modifier)
        return;

    d->modifiers.append(modifier);
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

void SubParser::setUseTokenMenu(bool value)
{
    d->useTokenMenu = value;
}

bool SubParser::useTokenMenu() const
{
    return d->useTokenMenu;
}

void SubParser::setUseModifiers(bool value)
{
    d->useModifiers = value;
}

bool SubParser::useModifiers() const
{
    return d->useModifiers;
}

TokenList SubParser::tokens() const
{
    return d->tokens;
}

ModifierList SubParser::modifiers() const
{
    return d->modifiers;
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

void SubParser::parse(const QString& parseString, const ParseInformation& info)
{
    if (!stringIsValid(parseString))
        return;

    d->parseResults.clear();
    d->modifierResults.clear();

    parseOperation(parseString, info, d->parseResults);

    if (d->useModifiers)
        d->modifierResults = applyModifiers(parseString, d->parseResults);
    else
        d->modifierResults.clear();
}

ParseResults SubParser::parseResults()
{
    return d->parseResults;
}

ParseResults SubParser::modifiedResults()
{
    if (d->modifierResults.isEmpty() || !d->useModifiers)
        return d->parseResults;
    return d->modifierResults;
}

ParseResults SubParser::applyModifiers(const QString& parseString, ParseResults& results)
{
    ParseResults tmp;

    ParseResults modifiers;
    QMap<ParseResults::ResultsKey, Modifier*> modifierCallbackMap;

    if (results.isEmpty())
        return tmp;

    tmp = results;

    // fill modifiers ParseResults with all possible modifier tokens
    foreach (Modifier* modifier, d->modifiers)
    {
        QRegExp regExp = modifier->regExp();
        regExp.setMinimal(true);
        int pos = 0;
        while (pos > -1)
        {
            pos = regExp.indexIn(parseString, pos);
            if (pos > -1)
            {
                ParseResults::ResultsKey   k(pos, regExp.matchedLength());
                ParseResults::ResultsValue v(regExp.cap(0), QString());

                modifiers.addEntry(k, v);
                modifierCallbackMap.insert(k, modifier);

                pos += regExp.matchedLength();
            }
        }
    }

    // Check for valid modifiers (they must appear directly after a token) and apply the modification to the token result.
    // We need to create a second ParseResults object with modified keys, otherwise the final parsing step will not
    // remove the modifier tokens from the result.

    foreach (ParseResults::ResultsKey key, results.keys())
    {
        int off  = results.offset(key);
        int diff = 0;
        for (int pos = off; pos < parseString.count();)
        {
            if (modifiers.hasKeyAtPosition(pos))
            {
                ParseResults::ResultsKey mkey = modifiers.keyAtPosition(pos);
                Modifier* mod                 = modifierCallbackMap[mkey];
                QString modToken              = modifiers.token(mkey);

                QString token                 = results.token(key);
                QString result                = results.result(key);

                QString modResult             = mod->modify(modToken, result);

                // update result
                ParseResults::ResultsKey   kResult = key;
                ParseResults::ResultsValue vResult(token, modResult);
                results.addEntry(kResult, vResult);

                // update modifier map
                ParseResults::ResultsKey kModifier = key;
                kModifier.second += diff;
                ParseResults::ResultsValue vModifier(modToken, modResult);

                tmp.deleteEntry(kModifier);
                kModifier.second += modToken.count();
                tmp.addEntry(kModifier, vModifier);

                // set position to the next possible token
                pos  += mkey.second;
                diff += mkey.second;
            }
            else
            {
                break;
            }
        }
    }
    return tmp;
}

bool SubParser::tokenAtPosition(ParseResults& results, int pos)
{
    int start;
    int length;
    return tokenAtPosition(results, pos, start, length);
}

bool SubParser::tokenAtPosition(ParseResults& results, int pos, int& start, int& length)
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

} // namespace Digikam
