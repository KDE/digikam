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

#include "parser.h"
#include "parser.moc"

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

namespace Digikam
{
namespace ManualRename
{

Parser::Parser(const QString& name, const QIcon& icon)
      : QObject(0)
{
    m_name              = name;
    m_icon              = icon;
    m_buttonRegistered  = false;
    m_MenuRegistered    = false;
    m_useTokenMenu      = true;
}

Parser::~Parser()
{
    foreach (Token* token, m_tokens)
    {
        delete token;
    }

    m_tokens.clear();
}

QPushButton* Parser::createButton(const QString& name, const QIcon& icon)
{
    const int maxHeight = 28;

    QPushButton* button = new QPushButton;
    button->setText(name);
    button->setIcon(icon);
    button->setMinimumHeight(maxHeight);
    button->setMaximumHeight(maxHeight);

    return button;
}

QPushButton* Parser::registerButton(QWidget* parent)
{
    QPushButton* button = 0;
    if (!m_buttonRegistered)
    {
        button = createButton(m_name, m_icon);

        QList<QAction*> actions;

        if (m_tokens.count() > 1 && m_useTokenMenu)
        {
            QMenu* menu = new QMenu(button);

            foreach (Token* token, m_tokens)
            {
                actions << token->action();
            }

            menu->addActions(actions);
            button->setMenu(menu);
        }
        else
        {
            Token* token = m_tokens.first();
            connect(button, SIGNAL(clicked()),
                    token, SLOT(slotTriggered()));

        }
        button->setParent(parent);

        m_buttonRegistered = button ? true : false;
    }
    return button;
}

QAction* Parser::registerMenu(QMenu* parent)
{
    QAction* action = 0;
    if (!m_MenuRegistered)
    {
        QList<QAction*> actions;

        if (m_tokens.count() > 1 && m_useTokenMenu)
        {
            QMenu* menu = new QMenu(parent);

            foreach (Token* token, m_tokens)
            {
                actions << token->action();
            }

            menu->addActions(actions);
            action = parent->addMenu(menu);
        }
        else
        {
            action = m_tokens.first()->action();
            parent->insertAction(0, action);
        }

        if (action)
        {
            action->setText(m_name);
            action->setIcon(m_icon);
            m_MenuRegistered = true;
        }
    }

    return action;
}

bool Parser::addToken(const QString& id, const QString& name, const QString& description)
{
    if (id.isEmpty() || name.isEmpty() || description.isEmpty())
        return false;

    Token* token = new Token(id, name, description);
    if (!token)
        return false;

    connect(token, SIGNAL(signalTokenTriggered(const QString&)),
            this, SLOT(slotTokenTriggered(const QString&)));

    m_tokens << token;
    return true;
}

void Parser::useTokenMenu(bool value)
{
    m_useTokenMenu = value;
}

QList<Token*> Parser::tokens() const
{
    return m_tokens;
}

void Parser::slotTokenTriggered(const QString& token)
{
    emit signalTokenTriggered(token);
}

bool Parser::stringIsValid(const QString& str)
{
    QRegExp invalidString("^\\s*$");
    if (str.isEmpty() || invalidString.exactMatch(str))
        return false;
    return true;
}

QString Parser::markResult(int length, const QString& result)
{
    QString tmp;
    if (result.isEmpty())
        tmp = emptyTokenMarker();
    else
        tmp = result;

    return resultsMarker().arg(length).arg(tmp);
}

void Parser::generateMarkerTemplate(QChar& left, QChar& right, int& width)
{
    QString marker("3{}");

    width = QString(marker.at(0)).toInt();
    left  = marker.at(1);
    right = marker.at(2);
}

QString Parser::resultsMarker()
{
    int width = 0;
    QChar left, right;
    generateMarkerTemplate(left, right, width);

    QString marker = QString("%1%3:%4%2").arg(left,  width, left)
                                         .arg(right, width, right);
    return marker;
}

QString Parser::resultsExtractor()
{
    int width = 0;
    QChar left, right;
    generateMarkerTemplate(left, right, width);

    QString marker;
    for (int i = 0; i < width; ++i)
        marker.append("\\").append(left);

    marker.append("(\\d+):(.*)");
    for (int i = 0; i < width; ++i)
        marker.append("\\").append(right);

    return marker;
};

QString Parser::emptyTokenMarker()
{
    return QString("!!!EMPTY!!!");
}


void Parser::parse(QString& parseString, const ParseInformation& info)
{
    if (!stringIsValid(parseString))
        return;

    parseOperation(parseString, info);
}

} // namespace ManualRename
} // namespace Digikam
