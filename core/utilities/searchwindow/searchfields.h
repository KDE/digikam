/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QObject>
#include <QWidget>
#include <QMap>

// Local includes

#include "coredbsearchxml.h"
#include "searchutilities.h"
#include "visibilitycontroller.h"

class QLabel;
class QCheckBox;
class QGridLayout;
class QSpinBox;
class QDoubleSpinBox;
class QTimeEdit;
class QTreeView;
class QComboBox;
class QLineEdit;

namespace Digikam
{

class DAdjustableLabel;
class AbstractAlbumTreeViewSelectComboBox;
class SearchFieldGroup;
class SqueezedComboBox;
class DDateEdit;
class ChoiceSearchModel;
class ChoiceSearchComboBox;
class RatingComboBox;
class PickLabelFilter;
class ColorLabelFilter;

class SearchField : public QObject, public VisibilityObject
{
    Q_OBJECT

public:

    enum WidgetRectType
    {
        LabelAndValueWidgetRects,
        ValueWidgetRectsOnly
    };

public:

    static SearchField* createField(const QString& fieldName, SearchFieldGroup* const parent);

    explicit SearchField(QObject* const parent);

    void setup(QGridLayout* const layout, int row = -1);
    void setFieldName(const QString& fieldName);
    void setCategoryLabelVisible(bool visible);
    void setCategoryLabelVisibleFromPreviousField(SearchField* const previousField);

    QList<QRect> widgetRects(WidgetRectType = ValueWidgetRectsOnly) const;

    virtual void setText(const QString& label, const QString& detailLabel);

    virtual bool supportsField(const QString& fieldName);
    virtual void read(SearchXmlCachingReader& reader) = 0;
    virtual void write(SearchXmlWriter& writer) = 0;
    virtual void reset() = 0;

    virtual void setVisible(bool visible);
    virtual bool isVisible();

protected Q_SLOTS:

    void clearButtonClicked();

protected:

    void setValidValueState(bool valueIsValid);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column) = 0;
    virtual void setupLabels(QGridLayout* layout, int line);

    virtual void setValueWidgetsVisible(bool visible) = 0;
    virtual QList<QRect> valueWidgetRects() const = 0;

protected:

    QString              m_name;

    QLabel*              m_label;
    QLabel*              m_detailLabel;

    AnimatedClearButton* m_clearButton;

    bool                 m_categoryLabelVisible;
    bool                 m_valueIsValid;
};

//-----------------------------------------------------------------------------

class SearchFieldText : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldText(QObject* const parent);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void setValueWidgetsVisible(bool visible);
    virtual void reset();
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void valueChanged(const QString& text);

protected:

    QLineEdit* m_edit;
};

//-----------------------------------------------------------------------------

class SearchFieldKeyword : public SearchFieldText
{
public:

    explicit SearchFieldKeyword(QObject* const parent);

    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
};

//-----------------------------------------------------------------------------

class SearchFieldRangeInt : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldRangeInt(QObject* const parent);

    void setBetweenText(const QString& text);
    void setNoValueText(const QString& text);
    void setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix);
    void setBoundary(int min, int max, int step = 1);
    void enableFractionMagic(const QString& prefix);

    void setSuggestedValues(const QList<int>& values);
    void setSuggestedInitialValue(int initialValue);
    void setSingleSteps(int smaller, int larger);
    void setInvertStepping(bool invert);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void valueChanged();

protected:

    int                    m_min;
    int                    m_max;
    bool                   m_reciprocal;
    CustomStepsIntSpinBox* m_firstBox;
    CustomStepsIntSpinBox* m_secondBox;
    QLabel*                m_betweenLabel;
};

//-----------------------------------------------------------------------------

class SearchFieldRangeDouble : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldRangeDouble(QObject* const parent);

    void setBetweenText(const QString& text);
    void setNoValueText(const QString& text);
    void setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix);
    void setBoundary(double min, double max, int decimals, double step);
    void setFactor(double factor);

    void setSuggestedValues(const QList<double>& values);
    void setSuggestedInitialValue(double initialValue);
    void setSingleSteps(double smaller, double larger);
    void setInvertStepping(bool invert);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void valueChanged();

protected:

    double                    m_min;
    double                    m_max;
    double                    m_factor;
    CustomStepsDoubleSpinBox* m_firstBox;
    CustomStepsDoubleSpinBox* m_secondBox;
    QLabel*                   m_betweenLabel;
};

//-----------------------------------------------------------------------------

class SearchFieldRangeDate : public SearchField
{
    Q_OBJECT

public:

    enum Type
    {
        DateOnly,
        DateTime
    };

public:

    SearchFieldRangeDate(QObject* const parent, Type type);

    void setBetweenText(const QString& between);
    void setBoundary(const QDateTime& min, const QDateTime& max);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void valueChanged();

protected:

    QTimeEdit* m_firstTimeEdit;
    DDateEdit* m_firstDateEdit;
    QTimeEdit* m_secondTimeEdit;
    DDateEdit* m_secondDateEdit;
    QLabel*    m_betweenLabel;

    Type       m_type;
};

//-----------------------------------------------------------------------------

class SearchFieldChoice : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldChoice(QObject* const parent);

    void setChoice(const QMap<int, QString>& map);
    void setChoice(const QStringList& choice);
    void setAnyText(const QString& anyText);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void checkStateChanged();

protected:

    void updateComboText();

protected:

    ChoiceSearchComboBox* m_comboBox;
    QVariant::Type        m_type;
    QString               m_anyText;
    ChoiceSearchModel*    m_model;
};

//-----------------------------------------------------------------------------

class Album;
class AbstractCheckableAlbumModel;
//class AlbumSelectComboBox;
class AbstractAlbumTreeViewSelectComboBox;
class AlbumTreeViewSelectComboBox;
class TagTreeViewSelectComboBox;
class SqueezedComboBox;

class SearchFieldAlbum : public SearchField
{
    Q_OBJECT

public:

    enum Type
    {
        TypeAlbum,
        TypeTag
    };

    enum Operation
    {
        All,
        OneOf
    };

public:

    SearchFieldAlbum(QObject* const parent, Type type);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void updateState();

protected:

    QWidget*                             m_wrapperBox;
    AlbumTreeViewSelectComboBox*         m_albumComboBox;
    TagTreeViewSelectComboBox*           m_tagComboBox;
    SqueezedComboBox*                    m_operation;
    Type                                 m_type;
    AbstractCheckableAlbumModel*         m_model;
};

//-----------------------------------------------------------------------------

class SearchFieldRating : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldRating(QObject* const parent);

    void setBetweenText(const QString& text);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void firstValueChanged();
    void secondValueChanged();

protected:

    RatingComboBox* m_firstBox;
    RatingComboBox* m_secondBox;
    QLabel*         m_betweenLabel;
};

//-----------------------------------------------------------------------------

class SearchFieldComboBox : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldComboBox(QObject* const  parent);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void indexChanged(int);

protected:

    QComboBox* m_comboBox;
};

//-----------------------------------------------------------------------------

class SearchFieldCheckBox : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldCheckBox(QObject* const parent);

    void setLabel(const QString& text);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void slotToggled(bool checked);

protected:

    QCheckBox* m_checkBox;
    QString    m_text;
};

//-----------------------------------------------------------------------------

class SearchFieldColorDepth : public SearchFieldComboBox
{
    Q_OBJECT

public:

    explicit SearchFieldColorDepth(QObject* const parent);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
};

//-----------------------------------------------------------------------------

class SearchFieldPageOrientation: public SearchFieldComboBox
{
    Q_OBJECT

public:

    explicit SearchFieldPageOrientation(QObject* const parent);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
};

//-----------------------------------------------------------------------------

class SearchFieldLabels : public SearchField
{
    Q_OBJECT

public:

    explicit SearchFieldLabels(QObject* const parent);

    virtual void setupValueWidgets(QGridLayout* layout, int row, int column);
    virtual void read(SearchXmlCachingReader& reader);
    virtual void write(SearchXmlWriter& writer);
    virtual void reset();
    virtual void setValueWidgetsVisible(bool visible);
    virtual QList<QRect> valueWidgetRects() const;

protected Q_SLOTS:

    void updateState();

protected:

    PickLabelFilter*  m_pickLabelFilter;
    ColorLabelFilter* m_colorLabelFilter;
};

} // namespace Digikam

#endif /* SEARCHFIELDS_H */
