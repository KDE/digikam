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

#ifndef RULE_H
#define RULE_H

// Local includes

#include "parseresults.h"
#include "parsesettings.h"
#include "token.h"

class QAction;
class QMenu;
class QPushButton;
class QString;

namespace Digikam
{

class Rule : public QObject
{
    Q_OBJECT

public:

    enum IconType
    {
        Action = 0,
        Dialog
    };

public:

    explicit Rule(const QString& name);
    Rule(const QString& name, const QString& icon);
    virtual ~Rule();

    /**
     * returns the currently assigned regExp object. Note that it is returned as a const ref, meaning
     * that if you use it in your custom parse operation, the main parse method has already searched for the pattern
     * and filled in the results of this search, so that you can use QRegExp::cap() immediately, you don't have to search
     * on your own.
     *
     * For example when implementing the Option::parseOperation() method, get the regExp object with
     *
     *      const QRegExp& reg = regExp();
     *
     * and immediately fetch possible matches with
     *
     *      const QString& param1 = reg.cap(1);
     *
     * @see Option
     * @see Modifier
     *
     * @return a const ref to the assigned regexp object
     */
    QRegExp& regExp() const;

    QString description() const;
    QPixmap icon(Rule::IconType type = Rule::Action) const;

    /**
     * @return a list of all registered tokens
     */
    TokenList& tokens() const;

    /**
     * Register a button in the parent object. By calling this method, a new button for the parser
     * object will be created and all necessary connections will be setup.

     * @param  parent the parent object the button will be registered for
     * @return a pointer to the newly created button
     */
    QPushButton* registerButton(QWidget* parent);

    /**
     * Register a menu action in the parent object. By calling this method, a new action for the parser
     * object will be created and all necessary connections will be setup.
     *
     * @param  parent the parent object the action will be registered for
     * @return a pointer to the newly created action
     */
    QAction* registerMenu(QMenu* parent);

    /**
     * Returns true if a token menu is used.
     */
    bool useTokenMenu() const;

    /**
     * Checks the validity of the parse object
     * @return true if valid
     */
    bool isValid() const;

    /**
     * Resets the parser to its initial state
     */
    virtual void reset();

    /**
     * Escape the token characters to make them work in regular expressions
     *
     * @param token token to be escaped
     * @return A token with escaped characters. This token can then be used in a regular expression
     */
    static QString escapeToken(const QString& token);

    ParseResults parse(ParseSettings& settings);

Q_SIGNALS:

    void signalTokenTriggered(const QString&);

protected:

    /**
     * TODO: describe me
     * @param settings
     * @return
     */
    virtual QString parseOperation(ParseSettings& settings) = 0;

    /**
     * add a token to the parser, every parser should at least assign one token object
     * @param id the token id string (used for parsing)
     * @param description the description of the token (used for example in the tooltip)
     * @param actionName [optional] the name of the token action (only used when the token menu is displayed)
     * @return
     */
    bool addToken(const QString& id, const QString& description, const QString& actionName = QString());

    void setRegExp(const QRegExp& regExp);
    void setDescription(const QString& desc);
    void setIcon(const QString& pixmap);

    /**
     * If multiple tokens have been assigned to a rule, a menu will be created.
     * If you want to display a menu for every defined token, set this method to 'true' and
     * re-implement the @see slotTokenTriggered method.
     * @param value boolean parameter to set token menu usage
     */
    void setUseTokenMenu(bool value);

protected Q_SLOTS:

    virtual void slotTokenTriggered(const QString&);

private:

    Rule(const Rule&);
    Rule& operator=(const Rule&);

    QPushButton* createButton(const QString& name, const QIcon& icon);

private:

    class Private;
    Private* const d;
};

typedef QList<Rule*> RulesList;

} // namespace Digikam

#endif /* RULE_H */
