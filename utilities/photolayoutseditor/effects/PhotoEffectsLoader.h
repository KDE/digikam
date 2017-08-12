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

#ifndef PHOTOEFFECTSLOADER_H
#define PHOTOEFFECTSLOADER_H

#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QUndoCommand>
#include <QSemaphore>
#include <QMap>
#include <QList>
#include <QDomDocument>

class QtProperty;
class QtAbstractPropertyBrowser;

namespace PhotoLayoutsEditor
{
    class AbstractPhoto;
    class PhotoEffectsGroup;
    class AbstractPhotoEffectFactory;
    class AbstractPhotoEffectInterface;

    class PhotoEffectsLoader : public QObject
    {
            Q_OBJECT

            class OpacityUndoCommand;

            static QString m_effect_name;
            QString m_name;

            static QMap<QString, AbstractPhotoEffectFactory*> registeredEffects;

            static PhotoEffectsLoader * m_instance;
            explicit PhotoEffectsLoader(QObject * parent);

        public:

            static PhotoEffectsLoader * instance(QObject * parent = 0);
            PhotoEffectsGroup * group() const;
            AbstractPhoto * photo() const;

          /**
            * Name propetry
            */
            Q_PROPERTY(QString m_name READ name WRITE setName)
            virtual QString name() const
            {
                return m_name;
            }
            virtual void setName(const QString & name)
            {
                m_name = name;
            }

          /** Registers new effect using it's factory object.
            * \arg effectFactory - this object should be allocated on heap usong \fn operator new(),
            * this class takes over the parenthood of this factory and it will delete it if it'll no longer needed.
            */
            static bool registerEffect(AbstractPhotoEffectFactory * effectFactory);

          /** Returns registered effects names
            * This implementation returns \class QStringList object with effects names obtained by calling \fn effectName()
            * method of its factory object.
            */
            static QStringList registeredEffectsNames();

          /** Returns factory object for the given name
            */
            static AbstractPhotoEffectFactory * getFactoryByName(const QString & name);

          /** Return an instance of effect using its name.
            */
            static AbstractPhotoEffectInterface * getEffectByName(const QString & name);

          /** Returns property browser for effect.
            * \arg effect is the object of \class AbstractPhotoEffectInterface base type which represents effect with set of properties to configure.
            * \arg does browser should create undo commands or just set properties values.
            */
            static QtAbstractPropertyBrowser * propertyBrowser(AbstractPhotoEffectInterface * effect, bool createCommands);

          /** Returns DOM node which contains effects attributes
            */
            static QDomElement effectToSvg(AbstractPhotoEffectInterface * effect, QDomDocument & document);

          /** Reads node attributes from DOM node and returns ready effect object.
            */
            static AbstractPhotoEffectInterface * getEffectFromSvg(QDomElement & element);

        protected:

            AbstractPhotoEffectInterface * m_effect;

        friend class AbstractPhotoEffectFactory;
    };
}

#endif // PHOTOEFFECTSLOADER_H
