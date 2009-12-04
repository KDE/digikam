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

// Qt includes

#include <QPixmap>

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

class ParseObjectPriv;

class ParseObject : public QObject
{
    Q_OBJECT

public:

    ParseObject(const QString& name);
    ParseObject(const QString& name, const QPixmap& icon);
    virtual ~ParseObject();

    QRegExp regExp() const;
    void    setRegExp(const QRegExp& regExp);

    QString description() const;
    void    setDescription(const QString& desc);

    QPixmap icon() const;
    void    setIcon(const QPixmap& pixmap);

    /**
     * @return a list of all registered tokens
     */
    TokenList tokens() const;

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
     * If multiple tokens have been assigned to a parseobject, a menu will be created.
     * If you want to display a menu for every defined token, set this method to 'true' and
     * re-implement the @see slotTokenTriggered method.
     * @param value boolean parameter to set token menu usage
     */
    void setUseTokenMenu(bool value);

    /**
     * Checks the validity of the parse object
     * @return true if valid
     */
    bool isValid() const;

    /**
     * Resets the parser to its initial state. This method also resets the internal counter that is used
     * for sequence numbers.
     */
    virtual void reset();

Q_SIGNALS:

    void signalTokenTriggered(const QString&);

protected:

    /**
     * add a token to the parser, every parser should at least assign one token object
     * @param id the token id string (used for parsing)
     * @param description the description of the token (used for example in the tooltip)
     * @param actionName[optional] the name of the token action (only used when the token menu is displayed)
     * @return
     */
    bool addToken(const QString& id, const QString& description, const QString& actionName = QString());

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
