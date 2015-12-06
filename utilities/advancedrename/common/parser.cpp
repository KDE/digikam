/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for the AdvancedRename utility
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

#include "parser.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "cameranameoption.h"
#include "databaseoption.h"
#include "dateoption.h"
#include "directorynameoption.h"
#include "filepropertiesoption.h"
#include "metadataoption.h"
#include "sequencenumberoption.h"
#include "casemodifier.h"
#include "defaultvaluemodifier.h"
#include "rangemodifier.h"
#include "removedoublesmodifier.h"
#include "replacemodifier.h"
#include "trimmedmodifier.h"
#include "uniquemodifier.h"

namespace Digikam
{

class Parser::Private
{
public:

    Private()
    {}

    RulesList options;
    RulesList modifiers;
};

// --------------------------------------------------------

Parser::Parser()
    : d(new Private)
{
    registerOption(new FilePropertiesOption());
    registerOption(new DirectoryNameOption());
    registerOption(new CameraNameOption());
    registerOption(new SequenceNumberOption());
    registerOption(new DateOption());
    registerOption(new DatabaseOption());
    registerOption(new MetadataOption());

    // --------------------------------------------------------

    registerModifier(new CaseModifier());
    registerModifier(new TrimmedModifier());
    registerModifier(new UniqueModifier());
    registerModifier(new RemoveDoublesModifier());
    registerModifier(new DefaultValueModifier());
    registerModifier(new ReplaceModifier());
    registerModifier(new RangeModifier());
}

Parser::~Parser()
{
    qDeleteAll(d->options);
    d->options.clear();

    qDeleteAll(d->modifiers);
    d->modifiers.clear();

    delete d;
}

void Parser::reset()
{
    foreach(Rule* option, d->options)
    {
        option->reset();
    }
    foreach(Rule* modifier, d->modifiers)
    {
        modifier->reset();
    }
}

bool Parser::parseStringIsValid(const QString& str)
{
    QRegExp invalidString(QLatin1String("^\\s*$"));
    return (!str.isEmpty() && !invalidString.exactMatch(str));
}

RulesList Parser::options() const
{
    return d->options;
}

RulesList Parser::modifiers() const
{
    return d->modifiers;
}

void Parser::registerOption(Rule* option)
{
    if (!option || !option->isValid())
    {
        return;
    }

    d->options.append(option);
}

void Parser::unregisterOption(Rule* option)
{
    if (!option)
    {
        return;
    }

    for (RulesList::iterator it = d->options.begin();
         it != d->options.end();)
    {
        if (*it == option)
        {
            delete *it;
            it = d->options.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Parser::registerModifier(Rule* modifier)
{
    if (!modifier || !modifier->isValid())
    {
        return;
    }

    d->modifiers.append(modifier);
}

void Parser::unregisterModifier(Rule* modifier)
{
    if (!modifier)
    {
        return;
    }

    for (RulesList::iterator it = d->modifiers.begin();
         it != d->modifiers.end();)
    {
        if (*it == modifier)
        {
            delete *it;
            it = d->modifiers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

ParseResults Parser::results(ParseSettings& settings)
{
    ParseResults results;

    foreach(Rule* const option, d->options)
    {
        ParseResults r = option->parse(settings);
        results.append(r);
    }

    foreach(Rule* const modifier, d->modifiers)
    {
        ParseResults r = modifier->parse(settings);
        results.append(r);
    }

    return results;
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

    foreach(Rule* const option, d->options)
    {
        ParseResults r = option->parse(settings);
        results.append(r);
    }

    settings.invalidModifiers = applyModifiers(settings.parseString, results);
    QString newName           = results.replaceTokens(settings.parseString);
    settings.results          = results;

    // remove invalid modifiers from the new name
    foreach(Rule* const mod, d->modifiers)
    {
        newName.remove(mod->regExp());
    }

    if (newName.isEmpty())
    {
        return fi.fileName();
    }

    if (settings.useOriginalFileExtension)
    {
        newName.append(QLatin1Char('.')).append(fi.suffix());
    }

    return newName;
}

bool Parser::tokenAtPosition(ParseSettings& settings, int pos)
{
    int start;
    int length;
    return tokenAtPosition(settings, pos, start, length);
}

bool Parser::tokenAtPosition(ParseSettings& settings, int pos, int& start, int& length)
{
    bool found                   = false;
    ParseResults r               = results(settings);
    ParseResults::ResultsKey key = r.keyAtApproximatePosition(pos);
    start                        = key.first;
    length                       = key.second;

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
    QMap<ParseResults::ResultsKey, Rule*> modifierMap;

    foreach(Rule* const modifier, d->modifiers)
    {
        QRegExp regExp = modifier->regExp();
        int pos        = 0;

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

    foreach(const ParseResults::ResultsKey& key, results.keys())
    {
        int off  = results.offset(key);
        int diff = 0;

        for (int pos = off; pos < parseString.count();)
        {
            if (modifierResults.hasKeyAtPosition(pos))
            {
                ParseResults::ResultsKey mkey = modifierResults.keyAtPosition(pos);
                Rule* const mod               = modifierMap[mkey];
                QString modToken              = modifierResults.token(mkey);

                QString token                 = results.token(key);
                QString str2Modify            = results.result(key);

                QString modResult;

                if (mod)
                {
                    settings.parseString       = modToken;
                    settings.currentResultsKey = key;
                    settings.str2Modify        = str2Modify;
                    ParseResults modResults    = mod->parse(settings);

                    if (!modResults.isEmpty() && modResults.values().length() == 1)
                    {
                        modResult = modResults.result(modResults.keys().first());
                    }
                }

                // update result
                ParseResults::ResultsKey   resultKey = key;
                ParseResults::ResultsValue resultValue(token, modResult);
                results.addEntry(resultKey, resultValue);

                // update modifier map
                ParseResults::ResultsKey modifierKey = key;
                modifierKey.second += diff;
                ParseResults::ResultsValue modifierValue(modToken, modResult);

                appliedModifiers.deleteEntry(modifierKey);
                modifierKey.second += modToken.count();
                appliedModifiers.addEntry(modifierKey, modifierValue);

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
