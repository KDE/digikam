/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-23
 * Description : a widget to select between system font or a custom font.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DFONTSELECT_H
#define DFONTSELECT_H

// Qt includes

#include <QFont>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFontSelect : public DHBox
{
    Q_OBJECT

public:

    enum FontMode
    {
        SystemFont=0,
        CustomFont
    };

public:

    explicit DFontSelect(const QString& text, QWidget* const parent=0);
    virtual ~DFontSelect();

    void setMode(FontMode mode);
    FontMode mode() const;

    QFont font() const;
    void setFont(const QFont& font);

Q_SIGNALS:

    void signalFontChanged();

protected:

    bool event(QEvent* e);

private Q_SLOTS:

    void slotOpenFontDialog();
    void slotChangeMode(int index);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DFONTSELECT_H
