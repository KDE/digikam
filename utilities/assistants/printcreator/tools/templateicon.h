/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-26
 * Description : a tool to print images
 *
 * Copyright (C) 2008      by Andreas Trink <atrink at nociaro dot org>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TEMPLATE_ICON_H
#define TEMPLATE_ICON_H

// Qt incudes

#include <QPainter>
#include <QIcon>
#include <QColor>
#include <QSize>

namespace Digikam
{

class TemplateIcon
{

public:

    /**
     * Constructor: The height of the icon is <iconHeight>. The width is computed
     * during invocation of method 'begin()' according to the paper-size.
     */
    explicit TemplateIcon(int iconHeight, const QSize& templateSize);
    ~TemplateIcon();

    /**
     * Begin painting the icon
     */
    void begin();

    /**
     * End painting the icon
     */
    void end();

    /**
     * Returns a pointer to the icon.
     */
    QIcon& getIcon() const;

    /**
     * Returns the size of the icon.
     */
    QSize& getSize();

    /**
     * Returns the painter.
     */
    QPainter& getPainter() const;

    /**
     * Draw a filled rectangle with color <color> at position <x>/<y> (relative
     * to template-origin) and width <w> and height <h>.
     */
    void fillRect(int x, int y, int w, int h, const QColor& color);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TEMPLATE_ICON_H
