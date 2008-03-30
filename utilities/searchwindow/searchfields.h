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

#ifndef SEARCHFIELDS_H
#define SEARCHFIELDS_H

// Qt includes

#include <QWidget>
#include <QMap>

// KDE includes

// Local includes

#include "searchxml.h"
#include "searchutilities.h"

class QLabel;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QTimeEdit;
class QTreeView;
class KLineEdit;
class KSqueezedTextLabel;

namespace Digikam
{

class SearchFieldGroup;
class SqueezedComboBox;
class SearchSqueezedClickLabel;
class KDateEdit;

class SearchField : public QObject, public VisibilityObject
{
    Q_OBJECT

public:

    static SearchField *createField(const QString &fieldName, SearchFieldGroup *parent);

    SearchField(QObject *parent);
    void setup(QGridLayout *layout, int row = -1);
    void setFieldName(const QString &fieldName);
    virtual void setText(const QString &label, const QString &detailLabel);

    virtual bool supportsField(const QString &fieldName);
    virtual void read(SearchXmlReader &reader) = 0;
    virtual void write(SearchXmlWriter &writer) = 0;
    virtual void reset() = 0;

    virtual void setVisible(bool visible);
    virtual bool isVisible();

protected:

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column) = 0;
    virtual void setupLabels(QGridLayout *layout, int line);

    virtual void setValueWidgetsVisible(bool visible) = 0;

    QString m_name;

    QLabel *m_label;
    QLabel *m_detailLabel;

    bool    m_categoryLabelVisible;
};

class SearchFieldText : public SearchField
{
public:

    SearchFieldText(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void setValueWidgetsVisible(bool visible);
    virtual void reset();

protected:

    KLineEdit *m_edit;
};

class SearchFieldRangeInt : public SearchField
{
    Q_OBJECT

public:

    SearchFieldRangeInt(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);

    void setBetweenText(const QString &text);
    void setNoValueText(const QString &text);
    void setNumberPrefixAndSuffix(const QString &prefix, const QString &suffix);
    void setBoundary(int min, int max, int step = 1);

protected slots:

    void valueChanged();

protected:

    int             m_min;
    int             m_max;
    QSpinBox       *m_firstBox;
    QSpinBox       *m_secondBox;
    QLabel         *m_betweenLabel;
};

class SearchFieldRangeDouble : public SearchField
{
    Q_OBJECT

public:

    SearchFieldRangeDouble(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);

    void setBetweenText(const QString &text);
    void setNoValueText(const QString &text);
    void setNumberPrefixAndSuffix(const QString &prefix, const QString &suffix);
    void setBoundary(double min, double max, int decimals, double step);
    void setFactor(double factor);

protected slots:

    void valueChanged();

protected:

    double          m_min;
    double          m_max;
    double          m_factor;
    QDoubleSpinBox *m_firstBox;
    QDoubleSpinBox *m_secondBox;
    QLabel         *m_betweenLabel;
};

class SearchFieldRangeDate : public SearchField
{
public:

    enum Type
    {
        DateOnly,
        DateTime
    };

    SearchFieldRangeDate(QObject *parent, Type type);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);

    void setBetweenText(const QString &between);
    void setBoundary(QDateTime min, QDateTime max);

protected:

    QTimeEdit *m_firstTimeEdit;
    KDateEdit *m_firstDateEdit;
    QTimeEdit *m_secondTimeEdit;
    KDateEdit *m_secondDateEdit;
    QLabel    *m_betweenLabel;

    Type           m_type;
};

class SearchFieldChoice : public SearchField
{
    Q_OBJECT

public:

    SearchFieldChoice(SearchFieldGroup *parent);

    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();

    void setChoice(const QMap<int, QString> &map);
    void setAnyText(const QString &string);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void setValueWidgetsVisible(bool visible);

protected slots:

    void slotClicked();
    void slotUpdateLabel();

protected:

    void setValues(const QList<int> &values);
    void setValues(int value, SearchXml::Relation relation);

    QList<int> values() const;
    QString valueText() const;

    virtual void setupChoiceWidgets();

protected:

    QString                    m_anyText;
    SearchSqueezedClickLabel  *m_label;
    QVBoxLayout               *m_vbox;
    QMap<int, QString>         m_choiceMap;
    QMap<QCheckBox*, int>      m_widgetMap;
    VisibilityController      *m_controller;
};

class Album;
class AbstractCheckableAlbumModel;
class SearchFieldAlbum : public SearchField
{
    Q_OBJECT

public:

    enum Type
    {
        TypeAlbum,
        TypeTag
    };

    SearchFieldAlbum(QObject *parent, Type type);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);

protected slots:

    void checkStateChanged(Album *album, int state);

protected:

    void updateComboText();

    TreeViewComboBox            *m_comboBox;
    QLineEdit                   *m_comboLineEdit;
    Type                         m_type;
    QString                      m_anyText;
    AbstractCheckableAlbumModel *m_model;
};

class SearchFieldColorDepth : public SearchField
{
public:

    SearchFieldColorDepth(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);

protected:

    QComboBox      *m_comboBox;
};

}

#endif


