/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 11.09.2015
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
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

#ifndef TAGGINGACTIONFACTORY_H
#define TAGGINGACTIONFACTORY_H

#include <QList>

#include "taggingaction.h"

namespace Digikam
{

class TaggingActionFactory
{
public:

    class ConstraintInterface
    {
    public:
        virtual ~ConstraintInterface() {}
        virtual bool matches(int tagId) = 0;
    };
    enum NameMatchMode
    {
        // Default: use the "startingWith" method
        MatchStartingWithFragment,
        // use the "contains" method
        MatchContainingFragment
    };

    TaggingActionFactory();
    virtual ~TaggingActionFactory();

    // Set a fragment of a tag name to generate possible tags, as known from completers
    void setFragment(const QString& fragment);
    QString fragment() const;
    // Set a tag which may by the user be intended to be the parent of a newly created tag
    void setParentTag(int parentTagId);
    int parentTagId() const;
    // Allows to filter the scope of suggested tags. Pass an implementation of ConstraintInterface (reamins in your ownership).
    // actions() will then only suggest to assign tags for which matches() is true
    void setConstraintInterface(ConstraintInterface* iface);
    ConstraintInterface* constraintInterface() const;
    // Set the matching mode for the tag name
    void setNameMatchMode(NameMatchMode mode);
    NameMatchMode nameMatchMode() const;
    // reset all settings to the default (no fragment, no actions)
    void reset();

    // Returns the sorted list of suggested tagging actions, based on the above settings
    QList<TaggingAction> actions() const;
    // Returns one single action, which is decided to be the presumedly best action based on the settings.
    TaggingAction defaultTaggingAction() const;
    // Returns the index of the default action in the list returned by generate()
    int indexOfDefaultAction() const;

    // Returns a string to be used in the UI for the given TaggingAction, interpreted in the context of the current settings
    QString suggestedUIString(const TaggingAction& action) const;

    static TaggingAction defaultTaggingAction(const QString& tagName, int parentTagId = 0);

private:

    class Private;
    Private* const d;
    Q_DISABLE_COPY(TaggingActionFactory)
};

}

#endif // TAGGINGACTIONFACTORY_H
