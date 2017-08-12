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
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "PixelizePhotoEffect.h"

// Qt includes

#include <QImage>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "qttreepropertybrowser.h"
#include "qtpropertymanager.h"
#include "KEditFactory.h"

using namespace PhotoLayoutsEditor;

class PixelizePhotoEffect::PixelizeUndoCommand : public QUndoCommand
{
        PixelizePhotoEffect* m_effect;
        int                  m_pixelSize;

    public:

        PixelizeUndoCommand(PixelizePhotoEffect* effect, int pixelSize);
        void setPixelSize(int pixelSize);

        virtual void redo();
        virtual void undo();

    private:

        void runCommand()
        {
            int temp = m_effect->pixelSize();

            if (temp != m_pixelSize)
            {
                m_effect->setPixelSize(m_pixelSize);
                m_pixelSize = temp;
            }
        }
};

// --------------------------------------------------------------------------------------------------------------

PixelizePhotoEffect::PixelizeUndoCommand::PixelizeUndoCommand(PixelizePhotoEffect* effect, int pixelSize)
    : m_effect(effect), m_pixelSize(pixelSize)
{
}

void PixelizePhotoEffect::PixelizeUndoCommand::redo()
{
    runCommand();
}

void PixelizePhotoEffect::PixelizeUndoCommand::undo()
{
    runCommand();
}

void PixelizePhotoEffect::PixelizeUndoCommand::setPixelSize(int pixelSize)
{
    m_pixelSize = pixelSize;
}

const QString PixelizePhotoEffect::PIXEL_SIZE_STRING = i18n("Pixel size");

PixelizePhotoEffect::PixelizePhotoEffect(int pixelSize, QObject* parent)
    : PhotoEffectsLoader(parent),
    m_pixelSize(pixelSize)
{
}

QString PixelizePhotoEffect::effectName() const
{
    return i18n("Pixelize effect");
}

QImage PixelizePhotoEffect::apply(const QImage & image)
{
    QImage result = image;
    QPainter p(&result);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0, 0, PhotoEffectsLoader::apply(pixelize(image, m_pixelSize)));
    return result;
}

QtAbstractPropertyBrowser* PixelizePhotoEffect::propertyBrowser() const
{
    QtAbstractPropertyBrowser* browser = PhotoEffectsLoader::propertyBrowser();
    QtIntPropertyManager* intManager   = new QtIntPropertyManager(browser);
    KSliderEditFactory* sliderFactory  = new KSliderEditFactory(browser);
    browser->setFactoryForManager(intManager, sliderFactory);

    // Radius property
    QtProperty* pixelSize = intManager->addProperty(PIXEL_SIZE_STRING);
    intManager->setMaximum(pixelSize,200);
    intManager->setMinimum(pixelSize,1);
    browser->addProperty(pixelSize);

    intManager->setValue(pixelSize,m_pixelSize);

    connect(intManager, SIGNAL(propertyChanged(QtProperty*)),
            this, SLOT(propertyChanged(QtProperty*)));

    connect(sliderFactory, SIGNAL(editingFinished()),
            this, SLOT(postEffectChangedEvent()));

    return browser;
}

QString PixelizePhotoEffect::toString() const
{
    return (i18n("Pixelize [%1 = %2]", PIXEL_SIZE_STRING, QString::number(m_pixelSize)));
}

void PixelizePhotoEffect::propertyChanged(QtProperty* property)
{
    QtIntPropertyManager* const manager = qobject_cast<QtIntPropertyManager*>(property->propertyManager());
    int pixelSize                       = m_pixelSize;

    if (property->propertyName() == PIXEL_SIZE_STRING)
    {
        pixelSize = manager->value(property);
    }
    else
    {
        PhotoEffectsLoader::propertyChanged(property);
        return;
    }

    beginUndoCommandChange();

    if (m_undo_command)
    {
        PixelizeUndoCommand const *undo = dynamic_cast<PixelizeUndoCommand*>(m_undo_command);
        undo->setPixelSize(pixelSize);
    }
    else
    {
        m_undo_command = new PixelizeUndoCommand(this,pixelSize);
    }

    endUndoCommandChange();
}
