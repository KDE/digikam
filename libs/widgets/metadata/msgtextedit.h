/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-07-15
 * @brief  a text edit widget with click message.
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

#ifndef MSGTEXTEDIT_H
#define MSGTEXTEDIT_H

// Qt includes

#include <QtCore/QString>
#include <QtWidgets/QWidget>

// KDE includes

#include <ktextedit.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MsgTextEdit : public KTextEdit
{
    Q_OBJECT

public:

    MsgTextEdit(QWidget* const parent);
    ~MsgTextEdit();

    void    setClickMessage(const QString& msg);
    QString clickMessage() const;

    void setText(const QString& txt);

protected:

    void paintEvent(QPaintEvent*);
    void dropEvent(QDropEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* MSGTEXTEDIT_H */
