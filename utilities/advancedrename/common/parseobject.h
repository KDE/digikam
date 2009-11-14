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

#ifndef PARSEOBJECT_H
#define PARSEOBJECT_H

// Local includes

#include "parseinformation.h"
#include "parseresults.h"
#include "token.h"

class QAction;
class QIcon;
class QMenu;
class QPushButton;
class QString;

namespace Digikam
{

class ParseObjectPriv;

/*
 * Macro definitions:
 *
 * PARSE_LOOP_START and PARSE_LOOP_END can be used when parsing
 * a token with a regular expression.
 */
#define PARSE_LOOP_START(PARSESTRING_, REGEXP_)                              \
        {                                                                    \
            int POS_ = 0;                                                    \
            while (POS_ > -1)                                                \
            {                                                                \
                POS_ = REGEXP_.indexIn(PARSESTRING_, POS_);                  \
                if (POS_ > -1)                                               \
                {

#define PARSE_LOOP_END(PARSESTRING_, REGEXP_, PARSED_, RESULTS_)             \
            ParseResults::ResultsKey   k(POS_, REGEXP_.cap(0).count());      \
            ParseResults::ResultsValue v(REGEXP_.cap(0), PARSED_);           \
            RESULTS_.addEntry(k, v);                                         \
            POS_ += REGEXP_.matchedLength();                                 \
                }                                                            \
            }                                                                \
        }

class ParseObject : public QObject
{
    Q_OBJECT

public:

    ParseObject(const QString& name, const QIcon& icon);
    virtual ~ParseObject();

    QRegExp regExp() const;
    void    setRegExp(const QString& regExp);

    void    setDescription(const QString& desc);
    QString description() const;

    QIcon   icon() const;
    void    setIcon(const QIcon& icon);

    /**
     * @return a list of all registered tokens
     */
    TokenList tokens() const;

    /**
     * Register a button in the parent object. By calling this method, a new button for the parser
     * object will be created and all necessary connections will be setup. A button can only be registered once.
     * @see registerMenu
     *
     * @param parent the parent object the button will be registered for
     * @return a pointer to the newly created button
     */
    QPushButton* registerButton(QWidget* parent);

    /**
     * Register a menu action in the parent object. By calling this method, a new action for the parser
     * object will be created and all necessary connections will be setup. An action can only be registered once.
     * @see registerButton
     *
     * @param parent the parent object the action will be registered for
     * @return a pointer to the newly created action
     */
    QAction* registerMenu(QMenu* parent);

    /**
     * If multiple tokens have been assigned to a parser, a menu will be created.
     * If you do not want a menu for every defined token, set this method to 'false' and
     * re-implement the @see slotTokenTriggered method.
     * @param value boolean parameter to set token menu usage
     */
    void setUseTokenMenu(bool value);
    bool useTokenMenu() const;

    bool isValid() const;

Q_SIGNALS:

    void signalTokenTriggered(const QString&);

protected:

    /**
     * add a token to the parser, every parser should at least assign one token object
     * @param id the token id string (used for parsing)
     * @param name an alias name for the token (used for button and action text)
     * @param description the description of the token (used for example in the AdvancedRenameWidget for the tooltip)
     * @return
     */
    bool addTokenDescription(const QString& id, const QString& name, const QString& description);

protected Q_SLOTS:

    virtual void slotTokenTriggered(const QString&);

private:

    QPushButton* createButton(const QString& name, const QIcon& icon);
    bool         tokenAtPosition(ParseResults& results, int pos);
    bool         tokenAtPosition(ParseResults& results, int pos, int& start, int& length);

private:

    ParseObjectPriv* const d;
};

} // namespace Digikam

#endif /* PARSEOBJECT_H */
