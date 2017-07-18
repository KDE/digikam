/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Layouting photos on a page
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADV_PRINT_LAYOUT_TREE_H
#define ADV_PRINT_LAYOUT_TREE_H

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
    The classes AdvPrintLayoutNode and AdvPrintLayoutTree provide the actual implementation.
    Do not use these classes directly.
    Use the AtkinsPageLayout adaptor class.
*/

namespace Digikam
{

class AdvPrintLayoutNode
{
public:

    enum Type
    {
        TerminalNode,
        HorizontalDivision, // one image on top of the other
        VerticalDivision    // images side by side
    };

public:

    AdvPrintLayoutNode(double aspectRatio,
                       double relativeArea,
                       int index);
    AdvPrintLayoutNode(AdvPrintLayoutNode* const subtree,
                       AdvPrintLayoutNode* const terminalChild,
                       bool horizontal,
                       int index);
    AdvPrintLayoutNode(const AdvPrintLayoutNode&);
    ~AdvPrintLayoutNode();

    double aspectRatio()  const;
    double relativeArea() const;
    double division()     const;
    Type   type()         const;
    int    index()        const;

    AdvPrintLayoutNode* leftChild()  const;
    AdvPrintLayoutNode* rightChild() const;

    /** Replace one child with a new one.
     */
    void takeAndSetChild(AdvPrintLayoutNode* const oldChild,
                         AdvPrintLayoutNode* const newChild);

    /** Retrieve the node which has the given index in the hierarchy of this node.
     */
    AdvPrintLayoutNode* nodeForIndex(int index);
    
    /** Retrieve the parent node of the given child in the hierarchy of this node.
     */
    AdvPrintLayoutNode* parentOf(AdvPrintLayoutNode* const child);

    /** Compute the "aspect ratio" (m_a) and "relative size" (m_e) parameters.
     *  Section 2.2.1, (1)-(4).
     */
    void computeRelativeSizes();

    /** Compute the "division" (m_division) parameter.
     *  Section 2.2.2
     */
    void computeDivisions();

    AdvPrintLayoutNode& operator=(const AdvPrintLayoutNode&);

private:

    double              m_a;        // aspect ration
    double              m_e;        // relative size
    double              m_division;

    Type                m_type;

    int                 m_index;

    AdvPrintLayoutNode* m_leftChild;
    AdvPrintLayoutNode* m_rightChild;
};

// ---------------------------------------------------------------------

class AdvPrintLayoutTree
{
public:

    AdvPrintLayoutTree(double aspectRatioPage, double absoluteAreaPage);
    AdvPrintLayoutTree(const AdvPrintLayoutTree&);
    ~AdvPrintLayoutTree();

    int    addImage(double aspectRatio, double relativeArea);
    QRectF drawingArea(int index, const QRectF& absoluteRectPage);

    int    count() const;
    double score(AdvPrintLayoutNode* const root, int nodeCount);
    double G() const;

    AdvPrintLayoutTree& operator=(const AdvPrintLayoutTree& other);

private:

    double absoluteArea(AdvPrintLayoutNode* const node);
    
    /** Lays out a rectangle with given aspect ratio and absolute area inside the given larger rectangle
     *  (not in the paper).
     */
    QRectF rectInRect(const QRectF& rect, double aspectRatio, double absoluteArea);

private:

    AdvPrintLayoutNode* m_root;
    int                 m_count;

    double              m_aspectRatioPage;
    double              m_absoluteAreaPage;
};

} // Namespace Digikam

#endif // ADV_PRINT_LAYOUT_TREE_H
