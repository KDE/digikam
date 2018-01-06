/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Layouting photos on a page
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ATKINS_PAGE_LAYOUT_TREE_H
#define ATKINS_PAGE_LAYOUT_TREE_H

// Qt includes

#include <QRectF>
#include <QMap>

/**
    Implements the algorithm described in

    "Adaptive Photo Collection Page Layout",
    C. Brian Atkins
    Imaging Technology Department
    HP Labs
    Palo Alto, CA 94304
    cbatkins@hpl.hp.com

    PDF available at:
    http://hpl.hp.com/research/isl/layout/

    Comments in the source file refer to the PDF file.
*/

/**
    The classes AtkinsPageLayoutNode and AtkinsPageLayoutTree provide the actual implementation.
    Do not use these classes directly.
    Use the AtkinsPageLayout adaptor class.
*/

namespace Digikam
{

class AtkinsPageLayoutNode;

class AtkinsPageLayoutTree
{
public:

    AtkinsPageLayoutTree(double aspectRatioPage,
                         double absoluteAreaPage);
    AtkinsPageLayoutTree(const AtkinsPageLayoutTree&);
    ~AtkinsPageLayoutTree();

    int    addImage(double aspectRatio,
                    double relativeArea);
    QRectF drawingArea(int index,
                       const QRectF& absoluteRectPage);

    int    count() const;
    double score(AtkinsPageLayoutNode* const root,
                 int nodeCount);
    double G() const;

    AtkinsPageLayoutTree& operator=(const AtkinsPageLayoutTree& other);

private:

    double absoluteArea(AtkinsPageLayoutNode* const node);

    /** Lays out a rectangle with given aspect ratio and absolute area inside the given
     *  larger rectangle (not in the paper).
     */
    QRectF rectInRect(const QRectF& rect,
                      double aspectRatio,
                      double absoluteArea);

private:

    AtkinsPageLayoutNode* m_root;
    int                   m_count;

    double                m_aspectRatioPage;
    double                m_absoluteAreaPage;
};

} // Namespace Digikam

#endif // ATKINS_PAGE_LAYOUT_TREE_H
