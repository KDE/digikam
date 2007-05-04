/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : a tool tip widget witch follow cursor movements 
 *               Tool tip content is displayed without delay.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DCURSOR_TRACKER_H
#define DCURSOR_TRACKER_H   

// Qt includes.

#include <qlabel.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam 
{

/**
 * This class implements a decoration-less window which will follow the cursor
 * when it's over a specified widget.
 */
class DIGIKAM_EXPORT DCursorTracker : public QLabel 
{

public:

	DCursorTracker(const QString& txt, QWidget *parent);

	void setText(const QString& txt);
    void setEnable(bool b); 
    
protected:

	bool eventFilter(QObject*, QEvent*);

private:

    bool m_enable;
};


/**
 * A specialized CursorTracker class, which looks like a tool tip.
 */
class DTipTracker : public DCursorTracker 
{

public:

	DTipTracker(const QString& txt, QWidget *parent);
};

} // namespace Digikam

#endif /* DCURSOR_TRACKER_H */
