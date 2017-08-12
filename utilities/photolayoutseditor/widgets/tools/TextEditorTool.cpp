/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "TextEditorTool.h"
#include "TextItem.h"
#include "ToolsDockWidget.h"

#include <QVBoxLayout>
#include "qtpropertybrowser.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::TextEditorToolPrivate
{
    QPushButton * m_create_button;
    QVBoxLayout * m_layout;

    friend class TextEditorTool;
};

TextEditorTool::TextEditorTool(Scene * scene, QWidget * parent) :
    AbstractItemsTool(scene, Canvas::SingleSelcting, parent),
    d(new TextEditorToolPrivate),
    m_text_item(0),
    m_created_text_item(0),
    m_browser(0),
    m_create_new_item(false)
{
    d->m_layout = new QVBoxLayout();
    d->m_create_button = new QPushButton(i18n("Create text item"), this);
    connect(d->m_create_button, SIGNAL(clicked()), this, SLOT(createNewItem()));
    d->m_layout->insertWidget(0, d->m_create_button);
    d->m_layout->insertStretch(1, 1);
    this->setLayout(d->m_layout);
}

TextEditorTool::~TextEditorTool()
{
    delete d;
}

void TextEditorTool::currentItemAboutToBeChanged()
{
    if (m_browser)
    {
        d->m_layout->removeWidget(m_browser);
        m_browser->deleteLater();
        d->m_layout->insertStretch(1, 1);
        m_browser = 0;
    }
}

void TextEditorTool::currentItemChanged()
{
    m_text_item = dynamic_cast<TextItem*>(currentItem());
    if (m_text_item)
    {
        m_browser = m_text_item->propertyBrowser();
        if (m_browser)
        {
            d->m_layout->removeItem( d->m_layout->itemAt(1) );
            d->m_layout->insertWidget(1, m_browser, 2);
        }
    }
    setEnabled(true);
}

void TextEditorTool::positionAboutToBeChanged()
{
}

void TextEditorTool::positionChanged()
{
    if (m_create_new_item)
    {
        if (!m_created_text_item || !m_created_text_item->text().join(QLatin1String("\n")).isEmpty())
            m_created_text_item = new TextItem();
        setCurrentItem( m_created_text_item );
        currentItem()->setPos( this->mousePosition() );
        emit itemCreated( currentItem() );
        m_create_new_item = false;
        d->m_create_button->setEnabled(true);
    }
}

void TextEditorTool::createNewItem()
{
    m_create_new_item = true;
    d->m_create_button->setEnabled(false);
}
