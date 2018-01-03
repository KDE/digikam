/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC subjects editor.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "subjectwidget.h"

// Qt includes

#include <QStandardPaths>
#include <QFile>
#include <QValidator>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QApplication>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN SubjectWidget::Private
{
public:

    enum EditionMode
    {
        STANDARD = 0,
        CUSTOM
    };

public:

    Private()
    {
        addSubjectButton = 0;
        delSubjectButton = 0;
        repSubjectButton = 0;
        subjectsBox      = 0;
        iprLabel         = 0;
        refLabel         = 0;
        nameLabel        = 0;
        matterLabel      = 0;
        detailLabel      = 0;
        btnGroup         = 0;
        stdBtn           = 0;
        customBtn        = 0;
        refCB            = 0;
        optionsBox       = 0;
    }

    typedef QMap<QString, SubjectData> SubjectCodesMap;

    SubjectCodesMap                    subMap;

    QStringList                        subjectsList;

    QWidget*                           optionsBox;

    QPushButton*                       addSubjectButton;
    QPushButton*                       delSubjectButton;
    QPushButton*                       repSubjectButton;

    QLabel*                            iprLabel;
    QLabel*                            refLabel;
    QLabel*                            nameLabel;
    QLabel*                            matterLabel;
    QLabel*                            detailLabel;

    QButtonGroup*                      btnGroup;

    QRadioButton*                      stdBtn;
    QRadioButton*                      customBtn;

    QComboBox*                         refCB;

    QListWidget*                       subjectsBox;
};

// --------------------------------------------------------------------------------

SubjectWidget::SubjectWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    // Load subject codes provided by IPTC/NAA as xml file.
    // See http://iptc.cms.apa.at/std/topicset/topicset.iptc-subjectcode.xml for details.

    QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                          QString::fromLatin1("digikam/metadata/topicset.iptc-subjectcode.xml"));

    if (!loadSubjectCodesFromXML(QUrl::fromLocalFile(path)))
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Cannot load IPTC/NAA subject codes XML database";

    // --------------------------------------------------------

    // Subject Reference Number only accept digit.
    QRegExp refDigitRx(QString::fromLatin1("^[0-9]{8}$"));
    QValidator* const refValidator = new QRegExpValidator(refDigitRx, this);

    // --------------------------------------------------------

    m_subjectsCheck        = new QCheckBox(i18n("Use structured definition of the subject matter:"), this);
    d->optionsBox          = new QWidget;
    d->btnGroup            = new QButtonGroup(this);
    d->stdBtn              = new QRadioButton;
    d->customBtn           = new QRadioButton;
    d->refCB               = new QComboBox;
    QLabel* const codeLink = new QLabel(i18n("Use standard "
                                       "<b><a href='http://www.iptc.org/site/NewsCodes'>"
                                       "reference code</a></b>"));
    codeLink->setOpenExternalLinks(true);
    codeLink->setWordWrap(false);

    // By default, check box is not visible.
    m_subjectsCheck->setVisible(false);
    m_subjectsCheck->setEnabled(false);

    QLabel* const customLabel = new QLabel(i18n("Use custom definition"));

    d->btnGroup->addButton(d->stdBtn,    Private::STANDARD);
    d->btnGroup->addButton(d->customBtn, Private::CUSTOM);
    d->btnGroup->setExclusive(true);
    d->stdBtn->setChecked(true);

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin(); it != d->subMap.end(); ++it)
        d->refCB->addItem(it.key());

    // --------------------------------------------------------

    m_iprEdit = new QLineEdit;
    m_iprEdit->setClearButtonEnabled(true);
    m_iprEdit->setMaxLength(32);

    // --------------------------------------------------------

    m_refEdit = new QLineEdit;
    m_refEdit->setClearButtonEnabled(true);
    m_refEdit->setValidator(refValidator);
    m_refEdit->setMaxLength(8);

    // --------------------------------------------------------

    m_nameEdit = new QLineEdit;
    m_nameEdit->setClearButtonEnabled(true);
    m_nameEdit->setMaxLength(64);

    // --------------------------------------------------------

    m_matterEdit = new QLineEdit;
    m_matterEdit->setClearButtonEnabled(true);
    m_matterEdit->setMaxLength(64);

    // --------------------------------------------------------

    m_detailEdit = new QLineEdit;
    m_detailEdit->setClearButtonEnabled(true);
    m_detailEdit->setMaxLength(64);

    // --------------------------------------------------------

    d->iprLabel    = new QLabel(i18nc("Information Provider Reference: "
                                      "A name, registered with the IPTC/NAA, "
                                      "identifying the provider that guarantees "
                                      "the uniqueness of the UNO", "I.P.R:"));
    d->refLabel    = new QLabel(i18n("Reference:"));
    d->nameLabel   = new QLabel(i18n("Name:"));
    d->matterLabel = new QLabel(i18n("Matter:"));
    d->detailLabel = new QLabel(i18n("Detail:"));

    // --------------------------------------------------------

    d->subjectsBox = new QListWidget;
    d->subjectsBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addSubjectButton = new QPushButton(i18n("&Add"));
    d->delSubjectButton = new QPushButton(i18n("&Delete"));
    d->repSubjectButton = new QPushButton(i18n("&Replace"));
    d->addSubjectButton->setIcon(QIcon::fromTheme(QString::fromLatin1("list-add")).pixmap(16, 16));
    d->delSubjectButton->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-delete")).pixmap(16, 16));
    d->repSubjectButton->setIcon(QIcon::fromTheme(QString::fromLatin1("view-refresh")).pixmap(16, 16));
    d->delSubjectButton->setEnabled(false);
    d->repSubjectButton->setEnabled(false);

    // --------------------------------------------------------

    m_note = new QLabel;
    m_note->setMaximumWidth(150);
    m_note->setOpenExternalLinks(true);
    m_note->setWordWrap(true);
    m_note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    QGridLayout* const optionsBoxLayout = new QGridLayout;
    optionsBoxLayout->addWidget(d->stdBtn,      0, 0, 1, 1);
    optionsBoxLayout->addWidget(codeLink,       0, 1, 1, 2);
    optionsBoxLayout->addWidget(d->refCB,       0, 3, 1, 1);
    optionsBoxLayout->addWidget(d->customBtn,   1, 0, 1, 4);
    optionsBoxLayout->addWidget(customLabel,    1, 1, 1, 4);
    optionsBoxLayout->addWidget(d->iprLabel,    2, 0, 1, 1);
    optionsBoxLayout->addWidget(m_iprEdit,      2, 1, 1, 4);
    optionsBoxLayout->addWidget(d->refLabel,    3, 0, 1, 1);
    optionsBoxLayout->addWidget(m_refEdit,      3, 1, 1, 1);
    optionsBoxLayout->addWidget(d->nameLabel,   4, 0, 1, 1);
    optionsBoxLayout->addWidget(m_nameEdit,     4, 1, 1, 4);
    optionsBoxLayout->addWidget(d->matterLabel, 5, 0, 1, 1);
    optionsBoxLayout->addWidget(m_matterEdit,   5, 1, 1, 4);
    optionsBoxLayout->addWidget(d->detailLabel, 6, 0, 1, 1);
    optionsBoxLayout->addWidget(m_detailEdit,   6, 1, 1, 4);
    optionsBoxLayout->setColumnStretch(4, 10);
    optionsBoxLayout->setContentsMargins(QMargins());
    optionsBoxLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    d->optionsBox->setLayout(optionsBoxLayout);

    // --------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->setAlignment( Qt::AlignTop );
    mainLayout->addWidget(m_subjectsCheck,     0, 0, 1, 4);
    mainLayout->addWidget(d->optionsBox,       1, 0, 1, 4);
    mainLayout->addWidget(d->subjectsBox,      2, 0, 5, 3);
    mainLayout->addWidget(d->addSubjectButton, 2, 3, 1, 1);
    mainLayout->addWidget(d->delSubjectButton, 3, 3, 1, 1);
    mainLayout->addWidget(d->repSubjectButton, 4, 3, 1, 1);
    mainLayout->addWidget(m_note,              5, 3, 1, 1);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->subjectsBox, &QListWidget::itemSelectionChanged,
            this, &SubjectWidget::slotSubjectSelectionChanged);

    connect(d->addSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::slotAddSubject);

    connect(d->delSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::slotDelSubject);

    connect(d->repSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::slotRepSubject);

    connect(d->btnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonReleased),
            this, &SubjectWidget::slotEditOptionChanged);

    connect(d->refCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &SubjectWidget::slotRefChanged);

    // --------------------------------------------------------

    connect(m_subjectsCheck, &QCheckBox::toggled,
            this, &SubjectWidget::slotSubjectsToggled);

    // --------------------------------------------------------

    connect(m_subjectsCheck, &QCheckBox::toggled,
            this, &SubjectWidget::signalModified);

    connect(d->addSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::signalModified);

    connect(d->delSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::signalModified);

    connect(d->repSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::signalModified);

    // --------------------------------------------------------

    slotEditOptionChanged(d->btnGroup->id(d->btnGroup->checkedButton()));
}

SubjectWidget::~SubjectWidget()
{
    delete d;
}

void SubjectWidget::slotSubjectsToggled(bool b)
{
    d->optionsBox->setEnabled(b);
    d->subjectsBox->setEnabled(b);
    d->addSubjectButton->setEnabled(b);
    d->delSubjectButton->setEnabled(b);
    d->repSubjectButton->setEnabled(b);
    slotEditOptionChanged(d->btnGroup->id(d->btnGroup->checkedButton()));
}

void SubjectWidget::slotEditOptionChanged(int b)
{
    if (b == Private::CUSTOM)
    {
        d->refCB->setEnabled(false);
        m_iprEdit->setEnabled(true);
        m_refEdit->setEnabled(true);
        m_nameEdit->setEnabled(true);
        m_matterEdit->setEnabled(true);
        m_detailEdit->setEnabled(true);
    }
    else
    {
        d->refCB->setEnabled(true);
        m_iprEdit->setEnabled(false);
        m_refEdit->setEnabled(false);
        m_nameEdit->setEnabled(false);
        m_matterEdit->setEnabled(false);
        m_detailEdit->setEnabled(false);
        slotRefChanged();
    }
}

void SubjectWidget::slotRefChanged()
{
    QString key = d->refCB->currentText();
    QString name, matter, detail;

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
    {
        if (key == it.key())
        {
            name   = it.value().name;
            matter = it.value().matter;
            detail = it.value().detail;
        }
    }

    m_refEdit->setText(key);
    m_nameEdit->setText(name);
    m_matterEdit->setText(matter);
    m_detailEdit->setText(detail);
}

QString SubjectWidget::buildSubject() const
{
    QString subject = m_iprEdit->text();
    subject.append(QString::fromLatin1(":"));
    subject.append(m_refEdit->text());
    subject.append(QString::fromLatin1(":"));
    subject.append(m_nameEdit->text());
    subject.append(QString::fromLatin1(":"));
    subject.append(m_matterEdit->text());
    subject.append(QString::fromLatin1(":"));
    subject.append(m_detailEdit->text());
    return subject;
}

void SubjectWidget::slotDelSubject()
{
    QListWidgetItem* const item = d->subjectsBox->currentItem();

    if (!item) return;

    d->subjectsBox->takeItem(d->subjectsBox->row(item));
    delete item;
}

void SubjectWidget::slotRepSubject()
{
    QString newSubject = buildSubject();

    if (newSubject.isEmpty()) return;

    if (!d->subjectsBox->selectedItems().isEmpty())
    {
        d->subjectsBox->selectedItems()[0]->setText(newSubject);
        m_iprEdit->clear();
        m_refEdit->clear();
        m_nameEdit->clear();
        m_matterEdit->clear();
        m_detailEdit->clear();
    }
}

void SubjectWidget::slotSubjectSelectionChanged()
{
    if (!d->subjectsBox->selectedItems().isEmpty())
    {
        QString subject = d->subjectsBox->selectedItems()[0]->text();
        m_iprEdit->setText(subject.section(QString::fromLatin1(":"), 0, 0));
        m_refEdit->setText(subject.section(QString::fromLatin1(":"), 1, 1));
        m_nameEdit->setText(subject.section(QString::fromLatin1(":"), 2, 2));
        m_matterEdit->setText(subject.section(QString::fromLatin1(":"), 3, 3));
        m_detailEdit->setText(subject.section(QString::fromLatin1(":"), 4, 4));
        d->delSubjectButton->setEnabled(true);
        d->repSubjectButton->setEnabled(true);
    }
    else
    {
        d->delSubjectButton->setEnabled(false);
        d->repSubjectButton->setEnabled(false);
    }
}

void SubjectWidget::slotAddSubject()
{
    QString newSubject = buildSubject();

    if (newSubject.isEmpty()) return;

    bool found = false;

    for (int i = 0 ; i < d->subjectsBox->count(); i++)
    {
        QListWidgetItem* const item = d->subjectsBox->item(i);

        if (newSubject == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->subjectsBox->insertItem(d->subjectsBox->count(), newSubject);
        m_iprEdit->clear();
        m_refEdit->clear();
        m_nameEdit->clear();
        m_matterEdit->clear();
        m_detailEdit->clear();
    }
}

bool SubjectWidget::loadSubjectCodesFromXML(const QUrl& url)
{
    QFile xmlfile(url.toLocalFile());

    if (!xmlfile.open(QIODevice::ReadOnly))
        return false;

    QDomDocument xmlDoc(QString::fromLatin1("NewsML"));

    if (!xmlDoc.setContent(&xmlfile))
        return false;

    QDomElement xmlDocElem = xmlDoc.documentElement();

    if (xmlDocElem.tagName() != QString::fromLatin1("NewsML"))
        return false;

    for (QDomNode nbE1 = xmlDocElem.firstChild();
         !nbE1.isNull(); nbE1 = nbE1.nextSibling())
    {
        QDomElement newsItemElement = nbE1.toElement();

        if (newsItemElement.isNull()) continue;
        if (newsItemElement.tagName() != QString::fromLatin1("NewsItem")) continue;

        for (QDomNode nbE2 = newsItemElement.firstChild();
            !nbE2.isNull(); nbE2 = nbE2.nextSibling())
        {
            QDomElement topicSetElement = nbE2.toElement();

            if (topicSetElement.isNull()) continue;
            if (topicSetElement.tagName() != QString::fromLatin1("TopicSet")) continue;

            for (QDomNode nbE3 = topicSetElement.firstChild();
                !nbE3.isNull(); nbE3 = nbE3.nextSibling())
            {
                QDomElement topicElement = nbE3.toElement();

                if (topicElement.isNull()) continue;
                if (topicElement.tagName() != QString::fromLatin1("Topic")) continue;

                QString type, name, matter, detail, ref;

                for (QDomNode nbE4 = topicElement.firstChild();
                    !nbE4.isNull(); nbE4 = nbE4.nextSibling())
                {
                    QDomElement topicSubElement = nbE4.toElement();

                    if (topicSubElement.isNull()) continue;

                    if (topicSubElement.tagName() == QString::fromLatin1("TopicType"))
                        type = topicSubElement.attribute(QString::fromLatin1("FormalName"));

                    if (topicSubElement.tagName() == QString::fromLatin1("FormalName"))
                        ref = topicSubElement.text();

                    if (topicSubElement.tagName() == QString::fromLatin1("Description") &&
                        topicSubElement.attribute(QString::fromLatin1("Variant")) == QString::fromLatin1("Name"))
                    {
                        if (type == QString::fromLatin1("Subject"))
                            name = topicSubElement.text();
                        else if (type == QString::fromLatin1("SubjectMatter"))
                            matter = topicSubElement.text();
                        else if (type == QString::fromLatin1("SubjectDetail"))
                            detail = topicSubElement.text();
                    }
                }

                d->subMap.insert(ref, SubjectData(name, matter, detail));
            }
        }
    }

    // Set the Subject Name everywhere on the map.

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
    {
        QString name, keyPrefix;

        if (it.key().endsWith(QLatin1String("00000")))
        {
            keyPrefix = it.key().left(3);
            name      = it.value().name;

            for (Private::SubjectCodesMap::Iterator it2 = d->subMap.begin();
                it2 != d->subMap.end(); ++it2)
            {
                if (it2.key().startsWith(keyPrefix) &&
                    !it2.key().endsWith(QLatin1String("00000")))
                {
                    it2.value().name = name;
                }
            }
        }
    }

    // Set the Subject Matter Name everywhere on the map.

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin();
         it != d->subMap.end(); ++it)
    {
        QString matter, keyPrefix;

        if (it.key().endsWith(QLatin1String("000")))
        {
            keyPrefix = it.key().left(5);
            matter    = it.value().matter;

            for (Private::SubjectCodesMap::Iterator it2 = d->subMap.begin();
                it2 != d->subMap.end(); ++it2)
            {
                if (it2.key().startsWith(keyPrefix) &&
                    !it2.key().endsWith(QLatin1String("000")))
                {
                    it2.value().matter = matter;
                }
            }
        }
    }

    return true;
}

void SubjectWidget::setSubjectsList(const QStringList& list)
{
    d->subjectsList = list;

    blockSignals(true);
    d->subjectsBox->clear();

    if (m_subjectsCheck->isEnabled())
        m_subjectsCheck->setChecked(false);

    if (!d->subjectsList.isEmpty())
    {
        d->subjectsBox->insertItems(0, d->subjectsList);

        if (m_subjectsCheck->isEnabled())
            m_subjectsCheck->setChecked(true);
    }

    blockSignals(false);

    if (m_subjectsCheck->isEnabled())
        slotSubjectsToggled(m_subjectsCheck->isChecked());
}

QStringList SubjectWidget::subjectsList() const
{
    QStringList newSubjects;

    for (int i = 0 ; i < d->subjectsBox->count(); i++)
    {
        QListWidgetItem* item = d->subjectsBox->item(i);
        newSubjects.append(item->text());
    }

    return newSubjects;
}

}  // namespace Digikam
