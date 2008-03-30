/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 * 
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QVBoxLayout>

// KDE includes

#include <klocale.h>

// Local includes

#include "ddebug.h"
#include "searchxml.h"
#include "themeengine.h"
#include "searchwindow.h"
#include "searchgroup.h"
#include "searchview.h"
#include "searchview.moc"

namespace Digikam
{


SearchView::SearchView()
{
}

void SearchView::setup()
{
    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(setTheme()));
    setTheme();

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // create initial group
    addSearchGroup();

    setLayout(m_layout);
}

void SearchView::read(const QString &xml)
{
    SearchXmlReader reader(xml);

    int groupIndex = 0;
    SearchXml::Element element;
    while (!reader.atEnd())
    {
        element = reader.readNext();
        if (element == SearchXml::Group)
        {
            SearchGroup *group = 0;
            if (groupIndex >= m_groups.size())
                group = addSearchGroup();
            else
                group = m_groups[groupIndex];

            group->read(reader);

            groupIndex++;
        }
    }

    // remove superfluous groups
    while (m_groups.size() > (groupIndex+1))
        delete m_groups.takeLast();

    // for empty searches, reset the remaining search group
    if (!groupIndex)
        m_groups.first()->reset();
}

SearchGroup *SearchView::addSearchGroup()
{
    SearchGroup *group = new SearchGroup(this);
    group->setup();
    m_layout->addWidget(group);
    m_groups << group;
    if (m_groups.size() > 1)
        group->setChainSearchGroup();
    //group->setBackgroundRole(QPalette::Background);
    return group;
}

QString SearchView::write()
{
    SearchXmlWriter writer;
    foreach (SearchGroup *group, m_groups)
        group->write(writer);
    writer.finish();
    return writer.xml();
}

void SearchView::setTheme()
{
    // settings with style sheet results in extremely slow painting
    setBackgroundRole(QPalette::Base);

    QFont f = font();
    QString fontSizeLarger;
    if (f.pointSizeF() == -1)
    {
        // set pixel size
        fontSizeLarger = QString::number(f.pixelSize() + 2) + "px";
    }
    else
    {
        fontSizeLarger = QString::number(f.pointSizeF() + 2) + "pt";
    }

    QString sheet =
//             ".SearchView { background-color: " + ThemeEngine::instance()->baseColor().name() + "; } "
            "#SearchGroupLabel_MainLabel "
            " { font-weight: bold; font-size: "
              + fontSizeLarger + ";"
            "   color: "
              + ThemeEngine::instance()->textSelColor().name() + ";"
            " } "
            "#SearchGroupLabel_CheckBox "
            " { color: "
              + ThemeEngine::instance()->textSelColor().name() + ";"
            " } "
            "#SearchFieldGroupLabel_Label "
            " { color: "
              + ThemeEngine::instance()->textSpecialRegColor().name() + ";"
            "   font-weight: bold; "
            " } "
            "#SearchField_MainLabel "
            " { font-weight: bold; } "
            "#SearchFieldChoice_ClickLabel "
            " { color: "
              + ThemeEngine::instance()->textSpecialRegColor().name() + ";"
            "   font-style: italic; "
            "   text-decoration: underline; "
            " } ";
    QWidget::setStyleSheet(sheet);
}

QPixmap SearchView::groupLabelPixmap(int w, int h)
{
    if (m_cachedGroupLabelPixmap.isNull()
        || m_cachedGroupLabelPixmap.width() != w
        || m_cachedGroupLabelPixmap.height() != h)
    {
        m_cachedGroupLabelPixmap = ThemeEngine::instance()->bannerPixmap(w, h);
    }
    return m_cachedGroupLabelPixmap;
}

}



