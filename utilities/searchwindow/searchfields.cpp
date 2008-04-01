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

#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTimeEdit>
#include <QCheckBox>
#include <QTreeView>

// KDE includes

#include <klocale.h>
#include <klineedit.h>
#include <ksqueezedtextlabel.h>

// Local includes

#include "ddebug.h"
#include "albummanager.h"
#include "album.h"
#include "albummodel.h"
#include "kdateedit.h"
#include "squeezedcombobox.h"
#include "dmetadata.h"
#include "searchwindow.h"
#include "searchfieldgroup.h"
#include "searchfields.h"
#include "searchfields.moc"

namespace Digikam
{

SearchField *SearchField::createField(const QString &name, SearchFieldGroup *parent)
{
    if (name == "albumid")
    {
        //TODO: allow multiple selection
        SearchFieldAlbum *field = new SearchFieldAlbum(parent, SearchFieldAlbum::TypeAlbum);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("Search pictures located in"));
        return field;
    }
    else if (name == "albumname")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album name contains"));
        return field;
    }
    else if (name == "albumcaption")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album caption contains"));
        return field;
    }
    else if (name == "albumcollection")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album collection contains"));
        return field;
    }
    else if (name == "tagid")
    {
        //TODO: allow multiple selection
        SearchFieldAlbum *field = new SearchFieldAlbum(parent, SearchFieldAlbum::TypeTag);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("Return pictures with tag"));
        return field;
    }
    else if (name == "tagname")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("A tag of the picture contains"));
        return field;
    }
    else if (name == "filename")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("File Name"), i18n("Return pictures whose file name contains"));
        return field;
    }
    else if (name == "modificationdate")
    {
        SearchFieldRangeDate *field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateOnly);
        field->setFieldName(name);
        field->setText(i18n("Modification"), i18n("Return pictures modified between"));
        field->setBetweenText(i18nc("'Return pictures modified between...and...", "and"));
        return field;
    }
    else if (name == "filesize")
    {
        SearchFieldRangeDouble *field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("File Size"), i18n("Size of the file"));
        field->setBetweenText("-");
        field->setNumberPrefixAndSuffix(QString(), "MB");
        field->setBoundary(0, 1000000, 1, 0.5);
        field->setFactor(1024 * 1024);
        return field;
    }

    else if (name == "rating")
    {
    }
    else if (name == "creationdate")
    {
        SearchFieldRangeDate *field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateOnly);
        field->setFieldName(name);
        field->setText(i18n("Date"), i18n("Return pictures created between"));
        field->setBetweenText(i18nc("'Return pictures created between...and...", "and"));
        return field;
    }
    else if (name == "digitizationdate")
    {
        SearchFieldRangeDate *field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateOnly);
        field->setFieldName(name);
        field->setText(i18n("Digitization"), i18n("Return pictures digitized between"));
        field->setBetweenText(i18nc("'Return pictures digitized between...and...", "and"));
        return field;
    }
    else if (name == "orientation")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Orientation"), i18n("Find pictures with orientation"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::Orientation);
        field->setChoice(map);
        return field;
    }
    else if (name == "dimension")
    {
        // "width", "height", "pixels"
    }
    else if (name == "format")
    {//choice
        /*
        SearchFieldImageFormat *field = new SearchFieldImageFormat(parent);
        field->setFieldName(name);
        field->setText(i18n("File Format"), i18n("Return pictures with the image file format"));
        QMap<QString, QString> map;
        map.insert("JPG", "JPEG");
        map.insert("PNG", "PNG");
        map.insert("TIFF", "TIFF");
        map.insert("PPM", "PPM");
        map.insert("JP2K", "JPEG 2000");
        field->setChoice(map);
        return field;
        */
    }
    else if (name == "colordepth")
    {//choice
        SearchFieldColorDepth *field = new SearchFieldColorDepth(parent);
        field->setFieldName(name);
        field->setText(i18n("Color Depth"), i18nc("Find pictures with any color depth / 8 bits per channel...", "Find pictures with"));
        return field;
    }
    else if (name == "colormodel")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Color Model"), i18n("Find pictures with the color model"));
        QMap<int, QString> map;
        field->setChoice(map);
        return field;
    }

    else if (name == "make")
    {//string
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("The make of the camera"));
        return field;
    }
    else if (name == "model")
    {//string
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("The model of the camera"));
        return field;
    }
    else if (name == "aperture")
    {//double
        SearchFieldRangeDouble *field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("Aperture"), i18n("Lens aperture as f-number"));
        field->setBetweenText("-");
        field->setNoValueText("f/#");
        field->setNumberPrefixAndSuffix("f/", QString());
        field->setBoundary(0.5, 128, 1, 0.1);
        return field;
    }
    else if (name == "focallength")
    {//double
        SearchFieldRangeInt *field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Focal length"), i18n("Focal length of the lens"));
        field->setBetweenText("-");
        field->setNumberPrefixAndSuffix(QString(), "mm");
        field->setBoundary(0, 1500, 10);
        return field;
    }
    else if (name == "focallength35")
    {//double
        SearchFieldRangeInt *field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Focal length"), i18n("35mm equivalent focal length"));
        field->setBetweenText("-");
        field->setNumberPrefixAndSuffix(QString(), "mm");
        field->setBoundary(0, 1500, 10);
        return field;
    }
    else if (name == "exposuretime")
    {//double
        SearchFieldRangeInt *field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Exposure time"));
        field->setBetweenText("-");
        field->setNumberPrefixAndSuffix("1/", "s");
        field->setBoundary(1, 4000, 10);
        return field;
    }
    else if (name == "exposureprogram")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Automatic exposure program"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::ExposureProgram);
        field->setChoice(map);
        return field;
    }
    else if (name == "exposuremode")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Automatic or manual exposure"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::ExposureMode);
        field->setChoice(map);
        return field;
    }
    else if (name == "sensitivity")
    {//int
        SearchFieldRangeInt *field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Sensitivity"), i18n("ISO film speed (linear scale, ASA)"));
        field->setBetweenText("-");
        field->setBoundary(5, 6400, 50);
        return field;
    }
    else if (name == "flashmode")
    {//choice
        //TODO: This is a bitmask, and gives some more information
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Flash"), i18n("Flash mode"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::FlashMode);
        field->setChoice(map);
        return field;
    }
    else if (name == "whitebalance")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("White Balance"), i18n("Automatic or manual white balance"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::WhiteBalance);
        field->setChoice(map);
        return field;
    }
    else if (name == "whitebalancecolortemperature")
    {//int
        SearchFieldRangeInt *field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("White balance"), i18n("Color temperature used for white balance"));
        field->setBetweenText("-");
        field->setNumberPrefixAndSuffix(QString(), "K");
        field->setBoundary(1, 1000, 100);
        return field;
    }
    else if (name == "meteringmode")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Metering Mode"), i18n("Method to determine the exposure"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::MeteringMode);
        field->setChoice(map);
        return field;
    }
    else if (name == "subjectdistance")
    {//double
        SearchFieldRangeDouble *field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("Subject Distance"), i18n("Distance of the subject from the lens"));
        field->setBetweenText("-");
        field->setNumberPrefixAndSuffix(QString(), "m");
        field->setBoundary(0.5, 128, 1, 0.1);
        return field;
    }
    else if (name == "subjectdistancecategory")
    {//choice
        SearchFieldChoice *field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Subject Distance"), i18n("Macro, close or distant view"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::SubjectDistanceCategory);
        field->setChoice(map);
        return field;
    }

    else if (name == "latitude")
    {
    }
    else if (name == "longitude")
    {
    }
    else if (name == "altitude")
    {
    }
    else if (name == "positionorientation")
    {
    }
    else if (name == "positiontilt")
    {
    }
    else if (name == "positionroll")
    {
    }
    else if (name == "positiondescription")
    {
    }
    else if (name == "nogps")
    {
    }

    else if (name == "comment")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Caption"), i18n("Return pictures whose comment contains"));
        return field;
    }
    else if (name == "commentauthor")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Author"), i18n("Return pictures commented by"));
        return field;
    }
    else if (name == "headline")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Headline"), i18n("Return pictures with the IPTC headline"));
        return field;
        return field;
    }
    else if (name == "title")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Title"), i18n("Return pictures with the IPTC title"));
        return field;
    }
    else if (name == "keyword")
    {
        SearchFieldText *field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Find results"), i18n("Find pictures with all of the words"));
        return field;
    }
    else
    {
        DWarning() << "SearchField::createField: cannot create SearchField for" << name;
    }
    return 0;
}

SearchField::SearchField(QObject *parent)
    : QObject(parent)
{
    m_label       = new QLabel;
    m_detailLabel = new QLabel;

    m_categoryLabelVisible = true;
}

void SearchField::setup(QGridLayout *layout, int line)
{
    if (line == -1)
        line = layout->rowCount();

    // 10px indent
    layout->setColumnMinimumWidth(0, 10);
    // set stretch for the value widget colums
    layout->setColumnStretch(3, 1);
    layout->setColumnStretch(5, 1);
    // push value widgets to the left
    layout->setColumnStretch(6, 1);

    setupLabels(layout, line);
    setupValueWidgets(layout, line, 3);
}

void SearchField::setupLabels(QGridLayout *layout, int line)
{
    m_label->setObjectName("SearchField_MainLabel");
    m_detailLabel->setObjectName("SearchField_DetailLabel");
    layout->addWidget(m_label, line, 1, Qt::Alignment(Qt::AlignTop | Qt::AlignLeft));
    layout->addWidget(m_detailLabel, line, 2, Qt::Alignment(Qt::AlignTop | Qt::AlignLeft));
}

void SearchField::setFieldName(const QString &fieldName)
{
    m_name = fieldName;
}

void SearchField::setText(const QString &label, const QString &detailLabel)
{
    m_label->setText(label);
    m_detailLabel->setText(detailLabel);
}

bool SearchField::supportsField(const QString &fieldName)
{
    return m_name == fieldName;
}

void SearchField::setVisible(bool visible)
{
    m_label->setVisible(visible && m_categoryLabelVisible);
    m_detailLabel->setVisible(visible);
    setValueWidgetsVisible(visible);
}

bool SearchField::isVisible()
{
    // the detail label is considered representative for all widgets
    return m_detailLabel->isVisible();
}

// ----------------------------------- //

SearchFieldText::SearchFieldText(QObject *parent)
    : SearchField(parent), m_edit(0)
{
}

void SearchFieldText::setupValueWidgets(QGridLayout *layout, int row, int column)
{
    m_edit = new QLineEdit;
    layout->addWidget(m_edit, row, column, 1, 3);
}

void SearchFieldText::read(SearchXmlReader &reader)
{
    QString value = reader.value();
    m_edit->setText(value);
}

void SearchFieldText::write(SearchXmlWriter &writer)
{
    QString value = m_edit->text();
    if (!value.isEmpty())
    {
        writer.writeField(m_name, SearchXml::Like);
        writer.writeValue(value);
        writer.finishField();
    }
}

void SearchFieldText::reset()
{
    m_edit->setText(QString());
}

void SearchFieldText::setValueWidgetsVisible(bool visible)
{
    m_edit->setVisible(visible);
}

// ----------------------------------- //

SearchFieldRangeDate::SearchFieldRangeDate(QObject *parent, Type type)
    : SearchField(parent),
      m_firstTimeEdit(0), m_firstDateEdit(0),
      m_secondTimeEdit(0), m_secondDateEdit(0), m_type(type)
{
    m_betweenLabel   = new QLabel;
}

void SearchFieldRangeDate::setupValueWidgets(QGridLayout *layout, int row, int column)
{
//     QHBoxLayout *hbox = new QHBoxLayout;
//     layout->addLayout(hbox, row, column, 1, 3);

    m_firstDateEdit  = new KDateEdit;
    m_secondDateEdit = new KDateEdit;

    if (m_type == DateOnly)
    {
        layout->addWidget(m_firstDateEdit, row, column);
        layout->addWidget(m_betweenLabel, row, column+1, Qt::AlignHCenter);
        layout->addWidget(m_secondDateEdit, row, column+2);
/*        hbox->addWidget(m_firstDateEdit);
        hbox->addWidget(m_betweenLabel);
        hbox->addWidget(m_secondDateEdit);
        hbox->addWidget(m_endLabel);
        hbox->addStretch(1);*/
    }
    else
    {
        QHBoxLayout *hbox1 = new QHBoxLayout;
        QHBoxLayout *hbox2 = new QHBoxLayout;

        layout->addLayout(hbox1, row, column);
        layout->addWidget(m_betweenLabel, row, column+1, Qt::AlignHCenter);
        layout->addLayout(hbox2, row, column+2);

        hbox1->addWidget(m_firstDateEdit);
        hbox1->addWidget(m_firstTimeEdit);
        hbox2->addWidget(m_secondDateEdit);
        hbox2->addWidget(m_secondTimeEdit);

/*        m_firstTimeEdit = new QTimeEdit;
        m_secondTimeEdit = new QTimeEdit;

        hbox->addWidget(m_firstDateEdit);
        hbox->addWidget(m_firstTimeEdit);
        hbox->addWidget(m_betweenLabel);
        hbox->addWidget(m_secondDateEdit);
        hbox->addWidget(m_secondTimeEdit);
        hbox->addWidget(m_endLabel);
        hbox->addStretch(1);*/
    }
}

void SearchFieldRangeDate::setBetweenText(const QString &between)
{
    m_betweenLabel->setText(between);
}

void SearchFieldRangeDate::read(SearchXmlReader &reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    QDateTime dt = reader.valueToDateTime();
    if (dt.time() == QTime(0,0,0,0))
    {
        if (relation == SearchXml::GreaterThanOrEqual)
        {
            m_firstDateEdit->setDate(dt.date());
            if (m_type == DateTime)
                m_firstTimeEdit->setTime(QTime(0,0,0,0));
        }
        else if (relation == SearchXml::GreaterThan)
        {
            dt.addDays(1);
            m_firstDateEdit->setDate(dt.date());
            if (m_type == DateTime)
                m_firstTimeEdit->setTime(QTime(0,0,0,0));
        }
        else if (relation == SearchXml::LessThanOrEqual)
        {
            m_secondDateEdit->setDate(dt.date());
            if (m_type == DateTime)
                m_secondTimeEdit->setTime(QTime(0,0,0,0));
        }
        else if (relation == SearchXml::LessThan)
        {
            dt.addDays(-1);
            m_secondDateEdit->setDate(dt.date());
            if (m_type == DateTime)
                m_secondTimeEdit->setTime(QTime(0,0,0,0));
        }
    }
    else
    {
        if (relation == SearchXml::GreaterThanOrEqual || relation == SearchXml::GreaterThan)
        {
            m_firstDateEdit->setDate(dt.date());
            if (m_type == DateTime)
                m_firstTimeEdit->setTime(dt.time());
        }
        else if (relation == SearchXml::LessThanOrEqual || relation == SearchXml::LessThan)
        {
            m_secondDateEdit->setDate(dt.date());
            if (m_type == DateTime)
                m_secondTimeEdit->setTime(dt.time());
        }
    }
}

void SearchFieldRangeDate::write(SearchXmlWriter &writer)
{
    QDate date = m_firstDateEdit->date();
    if (date.isValid())
    {
        writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
        QDateTime dt(date);
        if (m_type == DateTime)
            dt.setTime(m_firstTimeEdit->time());
        writer.writeValue(dt);
        writer.finishField();
    }

    date = m_secondDateEdit->date();
    if (date.isValid())
    {
        writer.writeField(m_name, SearchXml::LessThan);
        QDateTime dt(date);
        if (m_type == DateTime)
            dt.setTime(m_secondTimeEdit->time());
        else
            dt = dt.addDays(1); // include whole day
        writer.writeValue(dt);
        writer.finishField();
    }
}

void SearchFieldRangeDate::reset()
{
    m_firstDateEdit->setDate(QDate());
    if (m_type == DateTime)
        m_firstTimeEdit->setTime(QTime(0,0,0,0));
    m_secondDateEdit->setDate(QDate());
    if (m_type == DateTime)
        m_secondTimeEdit->setTime(QTime(0,0,0,0));
}

void SearchFieldRangeDate::setBoundary(QDateTime min, QDateTime max)
{
    //something here?
    Q_UNUSED(min);
    Q_UNUSED(max);
}

void SearchFieldRangeDate::setValueWidgetsVisible(bool visible)
{
    m_firstDateEdit->setVisible(visible);
    if (m_firstTimeEdit)
        m_firstTimeEdit->setVisible(visible);
    m_secondDateEdit->setVisible(visible);
    if (m_secondTimeEdit)
        m_secondTimeEdit->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

// ----------------------------------- //

SearchFieldRangeInt::SearchFieldRangeInt(QObject *parent)
    : SearchField(parent), m_min(0), m_max(100),
      m_firstBox(0), m_secondBox(0)
{
    m_betweenLabel = new QLabel;
    m_firstBox  = new QSpinBox;
    m_secondBox = new QSpinBox;
}

void SearchFieldRangeInt::setupValueWidgets(QGridLayout *layout, int row, int column)
{
//     QHBoxLayout *hbox = new QHBoxLayout;
//     layout->addLayout(hbox, row, column);

    m_firstBox->setSpecialValueText(" ");
    m_secondBox->setSpecialValueText(" ");

//     hbox->addWidget(m_firstBox);
//     hbox->addWidget(m_betweenLabel);
//     hbox->addWidget(m_secondBox);
//     hbox->addStretch(1);
    layout->addWidget(m_firstBox, row, column);
    layout->addWidget(m_betweenLabel, row, column+1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox, row, column+2);

    connect(m_firstBox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged()));

    connect(m_secondBox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeInt::read(SearchXmlReader &reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    if (relation == SearchXml::GreaterThanOrEqual || relation == SearchXml::GreaterThan)
        m_firstBox->setValue(reader.valueToInt());
    if (relation == SearchXml::LessThanOrEqual || relation == SearchXml::LessThan)
        m_secondBox->setValue(reader.valueToInt());
}

void SearchFieldRangeInt::write(SearchXmlWriter &writer)
{
    if (m_firstBox->value() != m_firstBox->minimum()
        && m_firstBox->value() == m_secondBox->value())
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(m_firstBox->value());
        writer.finishField();
    }
    else
    {
        if (m_firstBox->value() != m_firstBox->minimum())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(m_firstBox->value());
            writer.finishField();
        }
        if (m_secondBox->value() != m_secondBox->minimum())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(m_firstBox->value());
            writer.finishField();
        }
    }
}

void SearchFieldRangeInt::setBetweenText(const QString &text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRangeInt::setNumberPrefixAndSuffix(const QString &prefix, const QString &suffix)
{
    m_firstBox->setPrefix(prefix);
    m_secondBox->setPrefix(prefix);
    m_firstBox->setSuffix(suffix);
    m_secondBox->setSuffix(suffix);
}

void SearchFieldRangeInt::setBoundary(int min, int max, int step)
{
    m_min = min;
    m_max = max;

    m_firstBox->setRange(min, max);
    m_firstBox->setSingleStep(step);
    m_firstBox->setValue(min);

    m_secondBox->setRange(min, max);
    m_secondBox->setSingleStep(step);
    m_secondBox->setValue(min);
}

void SearchFieldRangeInt::valueChanged()
{
    if (m_secondBox->value() != m_secondBox->minimum())
        m_firstBox->setRange(m_min, m_secondBox->value());
    if (m_firstBox->value() != m_firstBox->minimum())
        m_secondBox->setRange(m_firstBox->value(), m_max);
}

void SearchFieldRangeInt::reset()
{
    m_firstBox->setValue(m_firstBox->minimum());
    m_secondBox->setValue(m_secondBox->minimum());
}

void SearchFieldRangeInt::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

// ----------------------------------- //

SearchFieldRangeDouble::SearchFieldRangeDouble(QObject *parent)
    : SearchField(parent), m_min(0), m_max(100), m_factor(1),
      m_firstBox(0), m_secondBox(0)
{
    m_betweenLabel = new QLabel;
    m_firstBox  = new QDoubleSpinBox;
    m_secondBox = new QDoubleSpinBox;
}

void SearchFieldRangeDouble::setupValueWidgets(QGridLayout *layout, int row, int column)
{
//     QHBoxLayout *hbox = new QHBoxLayout;
//     layout->addLayout(hbox, row, column);

    m_firstBox->setSpecialValueText(" ");
    m_secondBox->setSpecialValueText(" ");

/*    hbox->addWidget(m_firstBox);
    hbox->addWidget(m_betweenLabel);
    hbox->addWidget(m_secondBox);
    hbox->addStretch(1);*/
    layout->addWidget(m_firstBox, row, column);
    layout->addWidget(m_betweenLabel, row, column+1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox, row, column+2);

    connect(m_firstBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueChanged()));

    connect(m_secondBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeDouble::read(SearchXmlReader &reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    if (relation == SearchXml::GreaterThanOrEqual || relation == SearchXml::GreaterThan)
        m_firstBox->setValue(reader.valueToDouble() / m_factor );
    if (relation == SearchXml::LessThanOrEqual || relation == SearchXml::LessThan)
        m_secondBox->setValue(reader.valueToDouble() / m_factor );
}

void SearchFieldRangeDouble::write(SearchXmlWriter &writer)
{
    if (m_firstBox->value() != m_firstBox->minimum()
        && m_firstBox->value() == m_secondBox->value())
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(m_firstBox->value() * m_factor);
        writer.finishField();
    }
    else
    {
        if (m_firstBox->value() != m_firstBox->minimum())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(m_firstBox->value() * m_factor);
            writer.finishField();
        }
        if (m_secondBox->value() != m_secondBox->minimum())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(m_firstBox->value() * m_factor);
            writer.finishField();
        }
    }
}

void SearchFieldRangeDouble::setBetweenText(const QString &text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRangeDouble::setNoValueText(const QString &text)
{
    m_firstBox->setSpecialValueText(text);
    m_secondBox->setSpecialValueText(text);
}

void SearchFieldRangeDouble::setNumberPrefixAndSuffix(const QString &prefix, const QString &suffix)
{
    m_firstBox->setPrefix(prefix);
    m_secondBox->setPrefix(prefix);
    m_firstBox->setSuffix(suffix);
    m_secondBox->setSuffix(suffix);
}

void SearchFieldRangeDouble::setBoundary(double min, double max, int decimals, double step)
{
    m_min = min;
    m_max = max;

    m_firstBox->setRange(min, max);
    m_firstBox->setSingleStep(step);
    m_firstBox->setDecimals(decimals);
    m_firstBox->setValue(min);

    m_secondBox->setRange(min, max);
    m_secondBox->setSingleStep(step);
    m_secondBox->setDecimals(decimals);
    m_secondBox->setValue(min);
}

void SearchFieldRangeDouble::setFactor(double factor)
{
    m_factor = factor;
}

void SearchFieldRangeDouble::valueChanged()
{
    if (m_secondBox->value() != m_secondBox->minimum())
        m_firstBox->setRange(m_min, m_secondBox->value());
    if (m_firstBox->value() != m_firstBox->minimum())
        m_secondBox->setRange(m_firstBox->value(), m_max);
}

void SearchFieldRangeDouble::reset()
{
    m_firstBox->setValue(m_firstBox->minimum());
    m_secondBox->setValue(m_secondBox->minimum());
}

void SearchFieldRangeDouble::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

// ----------------------------------- //

SearchFieldChoice::SearchFieldChoice(SearchFieldGroup *parent)
    : SearchField(parent), m_vbox(0)
{
    m_anyText = i18n("Any");
    m_label = new SearchSqueezedClickLabel;
    m_label->setObjectName("SearchFieldChoice_ClickLabel");
    m_controller = new VisibilityController(this);
    m_controller->setContainerWidget(parent);
}

void SearchFieldChoice::setupValueWidgets(QGridLayout *layout, int row, int column)
{
    m_vbox = new QVBoxLayout;
    layout->addLayout(m_vbox, row, column, 1, 3);

    m_label->setTextElideMode(Qt::ElideRight);
    m_vbox->addWidget(m_label);

    connect(m_label, SIGNAL(leftClicked()),
            this, SLOT(slotClicked()));

    setupChoiceWidgets();
    slotUpdateLabel();
}

void SearchFieldChoice::slotClicked()
{
    m_controller->triggerVisibility();
}

void SearchFieldChoice::slotUpdateLabel()
{
    QString text = valueText();
    if (text.isNull())
        text = m_anyText;
    m_label->setText(text);
}

void SearchFieldChoice::setValueWidgetsVisible(bool visible)
{
    m_label->setVisible(visible);
    if (!visible)
        m_controller->hide();
}

void SearchFieldChoice::setupChoiceWidgets()
{
    QGroupBox *groupbox = new QGroupBox;
    m_vbox->addWidget(groupbox);
    m_controller->addWidget(groupbox);
    QVBoxLayout *vbox = new QVBoxLayout;

    QMap<int, QString>::const_iterator it;
    for (it = m_choiceMap.begin(); it != m_choiceMap.end(); ++it)
    {
        QCheckBox *box = new QCheckBox;
        box->setText(it.value());
        vbox->addWidget(box);
        m_controller->addWidget(box);
        m_widgetMap[box] = it.key();

        connect(box, SIGNAL(stateChanged(int)),
                this, SLOT(slotUpdateLabel()));
    }

    groupbox->setLayout(vbox);
}

QString SearchFieldChoice::valueText() const
{
    QStringList list;
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        if (it.key()->isChecked())
            list << it.key()->text();
    }
    if (list.isEmpty())
        return QString();
    else if (list.size() == 1)
    {
        return list.first();
    }
    else
    {
        return i18n("Either of: %1", list.join(", "));
    }
}

void SearchFieldChoice::read(SearchXmlReader &reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    QList<int> values;
    if (relation == SearchXml::OneOf)
    {
        setValues(reader.valueToIntList());
    }
    else
    {
        setValues(reader.valueToInt(), relation);
    }
}

void SearchFieldChoice::write(SearchXmlWriter &writer)
{
    QList<int> v = values();
    if (!v.isEmpty())
    {
        if (v.size() == 1)
        {
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(v.first());
            writer.finishField();
        }
        else
        {
            writer.writeField(m_name, SearchXml::OneOf);
            writer.writeValue(v);
            writer.finishField();
        }
    }
}

void SearchFieldChoice::reset()
{
    setValues(QList<int>());
}

void SearchFieldChoice::setChoice(const QMap<int, QString> &map)
{
    m_choiceMap = map;
}

void SearchFieldChoice::setValues(const QList<int> &values)
{
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        it.key()->setChecked(values.contains(it.value()));
    }
}

void SearchFieldChoice::setValues(int value, SearchXml::Relation relation)
{
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        it.key()->setChecked(SearchXml::testRelation(it.value(), value, relation));
    }
}

QList<int> SearchFieldChoice::values() const
{
    QList<int> list;
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        if (it.key()->isChecked())
            list << it.value();
    }
    return list;
}


// ----------------------------------- //

SearchFieldAlbum::SearchFieldAlbum(QObject *parent, Type type)
    : SearchField(parent),
      m_comboBox(0), m_type(type)
{
}

void SearchFieldAlbum::setupValueWidgets(QGridLayout *layout, int row, int column)
{
    m_comboBox = new TreeViewComboBox;
    m_comboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    if (m_type == TypeAlbum)
    {
        m_model = new AlbumModel(AlbumModel::IgnoreRootAlbum);
        m_anyText = i18n("Any Album");
    }
    else if (m_type == TypeTag)
    {
        m_model = new TagModel(AlbumModel::IgnoreRootAlbum);
        m_anyText = i18n("Any Tag");
    }

    m_model->setCheckable(true);
    connect(m_model, SIGNAL(checkStateChanged(Album*, int)),
            this, SLOT(checkStateChanged(Album*, int)));

    m_comboBox->setModel(m_model);
    m_comboBox->installView();
    updateComboText();

    layout->addWidget(m_comboBox, row, column, 1, 3);
}

void SearchFieldAlbum::checkStateChanged(Album *, int)
{
    updateComboText();
}

void SearchFieldAlbum::updateComboText()
{
    QList<Album *> checkedAlbums = m_model->checkedAlbums();
    if (checkedAlbums.isEmpty())
    {
        m_comboBox->setLineEditText(m_anyText);
    }
    else if (checkedAlbums.count() == 1)
    {
        m_comboBox->setLineEditText(checkedAlbums.first()->title());
    }
    else
    {
        if (m_type == TypeAlbum)
            m_comboBox->setLineEditText(i18n("%1 Albums selected", checkedAlbums.count()));
        else
            m_comboBox->setLineEditText(i18n("%1 Tags selected", checkedAlbums.count()));
    }
}

void SearchFieldAlbum::read(SearchXmlReader &reader)
{
    int id = reader.valueToInt();
    Album *a = 0;
    if (m_type == TypeAlbum)
        a = AlbumManager::instance()->findPAlbum(id);
    else if (m_type == TypeTag)
        a = AlbumManager::instance()->findTAlbum(id);

    if (!a)
        DDebug() << "Search: Did not find album for ID" << id << "given in Search XML";

    m_model->setChecked(a, true);
}

void SearchFieldAlbum::write(SearchXmlWriter &writer)
{
    QList<Album *> checkedAlbums = m_model->checkedAlbums();
    foreach(Album *album, checkedAlbums)
    {
        writer.writeField(m_name, SearchXml::InTree);
        writer.writeValue(album->id());
        writer.finishField();
    }
}

void SearchFieldAlbum::reset()
{
    m_model->resetCheckedAlbums();
}

void SearchFieldAlbum::setValueWidgetsVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

// ----------------------------------- //

SearchFieldColorDepth::SearchFieldColorDepth(QObject *parent)
    : SearchField(parent),
      m_comboBox(0)
{
}

void SearchFieldColorDepth::setupValueWidgets(QGridLayout *layout, int row, int column)
{
    m_comboBox = new QComboBox;
    m_comboBox->setEditable(false);
    layout->addWidget(m_comboBox, row, column);

    m_comboBox->addItem(i18n("any color depth"));
    m_comboBox->addItem(i18n("16 bits per channel"), 16);
    m_comboBox->addItem(i18n("8 bits per channel"), 8);

    m_comboBox->setCurrentIndex(0);
}

void SearchFieldColorDepth::read(SearchXmlReader &reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    if (relation == SearchXml::Equal)
    {
        int bits = reader.valueToInt();
        if (bits == 8)
            m_comboBox->setCurrentIndex(1);
        else if (bits == 16)
            m_comboBox->setCurrentIndex(2);
    }
}

void SearchFieldColorDepth::write(SearchXmlWriter &writer)
{
    int index = m_comboBox->currentIndex();
    if (index != -1)
    {
        QVariant bits = m_comboBox->itemData(index);
        if (!bits.isNull())
        {
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(bits.toInt());
            writer.finishField();
        }
    }
}

void SearchFieldColorDepth::setValueWidgetsVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

void SearchFieldColorDepth::reset()
{
    m_comboBox->setCurrentIndex(0);
}


}



