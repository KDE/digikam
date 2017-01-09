/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-09-12
 * @brief  Simple helper widgets collection
 *
 * @author Copyright (C) 2014-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dwidgetutils.h"

// Qt includes

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QFileInfo>
#include <QPainter>
#include <QStandardPaths>
#include <QColorDialog>
#include <QStyleOptionButton>
#include <qdrawutil.h>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DHBox::DHBox(QWidget* const parent)
    : QFrame(parent)
{
    QHBoxLayout* const layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    setLayout(layout);
}

DHBox::DHBox(bool /*vertical*/, QWidget* const parent)
    : QFrame(parent)
{
    QVBoxLayout* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    setLayout(layout);
}

DHBox::~DHBox()
{
}

void DHBox::childEvent(QChildEvent* e)
{
    switch (e->type())
    {
        case QEvent::ChildAdded:
        {
            QChildEvent* const ce = static_cast<QChildEvent*>(e);

            if (ce->child()->isWidgetType())
            {
                QWidget* const w = static_cast<QWidget*>(ce->child());
                static_cast<QBoxLayout*>(layout())->addWidget(w);
            }

            break;
        }

        case QEvent::ChildRemoved:
        {
            QChildEvent* const ce = static_cast<QChildEvent*>(e);

            if (ce->child()->isWidgetType())
            {
                QWidget* const w = static_cast<QWidget*>(ce->child());
                static_cast<QBoxLayout*>(layout())->removeWidget(w);
            }

            break;
        }

        default:
            break;
    }

    QFrame::childEvent(e);
}

QSize DHBox::sizeHint() const
{
    DHBox* const b = const_cast<DHBox*>(this);
    QApplication::sendPostedEvents(b, QEvent::ChildAdded);

    return QFrame::sizeHint();
}

QSize DHBox::minimumSizeHint() const
{
    DHBox* const b = const_cast<DHBox*>(this);
    QApplication::sendPostedEvents(b, QEvent::ChildAdded );

    return QFrame::minimumSizeHint();
}

void DHBox::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

void DHBox::setContentsMargins(const QMargins& margins)
{
    layout()->setContentsMargins(margins);
}

void DHBox::setContentsMargins(int left, int top, int right, int bottom)
{
    layout()->setContentsMargins(left, top, right, bottom);
}

void DHBox::setStretchFactor(QWidget* const widget, int stretch)
{
    static_cast<QBoxLayout*>(layout())->setStretchFactor(widget, stretch);
}

// ------------------------------------------------------------------------------------

DVBox::DVBox(QWidget* const parent)
  : DHBox(true, parent)
{
}

DVBox::~DVBox()
{
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DFileSelector::Private
{
public:

    Private()
    {
        edit      = 0;
        btn       = 0;
        fdMode    = QFileDialog::ExistingFile;
        fdOptions = -1;
    }

    QLineEdit*            edit;
    QPushButton*          btn;

    QFileDialog::FileMode fdMode;
    QString               fdFilter;
    QString               fdTitle;
    int                   fdOptions;
};

DFileSelector::DFileSelector(QWidget* const parent)
    : DHBox(parent),
      d(new Private)
{
    d->edit    = new QLineEdit(this);
    d->btn     = new QPushButton(i18n("Browse..."), this);
    setStretchFactor(d->edit, 10);

    connect(d->btn, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

DFileSelector::~DFileSelector()
{
    delete d;
}

QLineEdit* DFileSelector::lineEdit() const
{
    return d->edit;
}

void DFileSelector::setFileDlgPath(const QString& path)
{
    d->edit->setText(QDir::toNativeSeparators(path));
}

QString DFileSelector::fileDlgPath() const
{
    return QDir::fromNativeSeparators(d->edit->text());
}

void DFileSelector::setFileDlgMode(QFileDialog::FileMode mode)
{
    d->fdMode = mode;
}

void DFileSelector::setFileDlgFilter(const QString& filter)
{
    d->fdFilter = filter;
}

void DFileSelector::setFileDlgTitle(const QString& title)
{
    d->fdTitle = title;
}

void DFileSelector::setFileDlgOptions(QFileDialog::Options opts)
{
    d->fdOptions = (int)opts;
}

void DFileSelector::slotBtnClicked()
{
    if (d->fdMode == QFileDialog::ExistingFiles)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Multiple selection is not supported";
        return;
    }

    QFileDialog* const fileDlg = new QFileDialog();

    if (d->fdOptions != -1)
        fileDlg->setOptions((QFileDialog::Options)d->fdOptions);

    fileDlg->setDirectory(QFileInfo(fileDlgPath()).filePath());
    fileDlg->setFileMode(d->fdMode);

    if (!d->fdFilter.isNull())
        fileDlg->setNameFilter(d->fdFilter);

    if (!d->fdTitle.isNull())
        fileDlg->setWindowTitle(d->fdTitle);

    emit signalOpenFileDialog();

    if (fileDlg->exec() == QDialog::Accepted)
    {
        QStringList sel = fileDlg->selectedFiles();

        if (!sel.isEmpty())
        {
            setFileDlgPath(sel.first());
            emit signalUrlSelected(QUrl::fromLocalFile(sel.first()));
        }
    }

    delete fileDlg;
}

// ---------------------------------------------------------------------------------------

DWorkingPixmap::DWorkingPixmap()
{
    QPixmap pix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/process-working.png")));
    QSize   size(22, 22);

    if (pix.isNull())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Invalid pixmap specified.";
        return;
    }

    if (!size.isValid())
    {
        size = QSize(pix.width(), pix.width());
    }

    if (pix.width() % size.width() || pix.height() % size.height())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Invalid framesize.";
        return;
    }

    const int rowCount = pix.height() / size.height();
    const int colCount = pix.width()  / size.width();
    m_frames.resize(rowCount * colCount);

    int pos = 0;

    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            QPixmap frm     = pix.copy(col * size.width(), row * size.height(), size.width(), size.height());
            m_frames[pos++] = frm;
        }
    }
}

DWorkingPixmap::~DWorkingPixmap()
{
}

bool DWorkingPixmap::isEmpty() const
{
    return m_frames.isEmpty();
}

QSize DWorkingPixmap::frameSize() const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "No frame loaded.";
        return QSize();
    }

    return m_frames[0].size();
}

int DWorkingPixmap::frameCount() const
{
    return m_frames.size();
}

QPixmap DWorkingPixmap::frameAt(int index) const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "No frame loaded.";
        return QPixmap();
    }

    return m_frames.at(index);
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DColorSelector::Private
{
public:

    Private()
    {
    }

    QColor color;
};

DColorSelector::DColorSelector(QWidget* const parent)
    : QPushButton(parent),
      d(new Private)
{
    connect(this, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

DColorSelector::~DColorSelector()
{
    delete d;
}

void DColorSelector::setColor(const QColor& color)
{
    if (color.isValid())
    {
        d->color = color;
        update();
    }
}

QColor DColorSelector::color() const
{
    return d->color;
}

void DColorSelector::slotBtnClicked()
{
    QColor color = QColorDialog::getColor(d->color);

    if (color.isValid())
    {
        setColor(color);
        emit signalColorSelected(color);
    }
}

void DColorSelector::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QStyle* const style = QWidget::style();

    QStyleOptionButton opt;

    opt.initFrom(this);
    opt.state    |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    opt.features  = QStyleOptionButton::None;
    opt.icon      = QIcon();
    opt.text.clear();

    style->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    QRect labelRect = style->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
    int shift       = style->pixelMetric(QStyle::PM_ButtonMargin, &opt, this) / 2;
    labelRect.adjust(shift, shift, -shift, -shift);
    int x, y, w, h;
    labelRect.getRect(&x, &y, &w, &h);

    if (isChecked() || isDown())
    {
        x += style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this);
        y += style->pixelMetric(QStyle::PM_ButtonShiftVertical,   &opt, this);
    }

    QColor fillCol = isEnabled() ? d->color : palette().color(backgroundRole());
    qDrawShadePanel(&painter, x, y, w, h, palette(), true, 1, 0);

    if (fillCol.isValid())
    {
        const QRect rect(x + 1, y + 1, w - 2, h - 2);

        if (fillCol.alpha() < 255)
        {
            QPixmap chessboardPattern(16, 16);
            QPainter patternPainter(&chessboardPattern);
            patternPainter.fillRect(0, 0, 8, 8, Qt::black);
            patternPainter.fillRect(8, 8, 8, 8, Qt::black);
            patternPainter.fillRect(0, 8, 8, 8, Qt::white);
            patternPainter.fillRect(8, 0, 8, 8, Qt::white);
            patternPainter.end();
            painter.fillRect(rect, QBrush(chessboardPattern));
        }

        painter.fillRect(rect, fillCol);
    }

    if (hasFocus())
    {
        QRect focusRect = style->subElementRect(QStyle::SE_PushButtonFocusRect, &opt, this);
        QStyleOptionFocusRect focusOpt;
        focusOpt.init(this);
        focusOpt.rect            = focusRect;
        focusOpt.backgroundColor = palette().background().color();
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, &painter, this);
    }
}

} // namespace Digikam
