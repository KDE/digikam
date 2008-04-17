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

#include <QPainter>
#include <QVBoxLayout>

// KDE includes

#include <kdialogbuttonbox.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>

// Local includes

#include "ddebug.h"
#include "searchxml.h"
#include "themeengine.h"
#include "searchwindow.h"
#include "searchgroup.h"
#include "searchutilities.h"
#include "searchview.h"
#include "searchview.moc"

namespace Digikam
{


SearchView::SearchView()
{
    m_pixmapCache.setMaxCost(4);
}

void SearchView::setup()
{
    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(setTheme()));
    setTheme();

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_bar = new SearchViewBottomBar(this);
    // add stretch at bottom
    m_layout->addStretch(1);
    // add bottom bar
    m_layout->addWidget(m_bar);

    // create initial group
    addSearchGroup();

    setLayout(m_layout);

    connect(m_bar, SIGNAL(okPressed()),
            this, SIGNAL(searchOk()));

    connect(m_bar, SIGNAL(cancelPressed()),
            this, SIGNAL(searchCancel()));

    connect(m_bar, SIGNAL(tryoutPressed()),
            this, SIGNAL(searchTryout()));

    connect(m_bar, SIGNAL(addGroupPressed()),
            this, SLOT(slotAddGroupButton()));
}

void SearchView::read(const QString &xml)
{
    SearchXmlCachingReader reader(xml);

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
    // insert at last-but-two position; leave bottom bar and stretch and the bottom
    m_layout->insertWidget(m_layout->count()-2, group);
    m_groups << group;
    if (m_groups.size() > 1)
        group->setChainSearchGroup();
    //group->setBackgroundRole(QPalette::Background);
    return group;
}

void SearchView::slotAddGroupButton()
{
    addSearchGroup();
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
            // ".SearchView { background-color: " + ThemeEngine::instance()->baseColor().name() + "; } "
            //"#SearchViewBottomBar "
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

QPixmap SearchView::cachedBannerPixmap(int w, int h)
{
    QString key = "BannerPixmap-" + QString::number(w) + "-" + QString::number(h);
    QPixmap *pix = m_pixmapCache.object(key);
    if (!pix)
    {
        QPixmap pixmap = ThemeEngine::instance()->bannerPixmap(w, h);
        m_pixmapCache.insert(key, new QPixmap(pixmap));
        return pixmap;
    }
    else
    {
        return *pix;
    }
}

QPixmap SearchView::groupLabelPixmap(int w, int h)
{
    return cachedBannerPixmap(w, h);
}

QPixmap SearchView::bottomBarPixmap(int w, int h)
{
    return cachedBannerPixmap(w, h);
}

// ------------------------------------- //

SearchViewBottomBar::SearchViewBottomBar(SearchViewThemedPartsCache * cache, QWidget *parent)
    : QWidget(parent),
      m_themeCache(cache)
{
    m_mainLayout = new QHBoxLayout;

    m_addGroupsButton = new KPushButton(KStandardGuiItem::add());
    m_addGroupsButton->setText(i18n("Add Search Group"));
    connect(m_addGroupsButton, SIGNAL(leftClicked()),
            this, SIGNAL(addGroupPressed()));
    m_mainLayout->addWidget(m_addGroupsButton, 0, Qt::AlignLeft);

    m_buttonBox = new KDialogButtonBox(this);
    m_buttonBox->addButton(KStandardGuiItem::ok(),
                           QDialogButtonBox::AcceptRole,
                           this,
                           SIGNAL(okPressed()));
    m_buttonBox->addButton(KStandardGuiItem::cancel(),
                           QDialogButtonBox::RejectRole,
                           this,
                           SIGNAL(cancelPressed()));
    m_buttonBox->addButton(KStandardGuiItem::test(),
                           QDialogButtonBox::ApplyRole,
                           this,
                           SIGNAL(tryoutPressed()));
    m_mainLayout->addWidget(m_buttonBox, 0, Qt::AlignRight);

    setLayout(m_mainLayout);
}

void SearchViewBottomBar::paintEvent(QPaintEvent *)
{
    // paint themed background
    QPainter p(this);
    p.drawPixmap(0, 0, m_themeCache->bottomBarPixmap(width(), height()));
}

}



