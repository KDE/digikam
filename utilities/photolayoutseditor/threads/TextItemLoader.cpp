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

#include "TextItemLoader.h"
#include "TextItem.h"
#include "ProgressObserver.h"

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

TextItemLoader::TextItemLoader(TextItem * item, QDomElement & element, QObject * parent) :
    AbstractPhotoItemLoader(item, element, parent)
{
}

void TextItemLoader::run()
{
    QDomElement e                    = this->element();
    TextItem* const item             = static_cast<TextItem*>(this->item());
    ProgressObserver* const observer = this->observer();
    AbstractPhotoItemLoader::run();

    QDomElement defs = e.firstChildElement(QLatin1String("defs"));
    while (!defs.isNull() && defs.attribute(QLatin1String("class")) != QLatin1String("data"))
        defs = defs.nextSiblingElement(QLatin1String("defs"));
    if (defs.isNull())
        this->exit(1);

    QDomElement data = defs.firstChildElement(QLatin1String("data"));
    if (data.isNull())
        this->exit(1);

    // text
    if (observer)
    {
        observer->progresChanged(0.5);
        observer->progresName(i18n("Reading text..."));
    }
    QDomElement text = data.firstChildElement(QLatin1String("text"));
    if (text.isNull())
        this->exit(1);
    QDomNode textValue = text.firstChild();
    while (!textValue.isNull() && !textValue.isText())
        textValue = textValue.nextSibling();
    if (textValue.isNull())
        this->exit(1);
    item->d->m_string_list = textValue.toText().data().remove(QLatin1Char('\t')).split(QLatin1Char('\n'));

    // Color
    if (observer)
    {
        observer->progresChanged(0.7);
        observer->progresName(i18n("Reading color..."));
    }
    QDomElement color = data.firstChildElement(QLatin1String("color"));
    if (color.isNull())
        this->exit(1);
    item->m_color = QColor(color.attribute(QLatin1String("name")));

    // Font
    if (observer)
    {
        observer->progresChanged(0.9);
        observer->progresName(i18n("Reading fonts..."));
    }
    QDomElement font = data.firstChildElement(QLatin1String("font"));
    if (font.isNull())
        this->exit(1);
    item->m_font.fromString(font.attribute(QLatin1String("data")));

    if (observer)
    {
        observer->progresChanged(1);
        observer->progresName(i18n("Finishing..."));
    }

    this->exit(0);
}
