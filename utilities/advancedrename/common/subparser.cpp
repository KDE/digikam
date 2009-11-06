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

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "firstletterofeachworduppercasemodifier.h"
#include "lowercasemodifier.h"
#include "rangemodifier.h"
#include "replacemodifier.h"
#include "trimmedmodifier.h"
#include "uppercasemodifier.h"

namespace Digikam
{

class SubParserPriv
{
public:

    SubParserPriv() :
        useModifiers(true)
    {}

    bool         useModifiers;

    ParseResults parseResults;
    ParseResults modifierResults;
    ModifierList modifiers;
};

SubParser::SubParser(const QString& name, const QString& description, const QIcon& icon)
         : ParseObject(name, icon), d(new SubParserPriv)
{
    setDescription(description);

    registerModifier(new LowerCaseModifier());
    registerModifier(new UpperCaseModifier());
    registerModifier(new FirstLetterEachWordUpperCaseModifier());
    registerModifier(new TrimmedModifier());
    registerModifier(new RangeModifier());
    registerModifier(new ReplaceModifier());
}

SubParser::~SubParser()
{
    foreach (Modifier* modifier, d->modifiers)
    {
        delete modifier;
    }

    d->modifiers.clear();

    delete d;
}

void SubParser::registerModifier(Modifier* modifier)
{
    if (!modifier)
    {
        return;
    }

    if (!modifier->isValid())
    {
        return;
    }

    d->modifiers.append(modifier);
}

void SubParser::setUseModifiers(bool value)
{
    d->useModifiers = value;
}

bool SubParser::useModifiers() const
{
    return d->useModifiers;
}

ModifierList SubParser::modifiers() const
{
    return d->modifiers;
}

void SubParser::parse(const QString& parseString, ParseInformation& info)
{
    d->parseResults.clear();
    d->modifierResults.clear();

    parseOperation(parseString, info, d->parseResults);

    if (d->useModifiers)
    {
        d->modifierResults = applyModifiers(parseString, d->parseResults);
    }
    else
    {
        d->modifierResults.clear();
    }
}

ParseResults SubParser::parseResults()
{
    return d->parseResults;
}

ParseResults SubParser::modifiedResults()
{
    if (d->modifierResults.isEmpty() || !d->useModifiers)
    {
        return d->parseResults;
    }
    return d->modifierResults;
}

ParseResults SubParser::applyModifiers(const QString& parseString, ParseResults& results)
{
    ParseResults tmp = results;

    ParseResults modifiers;
    QMap<ParseResults::ResultsKey, Modifier*> modifierCallbackMap;

    if (results.isEmpty())
    {
        return tmp;
    }

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

    foreach (const ParseResults::ResultsKey& key, results.keys())
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

} // namespace Digikam
