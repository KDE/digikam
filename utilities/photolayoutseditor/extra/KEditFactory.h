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

#ifndef KEDIT_FACTORY_H
#define KEDIT_FACTORY_H

#include <QMap>
#include <QList>

#include "qteditorfactory.h"
#include "qtvariantproperty.h"
#include "qtpropertybrowser.h"

namespace PhotoLayoutsEditor
{

class KSpinBoxFactory : public QtSpinBoxFactory
{
    Q_OBJECT

public:
    KSpinBoxFactory(QObject * parent = 0);
protected:
    QWidget * createEditor(QtIntPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KSliderFactory : public QtSliderFactory
{
    Q_OBJECT
public:
    KSliderFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtIntPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KScrollBarFactory : public QtScrollBarFactory
{
    Q_OBJECT
public:
    KScrollBarFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtIntPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KCheckBoxFactory : public QtCheckBoxFactory
{
    Q_OBJECT
public:
    KCheckBoxFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtBoolPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KDoubleSpinBoxFactory : public QtDoubleSpinBoxFactory
{
    Q_OBJECT
public:
    KDoubleSpinBoxFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtDoublePropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KLineEditFactory : public QtLineEditFactory
{
    Q_OBJECT
public:
    KLineEditFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtStringPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KDateEditFactory : public QtDateEditFactory
{
    Q_OBJECT
public:
    KDateEditFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtDatePropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KTimeEditFactory : public QtTimeEditFactory
{
    Q_OBJECT
public:
    KTimeEditFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtTimePropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KDateTimeEditFactory : public QtDateTimeEditFactory
{
    Q_OBJECT
public:
    KDateTimeEditFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtDateTimePropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KKeySequenceEditorFactory : public QtKeySequenceEditorFactory
{
    Q_OBJECT
public:
    KKeySequenceEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtKeySequencePropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KCharEditorFactory : public QtCharEditorFactory
{
    Q_OBJECT
public:
    KCharEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtCharPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KEnumEditorFactory : public QtEnumEditorFactory
{
    Q_OBJECT
public:
    KEnumEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtEnumPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KCursorEditorFactory : public QtCursorEditorFactory
{
    Q_OBJECT
public:
    KCursorEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtCursorPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KColorEditorFactory : public QtColorEditorFactory
{
    Q_OBJECT
public:
    KColorEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtColorPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KFontEditorFactory : public QtFontEditorFactory
{
    Q_OBJECT
public:
    KFontEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtFontPropertyManager * manager, QtProperty * property, QWidget * parent);
};

class KVariantEditorFactory : public QtVariantEditorFactory
{
    Q_OBJECT
public:
    KVariantEditorFactory(QObject *parent = 0);
protected:
    QWidget * createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent);
};

class KSliderEditFactory : public QtAbstractEditorFactory<QtIntPropertyManager>
{
    Q_OBJECT

public:

    explicit KSliderEditFactory(QObject * parent = 0);

protected:

    virtual void connectPropertyManager(QtIntPropertyManager * manager);
    virtual QWidget * createEditor(QtIntPropertyManager * manager, QtProperty * property, QWidget * parent);
    virtual void disconnectPropertyManager(QtIntPropertyManager *manager);

Q_SIGNALS:

    void editingFinished();

private Q_SLOTS:

    void slotEditorDestroyed(QObject *object);

private:

    QtSliderFactory * originalFactory;
    QMap<QtProperty *, QList<QWidget *> > createdEditors;
    QMap<QWidget *, QtProperty *> editorToProperty;
};

} // namespace PhotoLayoutsEditor

#endif // KEDIT_FACTORY_H
