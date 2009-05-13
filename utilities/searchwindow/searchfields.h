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
#include "visibilitycontroller.h"

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

class AlbumSelectComboBox;
class SearchFieldGroup;
class SqueezedComboBox;
class KDateEdit;
class ChoiceSearchModel;
class ChoiceSearchComboBox;
class RatingComboBox;

class SearchField : public QObject, public VisibilityObject
{
    Q_OBJECT

public:

    static SearchField *createField(const QString& fieldName, SearchFieldGroup *parent);

    SearchField(QObject *parent);
    void setup(QGridLayout *layout, int row = -1);
    void setFieldName(const QString& fieldName);
    virtual void setText(const QString& label, const QString& detailLabel);

    virtual bool supportsField(const QString& fieldName);
    virtual void read(SearchXmlCachingReader& reader) = 0;
    virtual void write(SearchXmlWriter& writer) = 0;
    virtual void reset() = 0;

    virtual void setVisible(bool visible);
    virtual bool isVisible();

    void setCategoryLabelVisible(bool visible);
    void setCategoryLabelVisibleFromPreviousField(SearchField *previousField);

    enum WidgetRectType { LabelAndValueWidgetRects, ValueWidgetRectsOnly };
    QList<QRect> widgetRects(WidgetRectType = ValueWidgetRectsOnly) const;

protected Q_SLOTS:

    void clearButtonClicked();

protected:

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column) = 0;
    virtual void setupLabels(QGridLayout *layout, int line);

    virtual void setValueWidgetsVisible(bool visible) = 0;
    virtual QList<QRect> valueWidgetRects() const = 0;

    void setValidValueState(bool valueIsValid);

    QString m_name;

    QLabel *m_label;
    QLabel *m_detailLabel;

    AnimatedClearButton *m_clearButton;

    bool    m_categoryLabelVisible;
    bool    m_valueIsValid;
};

class SearchFieldText : public SearchField
{
    Q_OBJECT

public:

    SearchFieldText(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void setValueWidgetsVisible(bool visible);
    virtual void reset();
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void valueChanged(const QString& text);

protected:

    QLineEdit *m_edit;
};

class SearchFieldKeyword : public SearchFieldText
{
public:

    SearchFieldKeyword(QObject *parent);

    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
};

class SearchFieldRangeInt : public SearchField
{
    Q_OBJECT

public:

    SearchFieldRangeInt(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

    void setBetweenText(const QString& text);
    void setNoValueText(const QString& text);
    void setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix);
    void setBoundary(int min, int max, int step = 1);
    void enableFractionMagic(const QString& prefix);

    void setSuggestedValues(const QList<int>& values);
    void setSuggestedInitialValue(int initialValue);
    void setSingleSteps(int smaller, int larger);
    void setInvertStepping(bool invert);

protected Q_SLOTS:

    void valueChanged();

protected:

    int             m_min;
    int             m_max;
    bool            m_reciprocal;
    CustomStepsIntSpinBox
                   *m_firstBox;
    CustomStepsIntSpinBox
                   *m_secondBox;
    QLabel         *m_betweenLabel;
};

class SearchFieldRangeDouble : public SearchField
{
    Q_OBJECT

public:

    SearchFieldRangeDouble(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

    void setBetweenText(const QString& text);
    void setNoValueText(const QString& text);
    void setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix);
    void setBoundary(double min, double max, int decimals, double step);
    void setFactor(double factor);

    void setSuggestedValues(const QList<double>& values);
    void setSuggestedInitialValue(double initialValue);
    void setSingleSteps(double smaller, double larger);
    void setInvertStepping(bool invert);

protected Q_SLOTS:

    void valueChanged();

protected:

    double          m_min;
    double          m_max;
    double          m_factor;
    CustomStepsDoubleSpinBox
                   *m_firstBox;
    CustomStepsDoubleSpinBox
                   *m_secondBox;
    QLabel         *m_betweenLabel;
};

class SearchFieldRangeDate : public SearchField
{
    Q_OBJECT

public:

    enum Type
    {
        DateOnly,
        DateTime
    };

    SearchFieldRangeDate(QObject *parent, Type type);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

    void setBetweenText(const QString& between);
    void setBoundary(QDateTime min, QDateTime max);

protected Q_SLOTS:

    void valueChanged();

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

    SearchFieldChoice(QObject *parent);

    void setChoice(const QMap<int, QString>& map);
    void setChoice(const QStringList& choice);
    void setAnyText(const QString& anyText);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void checkStateChanged();

protected:

    void updateComboText();

    ChoiceSearchComboBox        *m_comboBox;
    QVariant::Type               m_type;
    QString                      m_anyText;
    ChoiceSearchModel           *m_model;
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
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void updateState();

protected:

    AlbumSelectComboBox         *m_comboBox;
    Type                         m_type;
    AbstractCheckableAlbumModel *m_model;
};

class SearchFieldRating : public SearchField
{
    Q_OBJECT

public:

    SearchFieldRating(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

    void setBetweenText(const QString& text);

protected Q_SLOTS:

    void firstValueChanged();
    void secondValueChanged();

protected:

    RatingComboBox *m_firstBox;
    RatingComboBox *m_secondBox;
    QLabel         *m_betweenLabel;
};

class SearchFieldColorDepth : public SearchField
{
    Q_OBJECT

public:

    SearchFieldColorDepth(QObject *parent);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void indexChanged(int);

protected:

    QComboBox      *m_comboBox;
};

}

#endif


