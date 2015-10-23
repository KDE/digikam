/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-07-15
 * @brief  a text edit widget.
 *
 * @author Copyright (C) 2009-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "msgtextedit.h"

// Qt includes

#include <QPalette>
#include <QPainter>

namespace Digikam
{

MsgTextEdit::MsgTextEdit(QWidget* const parent)
    : KTextEdit(parent)
{
    setAcceptRichText(false);
}

MsgTextEdit::~MsgTextEdit()
{
}

}  // namespace Digikam
