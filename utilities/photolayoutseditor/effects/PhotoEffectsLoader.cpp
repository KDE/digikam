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

#include "PhotoEffectsLoader.h"
#include "PhotoEffectsGroup.h"
#include "AbstractPhoto.h"
#include "UndoCommandEvent.h"
#include "KEditFactory.h"
#include "global.h"
#include "AbstractPhotoEffectFactory.h"
#include "AbstractPhotoEffectInterface.h"
#include "PhotoEffectChangeListener.h"

#include "qtpropertybrowser.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

#include <QApplication>
#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

PhotoEffectsLoader * PhotoEffectsLoader::m_instance = 0;
QMap<QString, AbstractPhotoEffectFactory*> PhotoEffectsLoader::registeredEffects;

PhotoEffectsLoader::PhotoEffectsLoader(QObject * parent) :
    QObject(parent),
    m_effect(0)
{
}

PhotoEffectsLoader * PhotoEffectsLoader::instance(QObject * parent)
{
    if (m_instance)
    {
        if (parent)
            m_instance->setParent(parent);
        return m_instance;
    }
    return (m_instance = new PhotoEffectsLoader(parent));
}

PhotoEffectsGroup * PhotoEffectsLoader::group() const
{
    return qobject_cast<PhotoEffectsGroup*>(this->parent());
}

AbstractPhoto * PhotoEffectsLoader::photo() const
{
    PhotoEffectsGroup * tempGroup = this->group();
    if (tempGroup)
        return tempGroup->photo();
    else
        return 0;
}

bool PhotoEffectsLoader::registerEffect(AbstractPhotoEffectFactory * effectFactory)
{
    QString effectName = effectFactory->effectName();
    QStringList names = effectName.split(QLatin1Char(';'), QString::SkipEmptyParts);
    bool result = true;
    foreach(QString name, names)
        result &= (registeredEffects.insert(name, effectFactory) != registeredEffects.end());
    effectFactory->setParent(instance());
    return result;
}

QStringList PhotoEffectsLoader::registeredEffectsNames()
{
    return registeredEffects.keys();
}

AbstractPhotoEffectFactory * PhotoEffectsLoader::getFactoryByName(const QString & name)
{
    return registeredEffects.value(name, 0);
}

AbstractPhotoEffectInterface * PhotoEffectsLoader::getEffectByName(const QString & effectName)
{
    AbstractPhotoEffectFactory * factory = PhotoEffectsLoader::registeredEffects[effectName];
    if (factory)
        return factory->getEffectInstance(effectName);
    return 0;
}

QtAbstractPropertyBrowser * PhotoEffectsLoader::propertyBrowser(AbstractPhotoEffectInterface * effect, bool createCommands)
{
    if (!effect)
        return 0;

    QtAbstractPropertyBrowser * browser = new QtTreePropertyBrowser();
    PhotoEffectChangeListener * listener = new PhotoEffectChangeListener(effect, browser, createCommands);

    // QVariant::Int
    QtIntPropertyManager * integerManager = new QtIntPropertyManager(browser);
    KSliderEditFactory * integerFactory = new KSliderEditFactory(browser);
    browser->setFactoryForManager(integerManager, integerFactory);

    // Double type of property
    QtDoublePropertyManager * doubleManager = 0;
    KDoubleSpinBoxFactory * doubleFactory = 0;

    // QVariant others....
    QtVariantPropertyManager * variantManager = 0;
    KVariantEditorFactory * variantFactory = 0;

    const QMetaObject * meta = effect->metaObject();
    int propertiesCount = meta->propertyCount();
    for (int i = 0; i < propertiesCount; ++i)
    {
        QMetaProperty metaProperty = meta->property(i);
        QString propertyName = effect->propertyName(metaProperty);
        if (propertyName.isEmpty())
            continue;
        QtProperty * property = 0;
        switch(metaProperty.type())
        {
            case QVariant::Int:
                {
                    if (!integerManager || !integerFactory)
                    {
                        integerManager = new QtIntPropertyManager(browser);
                        integerFactory = new KSliderEditFactory(browser);
                        browser->setFactoryForManager(integerManager, integerFactory);
                    }
                    property = integerManager->addProperty(propertyName);
                    integerManager->setValue(property, metaProperty.read(effect).toInt());
                    integerManager->setMinimum(property, effect->minimumValue(metaProperty).toInt());
                    integerManager->setMaximum(property, effect->maximumValue(metaProperty).toInt());
                    integerManager->setSingleStep(property, effect->stepValue(metaProperty).toInt());
                }
                break;
            case QVariant::Double:
                {
                    if (!doubleManager || !doubleFactory)
                    {
                        doubleManager = new QtDoublePropertyManager(browser);
                        doubleFactory = new KDoubleSpinBoxFactory(browser);
                        browser->setFactoryForManager(doubleManager, doubleFactory);
                    }
                    property = doubleManager->addProperty(propertyName);
                    doubleManager->setValue(property, metaProperty.read(effect).toDouble());
                    doubleManager->setMinimum(property, effect->minimumValue(metaProperty).toDouble());
                    doubleManager->setMaximum(property, effect->maximumValue(metaProperty).toDouble());
                    integerManager->setSingleStep(property, effect->maximumValue(metaProperty).toDouble());
                }
                break;
            default:
                {
                    if (!variantManager || !variantFactory)
                    {
                        variantManager = new QtVariantPropertyManager(browser);
                        variantFactory = new KVariantEditorFactory(browser);
                        browser->setFactoryForManager(variantManager, variantFactory);
                    }
                    property = variantManager->addProperty(metaProperty.type(), propertyName);
                    variantManager->setValue(property, metaProperty.read(effect));
                    foreach(QtProperty* p, property->subProperties())
                        p->setEnabled(false);
                }
        }
        if (property)
            browser->addProperty(property);
    }
    connect(integerManager,SIGNAL(propertyChanged(QtProperty*)),listener,SLOT(propertyChanged(QtProperty*)));
    connect(integerFactory,SIGNAL(editingFinished()),listener,SLOT(editingFinished()));
    if (doubleManager && doubleFactory)
    {
        connect(doubleManager,SIGNAL(propertyChanged(QtProperty*)),listener,SLOT(propertyChanged(QtProperty*)));
        connect(doubleFactory,SIGNAL(editingFinished()),listener,SLOT(editingFinished()));
    }
    if (variantManager && variantFactory)
    {
        connect(variantManager,SIGNAL(propertyChanged(QtProperty*)),listener,SLOT(propertyChanged(QtProperty*)));
        connect(variantFactory,SIGNAL(editingFinished()),listener,SLOT(editingFinished()));
    }

    return browser;
}

QDomElement PhotoEffectsLoader::effectToSvg(AbstractPhotoEffectInterface * effect, QDomDocument & document)
{
    QDomElement element = document.createElement(QLatin1String("effect"));
    element.setAttribute(QLatin1String("name"), effect->name());
    const QMetaObject * meta = effect->metaObject();
    int count = meta->propertyCount();
    for (int i = 0; i < count; ++i)
    {
        QMetaProperty p = meta->property(i);
        element.setAttribute( QLatin1String(p.name()), QLatin1String(p.read(effect).toByteArray().toBase64()) );
    }
    return element;
}

AbstractPhotoEffectInterface * PhotoEffectsLoader::getEffectFromSvg(QDomElement & element)
{
    if ( element.tagName() != QLatin1String("effect"))
        return 0;
    QMap<QString,QString> properties;
    QDomNamedNodeMap attributes = element.attributes();
    for (int j = attributes.count()-1; j >= 0; --j)
    {
        QDomAttr attr = attributes.item(j).toAttr();
        if (attr.isNull())
            continue;
        properties.insert(attr.name(), attr.value());
    }
    QString effectName = properties.take(QLatin1String("name"));
    if ( !instance()->registeredEffectsNames().contains( effectName ) )
        return 0;
    AbstractPhotoEffectInterface * result = instance()->getEffectByName( effectName );
    const QMetaObject * meta = result->metaObject();
    int count = meta->propertyCount();
    for (int i = 0; i < count; ++i)
    {
        QMetaProperty p = meta->property(i);
        QString value = properties.take(QLatin1String(p.name()));
        if (value.isEmpty())
            continue;
        p.write(result, QVariant(QByteArray::fromBase64(value.toLatin1())));
    }
    return result;
}
