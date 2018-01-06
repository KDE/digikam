/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-12
 * Description : Widget for assignment and confirmation of names for faces
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "assignnamewidget.h"

// Qt includes

#include <QGridLayout>
#include <QKeyEvent>
#include <QToolButton>
#include <QApplication>
#include <QIcon>
#include <QFontDatabase>

// Local includes

#include "dexpanderbox.h"
#include "digikam_debug.h"
#include "addtagscombobox.h"
#include "addtagslineedit.h"
#include "album.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "facetagsiface.h"
#include "dimg.h"
#include "imageinfo.h"
#include "thememanager.h"
#include "applicationsettings.h"

namespace Digikam
{

class AssignNameWidget::Private
{
public:

    explicit Private(AssignNameWidget* const q)
        : q(q)
    {
        mode             = InvalidMode;
        layoutMode       = InvalidLayout;
        visualStyle      = InvalidVisualStyle;
        widgetMode       = InvalidTagEntryWidgetMode;
        comboBox         = 0;
        lineEdit         = 0;
        confirmButton    = 0;
        rejectButton     = 0;
        clickLabel       = 0;
        layout           = 0;
        modelsGiven      = 0;
        tagModel         = 0;
        tagFilterModel   = 0;
        tagFilteredModel = 0;
    }

    void         updateModes();
    void         updateContents();

    bool         isValid() const;

private:

    void         clearWidgets();
    void         checkWidgets();
    void         updateLayout();
    void         updateVisualStyle();

    QToolButton* createToolButton(const QIcon& icon, const QString& text, const QString& tip = QString()) const;

    QWidget* addTagsWidget() const
    {
        if (comboBox)
        {
            return comboBox;
        }
        else
        {
            return lineEdit;
        }
    }

    template <class T> void setupAddTagsWidget(T* const widget);
    template <class T> void setAddTagsWidgetContents(T* const widget);

    void layoutAddTagsWidget(bool exceedBounds, int minimumContentsLength);
    void setSizePolicies(QSizePolicy::Policy h, QSizePolicy::Policy v);
    void setToolButtonStyles(Qt::ToolButtonStyle style);

public:

    ImageInfo                  info;
    QVariant                   faceIdentifier;
    AlbumPointer<TAlbum>       currentTag;

    Mode                       mode;
    LayoutMode                 layoutMode;
    VisualStyle                visualStyle;
    TagEntryWidgetMode         widgetMode;

    AddTagsComboBox*           comboBox;
    AddTagsLineEdit*           lineEdit;
    QToolButton*               confirmButton;
    QToolButton*               rejectButton;
    DClickLabel*               clickLabel;

    bool                       modelsGiven;
    TagModel*                  tagModel;
    CheckableAlbumFilterModel* tagFilterModel;
    TagPropertiesFilterModel*  tagFilteredModel;
    AlbumPointer<TAlbum>       parentTag;

    QGridLayout*               layout;

    AssignNameWidget* const    q;
};

bool AssignNameWidget::Private::isValid() const
{
    return mode        != InvalidMode        &&
           layoutMode  != InvalidLayout      &&
           visualStyle != InvalidVisualStyle &&
           widgetMode  != InvalidTagEntryWidgetMode;
}

void AssignNameWidget::Private::clearWidgets()
{
    delete comboBox;
    comboBox = 0;

    delete lineEdit;
    lineEdit = 0;

    delete confirmButton;
    confirmButton = 0;

    delete rejectButton;
    rejectButton = 0;

    delete clickLabel;
    clickLabel = 0;
}

QToolButton* AssignNameWidget::Private::createToolButton(const QIcon& icon, const QString& text, const QString& tip) const
{
    QToolButton* const b = new QToolButton;
    b->setIcon(icon);
    b->setText(text);
    b->setToolTip(tip);
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    return b;
}

void AssignNameWidget::Private::updateModes()
{
    if (isValid())
    {
        clearWidgets();
        checkWidgets();
        updateLayout();
        updateVisualStyle();
    }
}

template <class T>
void AssignNameWidget::Private::setupAddTagsWidget(T* const widget)
{
    if (modelsGiven)
    {
        widget->setModel(tagModel, tagFilteredModel, tagFilterModel);
    }

    if (parentTag)
    {
        widget->setParentTag(parentTag);
    }

    q->connect(widget, SIGNAL(taggingActionActivated(TaggingAction)),
               q, SLOT(slotActionActivated(TaggingAction)));

    q->connect(widget, SIGNAL(taggingActionSelected(TaggingAction)),
               q, SLOT(slotActionSelected(TaggingAction)));
}

void AssignNameWidget::Private::checkWidgets()
{
    if (!isValid())
    {
        return;
    }

    switch (mode)
    {
        case InvalidMode:
            break;

        case UnconfirmedEditMode:
        case ConfirmedEditMode:

            switch (widgetMode)
            {
                case InvalidTagEntryWidgetMode:
                    break;

                case AddTagsComboBoxMode:

                    if (!comboBox)
                    {
                        comboBox = new AddTagsComboBox(q);
                        setupAddTagsWidget(comboBox);
                    }

                    break;

                case AddTagsLineEditMode:

                    if (!lineEdit)
                    {
                        lineEdit = new AddTagsLineEdit(q);
                        setupAddTagsWidget(lineEdit);
                    }

                    break;
            }

            if (!confirmButton)
            {
                confirmButton = createToolButton(QIcon::fromTheme(QLatin1String("dialog-ok-apply")), i18n("OK"));

                if (mode == UnconfirmedEditMode)
                {
                    confirmButton->setText(i18n("Confirm"));
                }

                //confirmButton->setToolTip(i18n("Confirm that the selected person is shown here"));
                q->connect(confirmButton, SIGNAL(clicked()),
                           q, SLOT(slotConfirm()));
            }

            if (!rejectButton)
            {
                rejectButton = createToolButton(QIcon::fromTheme(QLatin1String("list-remove")), i18n("Remove"));

                q->connect(rejectButton, SIGNAL(clicked()),
                           q, SLOT(slotReject()));
            }

            break;

        case ConfirmedMode:
        {
            clickLabel = new DClickLabel;
            clickLabel->setAlignment(Qt::AlignCenter);

            connect(clickLabel, SIGNAL(activated()),
                    q, SLOT(slotLabelClicked()));
            break;
        }
    }
}

void AssignNameWidget::Private::layoutAddTagsWidget(bool exceedBounds, int minimumContentsLength)
{
    if (comboBox)
    {
        comboBox->setMinimumContentsLength(minimumContentsLength);
        comboBox->lineEdit()->setAllowExceedBound(exceedBounds);
    }
    else
    {
        lineEdit->setAllowExceedBound(exceedBounds);
    }
}

void AssignNameWidget::Private::setSizePolicies(QSizePolicy::Policy h, QSizePolicy::Policy v)
{
    confirmButton->setSizePolicy(h, v);
    rejectButton->setSizePolicy(h, v);
    addTagsWidget()->setSizePolicy(h, v);
}

void AssignNameWidget::Private::setToolButtonStyles(Qt::ToolButtonStyle style)
{
    confirmButton->setToolButtonStyle(style);
    rejectButton->setToolButtonStyle(style);
}

void AssignNameWidget::Private::updateLayout()
{
    if (!isValid())
    {
        return;
    }

    delete layout;
    layout = new QGridLayout;

    switch (mode)
    {
        case InvalidMode:
            break;

        case UnconfirmedEditMode:
        case ConfirmedEditMode:

            switch (layoutMode)
            {
                case InvalidLayout:
                    break;

                case FullLine:
                {
                    layout->addWidget(addTagsWidget(), 0, 0);
                    layout->addWidget(confirmButton,   0, 1);
                    layout->addWidget(rejectButton,    0, 2);
                    layout->setColumnStretch(0, 1);

                    setSizePolicies(QSizePolicy::Expanding, QSizePolicy::Preferred);
                    setToolButtonStyles(Qt::ToolButtonTextBesideIcon);
                    layoutAddTagsWidget(false, 15);

                    break;
                }

                case TwoLines:
                case Compact:
                {
                    layout->addWidget(addTagsWidget(), 0, 0, 1, 2);
                    layout->addWidget(confirmButton,   1, 0);
                    layout->addWidget(rejectButton,    1, 1);

                    setSizePolicies(QSizePolicy::Expanding, QSizePolicy::Minimum);

                    if (layoutMode == AssignNameWidget::TwoLines)
                    {
                        setToolButtonStyles(Qt::ToolButtonTextBesideIcon);
                        layoutAddTagsWidget(true, 10);
                    }
                    else
                    {
                        setToolButtonStyles(Qt::ToolButtonIconOnly);
                        layoutAddTagsWidget(true, 0);
                    }

                    break;
                }
            }

            break;

        case ConfirmedMode:

            layout->addWidget(clickLabel, 0, 0);
            break;
    }

    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(1);
    q->setLayout(layout);
}

static QString styleSheetFontDescriptor(const QFont& font)
{
    QString s;
    s += (font.pointSize() == -1) ? QString::fromUtf8("font-size: %1px; ").arg(font.pixelSize())
                                  : QString::fromUtf8("font-size: %1pt; ").arg(font.pointSize());
    s += QString::fromUtf8("font-family: \"%1\"; ").arg(font.family());
    return s;
}

void AssignNameWidget::Private::updateVisualStyle()
{
    if (!isValid())
    {
        return;
    }

    switch (visualStyle)
    {
        case InvalidVisualStyle:
            break;

        case TranslucentDarkRound:
        {
            q->setStyleSheet(
                QString::fromUtf8(
                    "QWidget { "
                    " %1 "
                    "} "

                    "QFrame {"
                    "  background-color: rgba(0, 0, 0, 66%); "
                    "  border: 1px solid rgba(100, 100, 100, 66%); "
                    "  border-radius: %2px; "
                    "} "

                    "QToolButton { "
                    "  color: rgba(255, 255, 255, 220); "
                    "  padding: 1px; "
                    "  background-color: "
                    "    qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 100), "
                    "                    stop:1 rgba(255, 255, 255, 0)); "
                    "  border: 1px solid rgba(255, 255, 255, 127); "
                    "  border-radius: 4px; "
                    "} "

                    "QToolButton:hover { "
                    "  border-color: white; "
                    "} "

                    "QToolButton:pressed { "
                    "  background-color: "
                    "    qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0), "
                    "                    stop:1 rgba(255, 255, 255, 100)); "
                    "  border-color: white; "
                    "} "

                    "QToolButton:disabled { "
                    "  color: rgba(255, 255, 255, 120); "
                    "  background-color: "
                    "    qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 50), "
                    "                    stop:1 rgba(255, 255, 255, 0)); "
                    "} "

                    "QComboBox { "
                    "  color: white; "
                    "  background-color: transparent; "
                    "} "

                    "QComboBox QAbstractItemView, QListView::item:!selected { "
                    "  color: white; "
                    "  background-color: rgba(0, 0, 0, 80%); "
                    "} "

                    "QLabel { "
                    "  color: white; background-color: transparent; border: none; "
                    "}"
                ).arg(styleSheetFontDescriptor(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont)))
                 .arg((mode == ConfirmedMode) ? QLatin1String("8") : QLatin1String("4"))
            );
            break;
        }

        case TranslucentThemedFrameless:
        {
            QColor bg = qApp->palette().color(QPalette::Base);
            q->setStyleSheet(
                QString::fromUtf8(
                    "QWidget { "
                    " %1 "
                    "} "

                    "QFrame#assignNameWidget {"
                    "  background-color: "
                    "    qradialgradient(cx:0, cy:0, fx:0, fy:0, radius: 1, stop:0 rgba(%2, %3, %4, 255), "
                    "                    stop:0.8 rgba(%2, %3, %4, 200), stop:1 rgba(%2, %3, %4, 0));"
                    "  border: none; "
                    "  border-radius: 8px; "
                    "}"
                ).arg(styleSheetFontDescriptor(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont)))
                 .arg(bg.red())
                 .arg(bg.green())
                 .arg(bg.blue())
            );
            break;
        }

        case StyledFrame:
        {
            q->setStyleSheet(QString());
            q->setFrameStyle(Raised | StyledPanel);
            break;
        }
    }
}

template <class T>
void AssignNameWidget::Private::setAddTagsWidgetContents(T* const widget)
{
    if (widget)
    {
        widget->setCurrentTag(currentTag);
        widget->setPlaceholderText((mode == UnconfirmedEditMode) ? i18n("Who is this?") : QString());

        if (confirmButton)
        {
            confirmButton->setEnabled(widget->currentTaggingAction().isValid());
        }
    }
}

void AssignNameWidget::Private::updateContents()
{
    if (!isValid())
    {
        return;
    }

    if (comboBox)
    {
        setAddTagsWidgetContents(comboBox);
    }
    else if (lineEdit)
    {
        setAddTagsWidgetContents(lineEdit);
    }

    if (clickLabel)
    {
        clickLabel->setText(currentTag ? currentTag->title() : QString());
    }
}

// -------------------------------------------------------------------

AssignNameWidget::AssignNameWidget(QWidget* const parent)
    : QFrame(parent),
      d(new Private(this))
{
    setObjectName(QLatin1String("assignNameWidget"));
    setVisualStyle(StyledFrame);
}

AssignNameWidget::~AssignNameWidget()
{
    delete d;
}

void AssignNameWidget::setDefaultModel()
{
    setModel(0, 0, 0);
}

void AssignNameWidget::setModel(TagModel* const model, TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel)
{
    // Restrict the tag properties filter model to people if configured.
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        if (settings->showOnlyPersonTagsInPeopleSidebar())
        {
            filteredModel->listOnlyTagsWithProperty(TagPropertyName::person());
        }
    }

    if (d->comboBox)
    {
        d->comboBox->setModel(model, filteredModel, filterModel);
    }
    else if (d->lineEdit)
    {
        d->lineEdit->setModel(model, filteredModel, filterModel);
    }

    if (model || filteredModel || filterModel)
    {
        // possibly set later on box
        d->modelsGiven      = true;
        d->tagModel         = model;
        d->tagFilterModel   = filterModel;
        d->tagFilteredModel = filteredModel;
    }
}

void AssignNameWidget::setParentTag(TAlbum* album)
{
    d->parentTag = album;

    if (d->comboBox)
    {
        d->comboBox->setParentTag(album);
    }
    else if (d->lineEdit)
    {
        d->lineEdit->setParentTag(album);
    }
}

AddTagsComboBox* AssignNameWidget::comboBox() const
{
    return d->comboBox;
}

AddTagsLineEdit* AssignNameWidget::lineEdit() const
{
    return d->lineEdit;
}

void AssignNameWidget::setMode(Mode mode)
{
    if (mode == d->mode)
    {
        return;
    }

    d->mode = mode;
    d->updateModes();
    d->updateContents();
}

AssignNameWidget::Mode AssignNameWidget::mode() const
{
    return d->mode;
}

void AssignNameWidget::setTagEntryWidgetMode(TagEntryWidgetMode mode)
{
    if (d->widgetMode == mode)
    {
        return;
    }

    d->widgetMode = mode;
    d->updateModes();
    d->updateContents();
}

AssignNameWidget::TagEntryWidgetMode AssignNameWidget::tagEntryWidgetMode() const
{
    return d->widgetMode;
}

void AssignNameWidget::setLayoutMode(LayoutMode mode)
{
    if (d->layoutMode == mode)
    {
        return;
    }

    d->layoutMode = mode;
    d->updateModes();
    d->updateContents();
}

AssignNameWidget::LayoutMode AssignNameWidget::layoutMode() const
{
    return d->layoutMode;
}

void AssignNameWidget::setVisualStyle(VisualStyle style)
{
    if (d->visualStyle == style)
    {
        return;
    }

    d->visualStyle = style;
    d->updateModes();
}

AssignNameWidget::VisualStyle AssignNameWidget::visualStyle() const
{
    return d->visualStyle;
}

void AssignNameWidget::setUserData(const ImageInfo& info, const QVariant& faceIdentifier)
{
    d->info           = info;
    d->faceIdentifier = faceIdentifier;
}

ImageInfo AssignNameWidget::info() const
{
    return d->info;
}

QVariant AssignNameWidget::faceIdentifier() const
{
    return d->faceIdentifier;
}

void AssignNameWidget::setCurrentFace(const FaceTagsIface& face)
{
    TAlbum* album = 0;

    if (!face.isNull() && !face.isUnknownName())
    {
        album = AlbumManager::instance()->findTAlbum(face.tagId());
    }

    setCurrentTag(album);
}

void AssignNameWidget::setCurrentTag(int tagId)
{
    setCurrentTag(AlbumManager::instance()->findTAlbum(tagId));
}

void AssignNameWidget::setCurrentTag(TAlbum* album)
{
    if (d->currentTag == album)
    {
        return;
    }

    d->currentTag = album;
    d->updateContents();
}

void AssignNameWidget::slotConfirm()
{
    if (d->comboBox)
    {
        emit assigned(d->comboBox->currentTaggingAction(), d->info, d->faceIdentifier);
    }
    else if (d->lineEdit)
    {
        emit assigned(d->lineEdit->currentTaggingAction(), d->info, d->faceIdentifier);
    }
}

void AssignNameWidget::slotReject()
{
    emit rejected(d->info, d->faceIdentifier);
}

void AssignNameWidget::slotActionActivated(const TaggingAction& action)
{
    emit assigned(action, d->info, d->faceIdentifier);
}

void AssignNameWidget::slotActionSelected(const TaggingAction& action)
{
    if (d->confirmButton)
    {
        d->confirmButton->setEnabled(action.isValid());
    }

    emit selected(action, d->info, d->faceIdentifier);
}

void AssignNameWidget::slotLabelClicked()
{
    emit labelClicked(d->info, d->faceIdentifier);
}

void AssignNameWidget::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            return;

        case Qt::Key_Escape:
            slotReject();
            return;
    }

    QWidget::keyPressEvent(e);
}

void AssignNameWidget::showEvent(QShowEvent* e)
{
    if (d->mode == UnconfirmedEditMode || d->mode == ConfirmedEditMode)
    {
        if (d->comboBox)
        {
            d->comboBox->lineEdit()->selectAll();
            d->comboBox->lineEdit()->setFocus();
        }
        else if (d->lineEdit)
        {
            d->lineEdit->selectAll();
            d->lineEdit->setFocus();
        }
    }

    QWidget::showEvent(e);
}

} // namespace Digikam
