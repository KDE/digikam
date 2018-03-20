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

#include "atkinspagelayouttree.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>

// Local includes

#include "atkinspagelayoutnode.h"

namespace Digikam
{

AtkinsPageLayoutTree::AtkinsPageLayoutTree(double aspectRatioPage,
                                           double absoluteAreaPage)
    : m_root(0),
      m_count(0),
      m_aspectRatioPage(aspectRatioPage),
      m_absoluteAreaPage(absoluteAreaPage)
{
}

AtkinsPageLayoutTree::AtkinsPageLayoutTree(const AtkinsPageLayoutTree& other)
{
    (*this) = other;
}

AtkinsPageLayoutTree& AtkinsPageLayoutTree::operator=(const AtkinsPageLayoutTree& other)
{
    delete m_root;
    m_root             = new AtkinsPageLayoutNode(*(other.m_root));
    m_count            = other.m_count;
    m_aspectRatioPage  = other.m_aspectRatioPage;
    m_absoluteAreaPage = other.m_absoluteAreaPage;

    return *this;
}

AtkinsPageLayoutTree::~AtkinsPageLayoutTree()
{
    delete m_root;
}

int AtkinsPageLayoutTree::addImage(double aspectRatio,
                                   double relativeArea)
{
    int index = m_count;

    if (!m_root)
    {
        m_root = new AtkinsPageLayoutNode(aspectRatio,
                                          relativeArea,
                                          index);
        m_count++;
        return index;
    }

    // Section 2.1
    AtkinsPageLayoutNode* bestTree = NULL;
    double highScore               = 0;

    for (int i = 0 ; i < m_count ; ++i)
    {
        for (int horizontal = 0 ; horizontal < 2 ; ++horizontal)
        {
            // create temporary tree
            AtkinsPageLayoutNode* candidateTree          = new AtkinsPageLayoutNode(*m_root);

            // select the subtree which will be replace by a new internal node
            AtkinsPageLayoutNode* const candidateSubtree = candidateTree->nodeForIndex(i);

            // find parent node
            AtkinsPageLayoutNode* const parentNode       = candidateTree->parentOf(candidateSubtree);

            // create new terminal node
            AtkinsPageLayoutNode* const newTerminalNode  = new AtkinsPageLayoutNode(aspectRatio,
                                                                                    relativeArea,
                                                                                    index);

            // create new internal node
            AtkinsPageLayoutNode* const newInternalNode  = new AtkinsPageLayoutNode(candidateSubtree,
                                                                                    newTerminalNode,
                                                                                    horizontal,
                                                                                    index+1);

            // replace in tree
            if (parentNode)
            {
                // replace in tree
                parentNode->takeAndSetChild(candidateSubtree, newInternalNode);
            }
            else
            {
                // candidateTree is candidateSubtree is root
                candidateTree = newInternalNode;
            }

            // recompute sizes
            candidateTree->computeRelativeSizes();

            double candidateScore = score(candidateTree, m_count+2);

            if (candidateScore > highScore)
            {
                highScore = candidateScore;
                delete bestTree;
                bestTree  = candidateTree;
            }
            else
            {
                delete candidateTree;
            }
        }
    }

    delete m_root;
    m_root = bestTree;

    if (m_root)
        m_root->computeDivisions();

    m_count += 2;
    return index;
}

// Section 2.2.1
double AtkinsPageLayoutTree::score(AtkinsPageLayoutNode* const root,
                                   int nodeCount)
{
    if (!root)
        return 0;

    double areaSum = 0;

    for (int i = 0 ; i < nodeCount ; ++i)
    {
        AtkinsPageLayoutNode* const node = root->nodeForIndex(i);

        if (node->type() == AtkinsPageLayoutNode::TerminalNode)
            areaSum += node->relativeArea();
    }

    double minRatioPage = root->aspectRatio() < m_aspectRatioPage ? root->aspectRatio()
                                                                  : m_aspectRatioPage;
    double maxRatioPage = root->aspectRatio() > m_aspectRatioPage ? root->aspectRatio()
                                                                  : m_aspectRatioPage;

    return G() * (areaSum / root->relativeArea()) * (minRatioPage / maxRatioPage);
}

// Section 2.2.2
double AtkinsPageLayoutTree::G() const
{
    return 0.95 * 0.95;
}

// Section 2.2.2
double AtkinsPageLayoutTree::absoluteArea(AtkinsPageLayoutNode* const node)
{
    // min(a_pbb, a_page), max(a_pbb, a_page)
    double minRatioPage     = m_root->aspectRatio() < m_aspectRatioPage ? m_root->aspectRatio()
                                                                        : m_aspectRatioPage;
    double maxRatioPage     = m_root->aspectRatio() > m_aspectRatioPage ? m_root->aspectRatio()
                                                                        : m_aspectRatioPage;

    // A_pbb
    double absoluteAreaRoot = m_absoluteAreaPage * minRatioPage / maxRatioPage;

    if (node == m_root)
        return absoluteAreaRoot;

    // A_i
    return G() * node->relativeArea() / m_root->relativeArea() * absoluteAreaRoot;
}

QRectF AtkinsPageLayoutTree::drawingArea(int index, const QRectF& absoluteRectPage)
{
    AtkinsPageLayoutNode* const node = m_root->nodeForIndex(index);

    if (!node)
        return QRectF();

    // find out the "line of ancestry" of the node
    QList<AtkinsPageLayoutNode*> treePath;
    AtkinsPageLayoutNode* parent = node;

    while (parent)
    {
        treePath.prepend(parent);
        parent = m_root->parentOf(parent);
    }

    // find out the rect of the page bounding box (the rect of the root node in the page rect)
    QRectF absoluteRect = rectInRect(absoluteRectPage,
                                     m_root->aspectRatio(),
                                     absoluteArea(m_root));

    // go along the line of ancestry and narrow down the bounding rectangle,
    // as described in section 2.2.2
    for (int i = 0 ; i < treePath.count() - 1 ; ++i)
    {
        AtkinsPageLayoutNode* const parent = treePath[i];
        AtkinsPageLayoutNode* const child  = treePath[i+1]; // only iterating to count-1

        if (parent->type() == AtkinsPageLayoutNode::VerticalDivision) // side by side
        {
            double leftWidth = absoluteRect.width() * parent->division();

            if (child == parent->leftChild())
            {
                absoluteRect.setWidth(leftWidth);
            }
            else // right child
            {
                double rightWidth = absoluteRect.width() - leftWidth;
                absoluteRect.setWidth(rightWidth);
                absoluteRect.translate(leftWidth, 0);
            }
        }
        else // horizontal division: one on top of the other
        {
            // left child is topmost
            double upperHeight = absoluteRect.height() * parent->division();

            if (child == parent->leftChild())
            {
                absoluteRect.setHeight(upperHeight);
            }
            else // right child
            {
                double lowerHeight = absoluteRect.height() - upperHeight;
                absoluteRect.setHeight(lowerHeight);
                absoluteRect.translate(0, upperHeight);
            }
        }
    }

    return rectInRect(absoluteRect, node->aspectRatio(), absoluteArea(node));
}

QRectF AtkinsPageLayoutTree::rectInRect(const QRectF &rect,
                                        double aspectRatio,
                                        double absoluteArea)
{
    double width  = std::sqrt(absoluteArea / aspectRatio);
    double height = std::sqrt(absoluteArea * aspectRatio);
    double x      = rect.x() + (rect.width()  - width)  / 2;
    double y      = rect.y() + (rect.height() - height) / 2;

    return QRectF(x, y, width, height);
}

} // Namespace Digikam
