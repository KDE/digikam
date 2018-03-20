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

#include "atkinspagelayoutnode.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>

namespace Digikam
{

AtkinsPageLayoutNode::AtkinsPageLayoutNode(double aspectRatio,
                                           double relativeArea,
                                           int    index)
    : m_a(aspectRatio),
      m_e(relativeArea),
      m_division(0),
      m_type(TerminalNode),
      m_index(index),
      m_leftChild(0),
      m_rightChild(0)
{
}

AtkinsPageLayoutNode::AtkinsPageLayoutNode(AtkinsPageLayoutNode* const subtree,
                                           AtkinsPageLayoutNode* const terminalChild,
                                           bool horizontal,
                                           int  index)
    : m_a(0),
      m_e(0),
      m_division(0),
      m_type(horizontal ? HorizontalDivision : VerticalDivision),
      m_index(index),
      m_leftChild(subtree),
      m_rightChild(terminalChild)
{
}

AtkinsPageLayoutNode::AtkinsPageLayoutNode(const AtkinsPageLayoutNode& other)
{
    (*this) = other;
}

AtkinsPageLayoutNode::~AtkinsPageLayoutNode()
{
    delete m_leftChild;
    delete m_rightChild;
}

AtkinsPageLayoutNode &AtkinsPageLayoutNode::operator=(const AtkinsPageLayoutNode& other)
{
    m_a          = other.m_a;
    m_e          = other.m_e;
    m_division   = other.m_division;
    m_type       = other.m_type;
    m_index      = other.m_index;
    m_leftChild  = other.m_leftChild  ? new AtkinsPageLayoutNode(*other.m_leftChild)  : 0;
    m_rightChild = other.m_rightChild ? new AtkinsPageLayoutNode(*other.m_rightChild) : 0;

    return *this;
}

void AtkinsPageLayoutNode::takeAndSetChild(AtkinsPageLayoutNode* const oldChild,
                                           AtkinsPageLayoutNode* const newChild)
{
    if (m_leftChild == oldChild)
    {
        m_leftChild = newChild;
    }
    else if (m_rightChild == oldChild)
    {
        m_rightChild = newChild;
    }
}

AtkinsPageLayoutNode* AtkinsPageLayoutNode::nodeForIndex(int index)
{
    if (m_index == index)
        return this;

    if (m_type == TerminalNode)
        return 0;

    AtkinsPageLayoutNode* const fromLeft = m_leftChild->nodeForIndex(index);

    if (fromLeft)
        return fromLeft;

    return m_rightChild->nodeForIndex(index);
}

AtkinsPageLayoutNode* AtkinsPageLayoutNode::parentOf(AtkinsPageLayoutNode* const child)
{
    if (m_type == TerminalNode)
        return 0;

    if (m_leftChild == child || m_rightChild == child)
        return this;

    AtkinsPageLayoutNode* const fromLeft = m_leftChild->parentOf(child);

    if (fromLeft)
        return fromLeft;

    return m_rightChild->parentOf(child);
}

void AtkinsPageLayoutNode::computeRelativeSizes()
{
    if (m_type == TerminalNode)
        return;

    m_leftChild->computeRelativeSizes();
    m_rightChild->computeRelativeSizes();

    double leftProductRoot   = std::sqrt(m_leftChild->m_a  * m_leftChild->m_e);
    double rightProductRoot  = std::sqrt(m_rightChild->m_a * m_rightChild->m_e);
    double maxProductRoot    = leftProductRoot > rightProductRoot ? leftProductRoot : rightProductRoot;

    double leftDivisionRoot  = std::sqrt(m_leftChild->m_e  / m_leftChild->m_a);
    double rightDivisionRoot = std::sqrt(m_rightChild->m_e / m_rightChild->m_a);
    double maxDivisionRoot   = leftDivisionRoot > rightDivisionRoot ? leftDivisionRoot : rightDivisionRoot;

    if (m_type == VerticalDivision)        // side by side
    {
        m_a = maxProductRoot / (leftDivisionRoot + rightDivisionRoot);
        m_e = maxProductRoot * (leftDivisionRoot + rightDivisionRoot);
    }
    else if (m_type == HorizontalDivision) // one on top of the other
    {
        m_a = (leftProductRoot + rightProductRoot) / maxDivisionRoot;
        m_e = maxDivisionRoot  * (leftProductRoot  + rightProductRoot);
    }
}

void AtkinsPageLayoutNode::computeDivisions()
{
    if (m_type == TerminalNode)
        return;

    m_leftChild->computeDivisions();
    m_rightChild->computeDivisions();

    if (m_type == VerticalDivision)        // side by side
    {
        double leftDivisionRoot  = std::sqrt(m_leftChild->m_e  / m_leftChild->m_a);
        double rightDivisionRoot = std::sqrt(m_rightChild->m_e / m_rightChild->m_a);
 
        m_division               = leftDivisionRoot / (leftDivisionRoot + rightDivisionRoot);
    }
    else if (m_type == HorizontalDivision) // one on top of the other
    {
        // left child is topmost
        double leftProductRoot  = std::sqrt(m_leftChild->m_a  * m_leftChild->m_e);
        double rightProductRoot = std::sqrt(m_rightChild->m_a * m_rightChild->m_e);

        // the term in the paper takes 0 = bottom, we use 0 = top
        m_division              = 1 - (rightProductRoot / (rightProductRoot + leftProductRoot));
    }
}

double AtkinsPageLayoutNode::aspectRatio() const
{
    return m_a;
}

double AtkinsPageLayoutNode::relativeArea() const
{
    return m_e;
}

double AtkinsPageLayoutNode::division() const
{
    return m_division;
}

AtkinsPageLayoutNode::Type AtkinsPageLayoutNode::type() const
{
    return m_type;
}

int AtkinsPageLayoutNode::index() const
{
    return m_index;
}

AtkinsPageLayoutNode* AtkinsPageLayoutNode::leftChild() const
{
    return m_leftChild;
}

AtkinsPageLayoutNode* AtkinsPageLayoutNode::rightChild() const
{
    return m_rightChild;
}

} // Namespace Digikam
