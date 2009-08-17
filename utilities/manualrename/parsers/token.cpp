/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a token class
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

#include "token.h"

// Qt includes

#include <QAction>

namespace Digikam
{
namespace ManualRename
{

Token::Token(const QString& id, const QString& alias, const QString& description)
     : QObject(0)
 {
    m_id          = id;
    m_alias       = alias;
    m_description = description;
    m_action      = new QAction(m_alias, this);

    connect(m_action, SIGNAL(triggered()),
            this, SLOT(slotTriggered()));
 }

Token::~Token()
{
    delete m_action;
};

void Token::slotTriggered()
{
    emit signalTokenTriggered(m_id);
}

} // namespace ManualRename
} // namespace Digikam
