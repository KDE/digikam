/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for the AdvancedRename utility
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

#include "parser.h"

// Qt includes

#include <QFileInfo>

namespace Digikam
{

class ParserPriv
{
public:

    ParserPriv() :
        counter(0)
    {}

    ParseSettings globalSettings;
    OptionsList   options;
    ModifierList  modifiers;
    int           counter;
};

// --------------------------------------------------------

Parser::Parser()
      : d(new ParserPriv)
{
    init(ParseSettings());
}

Parser::~Parser()
{
    foreach (Option* option, d->options)
    {
        delete option;
    }
    d->options.clear();

    foreach (Modifier* modifier, d->modifiers)
    {
        delete modifier;
    }
    d->modifiers.clear();

    delete d;
}

void Parser::init(const ParseSettings& settings)
{
    d->globalSettings = settings;
    d->counter  = 1;
}

void Parser::reset()
{
    init(ParseSettings());
    foreach (Option* option, d->options)
    {
        option->reset();
    }
    foreach (Modifier* modifier, d->modifiers)
    {
        modifier->reset();
    }
}

bool Parser::parseStringIsValid(const QString& str)
{
    QRegExp invalidString("^\\s*$");
    return (!str.isEmpty() && !invalidString.exactMatch(str));
}

OptionsList Parser::options() const
{
    return d->options;
}

ModifierList Parser::modifiers() const
{
    return d->modifiers;
}

void Parser::registerOption(Option* option)
{
    if (!option || !option->isValid())
    {
        return;
    }

    d->options.append(option);
}

void Parser::registerModifier(Modifier* modifier)
{
    if (!modifier || !modifier->isValid())
    {
        return;
    }

    d->modifiers.append(modifier);
}

ParseResults Parser::results(ParseSettings& settings)
{
    parse(settings);
    return settings.results;
}

ParseResults Parser::invalidModifiers(ParseSettings& settings)
{
    parse(settings);
    return settings.invalidModifiers;
}

QString Parser::parse(ParseSettings& settings)
{
    QFileInfo fi(settings.fileUrl.toLocalFile());

    if (!parseStringIsValid(settings.parseString))
    {
        return fi.fileName();
    }

    ParseResults results;

    settings.startIndex   = d->globalSettings.startIndex;
    settings.currentIndex = d->counter++;

    foreach (Option* option, d->options)
    {
        ParseResults r = option->parse(settings);
        results.append(r);
    }

    QString newName;
    if (settings.applyModifiers)
    {
        settings.invalidModifiers = applyModifiers(settings.parseString, results);
        newName                   = results.replaceTokens(settings.parseString);
    }
    settings.results = results;

    // remove invalid modifiers from the new name
    foreach (const Modifier* mod, d->modifiers)
    {
        newName.remove(mod->regExp());
    }

    if (newName.isEmpty())
    {
        return fi.fileName();
    }

    if (settings.useOriginalFileExtension)
    {
        newName.append('.').append(fi.suffix());
    }

    return newName;
}

bool Parser::tokenAtPosition(TokenType type, ParseSettings& settings, int pos)
{
    int start;
    int length;
    return tokenAtPosition(type, settings, pos, start, length);
}

bool Parser::tokenAtPosition(TokenType type, ParseSettings& settings, int pos, int& start, int& length)
{
    bool found = false;

    ParseResults r;

    switch (type)
    {
        case OptionToken:
            settings.applyModifiers = false;
            r = results(settings);
            break;
        case OptionModifiersToken:
            settings.applyModifiers = true;
            r = results(settings);
            break;
        default:
            break;
    }

    ParseResults::ResultsKey key = r.keyAtApproximatePosition(pos);
    start  = key.first;
    length = key.second;

    if ((pos >= start) && (pos <= start + length))
    {
        found = true;
    }
    return found;
}

ParseResults Parser::applyModifiers(const QString& parseString, ParseResults& results)
{
    if (results.isEmpty() || d->modifiers.isEmpty())
    {
        return ParseResults();
    }

    ParseSettings settings;
    settings.results = results;

    // appliedModifiers holds all the modified parse results
    ParseResults appliedModifiers = results;

    // modifierResults holds all the modifiers found in the parse string
    ParseResults modifierResults;

    // modifierMap maps the actual modifier objects to the entries in the modifierResults structure
    QMap<ParseResults::ResultsKey, Modifier*> modifierMap;

    foreach (Modifier* modifier, d->modifiers)
    {
        QRegExp regExp = modifier->regExp();
        int pos = 0;
        while (pos > -1)
        {
            pos = regExp.indexIn(parseString, pos);
            if (pos > -1)
            {
                ParseResults::ResultsKey   k(pos, regExp.matchedLength());
                ParseResults::ResultsValue v(regExp.cap(0), QString());

                modifierResults.addEntry(k, v);
                modifierMap.insert(k, modifier);

                pos += regExp.matchedLength();
            }
        }
    }

    // Check for valid modifiers (they must appear directly after an option) and apply the modification to the option
    // parse result.
    // We need to create a second ParseResults object with modified keys, otherwise the final parsing step will not
    // remove the modifier tokens from the result.

    foreach (const ParseResults::ResultsKey& key, results.keys())
    {
        int off  = results.offset(key);
        int diff = 0;
        for (int pos = off; pos < parseString.count();)
        {
            if (modifierResults.hasKeyAtPosition(pos))
            {
                ParseResults::ResultsKey mkey = modifierResults.keyAtPosition(pos);
                Modifier* mod                 = modifierMap[mkey];
                QString modToken              = modifierResults.token(mkey);

                QString token                 = results.token(key);
                QString str2Modify            = results.result(key);

                QString modResult;
                if (mod)
                {
                    settings.parseString       = modToken;
                    settings.currentResultsKey = key;
                    modResult = mod->modify(settings, str2Modify);
                }

                // update result
                ParseResults::ResultsKey   kResult = key;
                ParseResults::ResultsValue vResult(token, modResult);
                results.addEntry(kResult, vResult);

                // update modifier map
                ParseResults::ResultsKey kModifier = key;
                kModifier.second += diff;
                ParseResults::ResultsValue vModifier(modToken, modResult);

                appliedModifiers.deleteEntry(kModifier);
                kModifier.second += modToken.count();
                appliedModifiers.addEntry(kModifier, vModifier);

                // set position to the next possible token
                pos  += mkey.second;
                diff += mkey.second;

                // remove assigned modifier from modifierResults
                modifierResults.deleteEntry(mkey);
            }
            else
            {
                break;
            }
        }
    }
    results = appliedModifiers;

    return modifierResults;
}

}  // namespace Digikam
