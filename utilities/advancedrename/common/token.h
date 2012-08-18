/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a token class
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

#ifndef TOKEN_H
#define TOKEN_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>

class QAction;

namespace Digikam
{

/**
 * @brief %Token is the smallest parsing unit in AdvancedRename utility
 *
 * The %Token class represents the smallest parsing unit for the Parser class. Every string you enter as a renaming pattern
 * is a combination of tokens and literal text. For example
 * @code
 * "[file]{upper}_###_abc.[ext]{lower}"
 * @endcode
 * is composed of five tokens
 * @code
 * [file]
 * {upper}
 * ###
 * .[ext]
 * {lower}
 * @endcode
 * and two literals
 * @code
 * _
 * _abc
 * @endcode
 * A rule must assign at least one token object, to make parsing work. More than one token can be assigned to a %Rule.
 * @see Rule::addToken()
 *
 */
class Token : public QObject
{
    Q_OBJECT

public:

    Token(const QString& id, const QString& description);
    ~Token();

    /**
     * @return The ID of the token. This is the actual token string, for example
     * @code
     * "[file]"
     * @endcode
     * This id will be emitted as a signal by slotTriggered().
     */
    QString  id()
    {
        return m_id;
    };

    /**
     * @return The description of the token. It can be used for example in the tooltip of the AdvancedRenameWidget.
     */
    QString  description()
    {
        return m_description;
    };

    /**
     * @return The action of the token. This action can be connected to a button or menu item. If triggered, high-level classes
     * like AdvancedRenameWidget can connect to the signal and display the emitted text in the line edit input widget.
     */
    QAction* action()
    {
        return m_action;
    };

Q_SIGNALS:

    /**
     * This signal is emitted when the action of the token is triggered.
     */
    void signalTokenTriggered(const QString&);

private Q_SLOTS:

    /**
     * This slot will emit signalTokenTriggered() when the action of the token is triggered.
     */
    void slotTriggered();

private:

    Token(const Token&);
    Token& operator=(const Token&);

private:

    QString  m_id;
    QString  m_description;
    QAction* m_action;
};

typedef QList<Token*> TokenList;

} // namespace Digikam

#endif /* TOKEN_H */
