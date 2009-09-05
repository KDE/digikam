/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract parser class
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

#ifndef PARSER_H
#define PARSER_H

// Qt includes

#include <QDateTime>
#include <QIcon>
#include <QList>
#include <QString>

// KDE includes

#include <kdialog.h>

// Local includes

#include "token.h"

class QAction;
class QChar;
class QMenu;
class QPushButton;
class QWidget;

namespace Digikam
{
namespace ManualRename
{

class ParseInformation
{
public:

    ParseInformation():
        filePath(QString()),
        cameraName(QString()),
        datetime(QDateTime()),
        index(1)
        {};

    QString   filePath;
    QString   cameraName;
    QDateTime datetime;
    int       index;
};

class Parser : public QObject
{
    Q_OBJECT

public:

    Parser(const QString& name, const QIcon& icon);
    virtual ~Parser();

    /**
     * @return a list of all registered tokens
     */
    QList<Token*>  tokens() const;

    /**
     * Register a button in the parent object. By calling this method, a new button for the parser
     * object will be created and all necessary connections will be setup. A button can only be registered once.
     * @see registerMenu
     *
     * @param parent the parent object the button will be registered for
     * @return a pointer to the newly created button
     */
    QPushButton*   registerButton(QWidget* parent);

    /**
     * Register a menu action in the parent object. By calling this method, a new action for the parser
     * object will be created and all necessary connections will be setup. An action can only be registered once.
     * @see registerButton
     *
     * @param parent the parent object the action will be registered for
     * @return a pointer to the newly created action
     */
    QAction*       registerMenu(QMenu* parent);

    /**
     * check if the given parse string is valid
     * @param str the parse string
     * @return true if valid / can be parsed
     */
    static bool    stringIsValid(const QString& str);

    static void    generateMarkerTemplate(QChar& left, QChar& right, int& width);
    static QString resultsMarker();
    static QString resultsExtractor();
    static QString emptyTokenMarker();

Q_SIGNALS:

    void signalTokenTriggered(const QString&);

public Q_SLOTS:

    virtual void parse(QString& parseString, const ParseInformation& info) = 0;

protected:

    /**
     * add a token to the parser, every parser should at least assign one token object
     * @param id the token id string
     * @param alias an alias for the token
     * @param description the description of the token
     * @return
     */
    bool    addToken(const QString& id, const QString& alias, const QString& description);

    /**
     * If multiple tokens have been assigned to a parser, a menu will be created.
     * If you do not want a menu for every defined token, set this method to 'false' and
     * re-implement the @see slotTokenTriggered method.
     * @param value boolean parameter to set token menu usage
     */
    void    useTokenMenu(bool value);

    /**
     * Marks the replaced token results in the parsed string.
     * Since token replacements can again contain a token character, the replacement needs to be marked so
     * that the main parser can extract those results and save them in a list for later usage.
     *
     * Example:
     *
     * filename:
     * my_file#001.jpg
     *
     * renaming string:
     * new_###_$
     *
     * The above example will be parsed as:
     * new_001_my_file#001
     *
     * In a next parser step the '#' token will again be replaced by a number, although not wanted:
     * new_001_my_file1001
     *
     * To avoid this, we mark every result (every token we replace in the @see parse method):
     * new_{{{001}}}_{{{my_file#001}}}
     *
     * The main parser will extract these markers, save it in a list for later usage and replace the parse string with:
     * new_index:0_index:1
     * or any string that was defined in @see ManualRenameParser::tokenMarker()
     *
     * When all parsers have been called, the main parser will replace those special markers with the saved results:
     * new_001_my_file#001
     *
     * This ensures that token characters can exist in replacement strings, without being parsed again, which in some cases
     * might even lead to an infinite parse loop.
     *
     * @param result the result token to be marked
     * @return
     */
    QString markResult(int length, const QString& result);

protected Q_SLOTS:

    virtual void slotTokenTriggered(const QString&);

private:

    QPushButton*  createButton(const QString& name, const QIcon& icon);

    bool          m_buttonRegistered;
    bool          m_MenuRegistered;
    bool          m_useTokenMenu;
    QString       m_name;
    QIcon         m_icon;
    QList<Token*> m_tokens;
};

} // namespace ManualRename
} // namespace Digikam

#endif /* PARSER_H */
