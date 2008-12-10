/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : tool tip widget for iconview or thumbbar items
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DITEMTOOLTIP_H
#define DITEMTOOLTIP_H

#define MAXSTRINGLENGHT 30

// Qt includes.

#include <QFrame>
#include <QString>
#include <QEvent>
#include <QResizeEvent>
#include <QTextDocument>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "themeengine.h"
#include "digikam_export.h"

namespace Digikam
{

class DItemToolTipPriv;

class DIGIKAM_EXPORT DToolTipContainer
{
public:

    DToolTipContainer()
    {
        unavailable = i18n("unavailable");

        tipHeader   = QString("<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">");
        tipFooter   = QString("</table>");

        headBeg     = QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"
                              "<nobr><font size=\"-1\" color=\"%2\"><b>")
                              .arg(ThemeEngine::instance()->baseColor().name())
                              .arg(ThemeEngine::instance()->textRegColor().name());
        headEnd     = QString("</b></font></nobr></td></tr>");

        cellBeg     = QString("<tr><td><nobr><font size=\"-1\" color=\"%1\">")
                              .arg(ThemeEngine::instance()->textRegColor().name());
        cellMid     = QString("</font></nobr></td><td><nobr><font size=\"-1\" color=\"%1\">")
                              .arg(ThemeEngine::instance()->textRegColor().name());
        cellEnd     = QString("</font></nobr></td></tr>");

        cellSpecBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"%1\">")
                              .arg(ThemeEngine::instance()->textRegColor().name());
        cellSpecMid = QString("</font></nobr></td><td><nobr><font size=\"-1\" color=\"%1\"><i>")
                              .arg(ThemeEngine::instance()->textSpecialRegColor().name());
        cellSpecEnd = QString("</i></font></nobr></td></tr>");
    };

    QString breakString(const QString& input)
    {
        QString str      = input.simplified();
        str              = Qt::escape(str);
        const int maxLen = MAXSTRINGLENGHT;

        if (str.length() <= maxLen)
            return str;

        QString br;

        int i     = 0;
        int count = 0;

        while (i < str.length())
        {
            if (count >= maxLen && str[i].isSpace())
            {
                count = 0;
                br.append("<br/>");
            }
            else
            {
                br.append(str[i]);
            }

            i++;
            count++;
        }
        return br;
    };

    QString unavailable;
    QString tipHeader;
    QString tipFooter;
    QString headBeg;
    QString headEnd;
    QString cellBeg;
    QString cellMid;
    QString cellEnd;
    QString cellSpecBeg;
    QString cellSpecMid;
    QString cellSpecEnd;
};

class DIGIKAM_EXPORT DItemToolTip : public QFrame
{
public:

    DItemToolTip();
    virtual ~DItemToolTip();

protected:

    bool    event(QEvent*);
    void    resizeEvent(QResizeEvent*);
    void    paintEvent(QPaintEvent*);
    void    updateToolTip();
    void    reposition();
    void    renderArrows();

    virtual QRect   repositionRect()=0;
    virtual QString tipContents()=0;

private:

    DItemToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* DITEMTOOLTIP_H */
