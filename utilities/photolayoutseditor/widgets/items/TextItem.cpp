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
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "TextItem.h"
#include "global.h"
#include "KEditFactory.h"
#include "TextColorChangeListener.h"
#include "TextFontChangeListener.h"

#include <QTimeLine>
#include <QInputMethodEvent>

#include <klocalizedstring.h>

#include "qttreepropertybrowser.h"
#include "qtpropertymanager.h"

#define IS_NULL(node) if (node.isNull()) goto _delete;

using namespace PhotoLayoutsEditor;

QColor TextItem::DEFAULT_COLOR = Qt::black;
QFont TextItem::DEFAULT_FONT = QFont();

class PhotoLayoutsEditor::TextChangeUndoCommand : public QUndoCommand
{
    QStringList m_text;
    TextItem * m_item;
public:
    TextChangeUndoCommand(const QStringList & text, TextItem * item, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Text change"), parent),
        m_text(text),
        m_item(item)
    {}
    virtual void redo()
    {
        QStringList temp = m_item->d->m_string_list;
        m_item->d->m_string_list = m_text;
        m_text = temp;
    }
    virtual void undo()
    {
        QStringList temp = m_item->d->m_string_list;
        m_item->d->m_string_list = m_text;
        m_text = temp;
    }
};
class PhotoLayoutsEditor::TextColorUndoCommand : public QUndoCommand
{
        TextItem * m_item;
        QColor m_color;
    public:
        TextColorUndoCommand(const QColor & color, TextItem * item, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Text color change"), parent),
            m_item(item),
            m_color(color)
        {}
        virtual void redo()
        {
            run();
        }
        virtual void undo()
        {
            run();
        }
        void run()
        {
            QColor temp = m_item->m_color;
            m_item->m_color = m_color;
            m_color = temp;
            m_item->refresh();
        }
};
class PhotoLayoutsEditor::TextFontUndoCommand : public QUndoCommand
{
        TextItem * m_item;
        QFont m_font;
    public:
        TextFontUndoCommand(const QFont & font, TextItem * item, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Text font change"), parent),
            m_item(item),
            m_font(font)
        {}
        virtual void redo()
        {
            run();
        }
        virtual void undo()
        {
            run();
        }
        void run()
        {
            QFont temp = m_item->m_font;
            m_item->m_font = m_font;
            m_font = temp;
            m_item->refresh();
        }
};
class PhotoLayoutsEditor::AddTextUndoCommand : public QUndoCommand
{
    TextItem::TextItemPrivate * m_item_p;
    QString text;
    int row;
    int at;
public:
    AddTextUndoCommand(int row, int at, TextItem::TextItemPrivate * item_p, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Text edit"), parent),
        m_item_p(item_p),
        row(row),
        at(at)
    {}
    virtual void redo()
    {
        m_item_p->addText(row, at, text);
    }
    virtual void undo()
    {
        m_item_p->removeText(row, at, this->text.length());
        m_item_p->command = 0;
    }
    void addText(const QString & text)
    {
        m_item_p->addText(row, at+this->text.length(), text);
        this->text.append(text);
    }
};
class PhotoLayoutsEditor::RemoveTextUndoCommand : public QUndoCommand
{
    TextItem::TextItemPrivate * m_item_p;
    QString text;
    int row;
    int at;
public:
    RemoveTextUndoCommand(int row, int at, TextItem::TextItemPrivate * item_p, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Text edit"), parent),
        m_item_p(item_p),
        row(row),
        at(at)
    {}
    virtual void redo()
    {
        m_item_p->removeText(row, at, text.length());
    }
    virtual void undo()
    {
        m_item_p->addText(row, at, text);
        m_item_p->command = 0;
    }
    virtual void removeLeft()
    {
        text.prepend(m_item_p->m_string_list[row][--at]);
        m_item_p->m_string_list[row].remove(at, 1);
        --(m_item_p->m_cursor_character);
        m_item_p->m_item->refreshItem();
    }
    virtual void removeRight()
    {
        text.append(m_item_p->m_string_list[row][at]);
        m_item_p->m_string_list[row].remove(at, 1);
        m_item_p->m_item->refreshItem();
    }
};
class PhotoLayoutsEditor::AddLineUndoCommand : public QUndoCommand
{
    TextItem::TextItemPrivate * m_item_p;
    int row;
    int at;
public:
    AddLineUndoCommand(int row, int at, TextItem::TextItemPrivate * item_p, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Text edit"), parent),
        m_item_p(item_p),
        row(row),
        at(at)
    {}
    virtual void redo()
    {
        int length = m_item_p->m_string_list[row].length()-at;
        length = length < 0 ? 0 : length;
        QString temp = m_item_p->m_string_list[row].right( length );
        m_item_p->m_string_list[row].remove(at, length);
        m_item_p->m_cursor_character = at = 0;
        m_item_p->m_cursor_row = ++row;
        m_item_p->m_string_list.insert(row, temp);
        m_item_p->m_item->refreshItem();
        m_item_p->command = 0;
    }
    virtual void undo()
    {
        m_item_p->m_cursor_character = at = m_item_p->m_string_list[row-1].length();
        m_item_p->m_string_list[row-1].append( m_item_p->m_string_list[row] );
        m_item_p->m_string_list.removeAt(row);
        m_item_p->m_cursor_row = --row;
        m_item_p->m_item->refreshItem();
        m_item_p->command = 0;
    }
};
class PhotoLayoutsEditor::MergeLineUndoCommand : public QUndoCommand
{
    TextItem::TextItemPrivate * m_item_p;
    int row;
    int at;
public:
    MergeLineUndoCommand(int row, TextItem::TextItemPrivate * item_p, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Text edit"), parent),
        m_item_p(item_p),
        row(row),
        at(0)
    {}
    virtual void redo()
    {
        m_item_p->m_cursor_row = --row;
        m_item_p->m_cursor_character = at = m_item_p->m_string_list[row].length();
        m_item_p->m_string_list[row].append( m_item_p->m_string_list[row+1] );
        m_item_p->m_string_list.removeAt(row+1);
        m_item_p->command = 0;
        m_item_p->m_item->refreshItem();
    }
    virtual void undo()
    {
        QString temp = m_item_p->m_string_list[row].right( m_item_p->m_string_list[row].length()-at );
        m_item_p->m_string_list[row].remove(at, temp.length());
        m_item_p->m_cursor_row = ++row;
        m_item_p->m_string_list.insert(row, temp);
        m_item_p->m_cursor_character = at = 0;
        m_item_p->command = 0;
        m_item_p->m_item->refreshItem();
    }
};

void TextItem::TextItemPrivate::moveCursorLeft()
{
    --m_cursor_character;
    if (m_cursor_character < 0)
    {
        --m_cursor_row;
        if (m_cursor_row < 0)
        {
            ++m_cursor_row;
            ++m_cursor_character;
        }
        else
            m_cursor_character = m_string_list.at(m_cursor_row).length();
    }
    command = 0;
}

void TextItem::TextItemPrivate::moveCursorRight()
{
    ++m_cursor_character;
    if (m_cursor_character > m_string_list.at(m_cursor_row).length())
    {
        ++m_cursor_row;
        if (m_cursor_row >= m_string_list.count())
        {
            --m_cursor_row;
            --m_cursor_character;
        }
        else
            m_cursor_character = 0;
    }
    command = 0;
}

void TextItem::TextItemPrivate::moveCursorUp()
{
    --(m_cursor_row);
    if (m_cursor_row < 0)
        m_cursor_row = 0;
    else if (m_cursor_character > m_string_list.at(m_cursor_row).length())
        m_cursor_character = m_string_list.at(m_cursor_row).length();
    command = 0;
}

void TextItem::TextItemPrivate::moveCursorDown()
{
    ++(m_cursor_row);
    if (m_cursor_row >= m_string_list.count())
        --m_cursor_row;
    else if (m_cursor_character > m_string_list.at(m_cursor_row).length())
        m_cursor_character = m_string_list.at(m_cursor_row).length();
    command = 0;
}

void TextItem::TextItemPrivate::moveCursorEnd()
{
    m_cursor_character = m_string_list.at(m_cursor_row).length();
    command = 0;
}

void TextItem::TextItemPrivate::moveCursorHome()
{
    m_cursor_character = 0;
    command = 0;
}

void TextItem::TextItemPrivate::removeTextAfter()
{
    // Remove text from current line
    if (m_cursor_character < m_string_list.at(m_cursor_row).length())
    {
        RemoveTextUndoCommand * command = dynamic_cast<RemoveTextUndoCommand*>(this->command);
        if (!command)
        {
            this->command = command = new RemoveTextUndoCommand(m_cursor_row, m_cursor_character, this);
            PLE_PostUndoCommand(command);
        }
        command->removeRight();
    }
    // Remove current line
    else if (m_string_list.count()-1 > m_cursor_row)
    {
        PLE_PostUndoCommand( new MergeLineUndoCommand(m_cursor_row+1, this) );
    }
}

void TextItem::TextItemPrivate::removeTextBefore()
{
    // Remove text from current line
    if (m_cursor_character > 0 && m_string_list.at(m_cursor_row).length() >= m_cursor_character)
    {
        RemoveTextUndoCommand * command = dynamic_cast<RemoveTextUndoCommand*>(this->command);
        if (!command)
        {
            this->command = command = new RemoveTextUndoCommand(m_cursor_row, m_cursor_character, this);
            PLE_PostUndoCommand(command);
        }
        command->removeLeft();
    }
    // Remove current line
    else if (m_cursor_row > 0)
    {
        PLE_PostUndoCommand( new MergeLineUndoCommand(m_cursor_row, this) );
    }
}

void TextItem::TextItemPrivate::addNewLine()
{
    PLE_PostUndoCommand( new AddLineUndoCommand(m_cursor_row, m_cursor_character, this) );
}

void TextItem::TextItemPrivate::addText(const QString & text)
{
    if (!text.length())
        return;
    AddTextUndoCommand * command = dynamic_cast<AddTextUndoCommand*>(this->command);
    if (!command)
    {
        this->command = command = new AddTextUndoCommand(m_cursor_row, m_cursor_character, this);
        PLE_PostUndoCommand(command);
    }
    command->addText(text);
}

void TextItem::TextItemPrivate::addText(int row, int at, const QString & text)
{
    row = row < m_string_list.count() ? row : m_string_list.count()-1;
    row = row < 0 ? 0 : row;
    at = at < m_string_list[row].length() ? at : m_string_list[row].length();
    at = at < 0 ? 0 : at;
    m_string_list[row].insert(at, text);
    m_cursor_row = row;
    m_cursor_character = at + text.length();
    m_item->refreshItem();
}

void TextItem::TextItemPrivate::removeText(int row, int at, int length)
{
    row = row < m_string_list.count() ? row : m_string_list.count()-1;
    row = row < 0 ? 0 : row;
    at = at < m_string_list[row].length() ? at : m_string_list[row].length();
    at = at < 0 ? 0 : at;
    m_string_list[row].remove(at, length);
    m_cursor_row = row;
    m_cursor_character = at;
    m_item->refreshItem();
}

void TextItem::TextItemPrivate::closeEditor()
{
    m_item->clearFocus();
    command = 0;
}

TextItem::TextItem(const QString & text, Scene * scene) :
    AbstractPhoto((text.isEmpty() ? i18n("Text item") : text), scene),
    d(new TextItemPrivate(this)),
    m_color(DEFAULT_COLOR),
    m_font(DEFAULT_FONT),
    m_metrics(m_font)
{
    d->m_string_list = QString(text).remove(QLatin1Char('\t')).split(QLatin1Char('\n'));

    this->setFlag(QGraphicsItem::ItemIsFocusable);
    this->refresh();
}

void TextItem::focusInEvent(QFocusEvent * event)
{
    if (!this->isSelected())
    {
        this->clearFocus();
        return;
    }
    this->setCursorPositionVisible(true);
    AbstractPhoto::focusInEvent(event);
    this->setCursor(QCursor(Qt::IBeamCursor));
    this->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void TextItem::focusOutEvent(QFocusEvent * event)
{
    d->command = 0;
    this->setCursorPositionVisible(false);
    AbstractPhoto::focusOutEvent(event);
    this->unsetCursor();
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->refresh();
}

void TextItem::keyPressEvent(QKeyEvent * event)
{
    //bool textChange = false;
    switch (event->key())
    {
        case Qt::Key_Left:
            d->moveCursorLeft();
            break;
        case Qt::Key_Right:
            d->moveCursorRight();
            break;
        case Qt::Key_Up:
            d->moveCursorUp();
            break;
        case Qt::Key_Down:
            d->moveCursorDown();
            break;
        case Qt::Key_Home:
            d->moveCursorHome();
            break;
        case Qt::Key_End:
            d->moveCursorEnd();
            break;
        case Qt::Key_Return:
            d->addNewLine();
            break;
        case Qt::Key_Escape:
            d->closeEditor();
            break;
        case Qt::Key_Delete:
            d->removeTextAfter();
            //textChange = true;
            break;
        case Qt::Key_Backspace:
            d->removeTextBefore();
            //textChange = true;
            break;
        default:
            d->addText(event->text());
            //textChange = true;
    }
    refreshItem();
    event->setAccepted(true);
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QPointF p = event->pos();

    // Get clicked line number
    d->m_cursor_row =  p.y() / m_metrics.lineSpacing();
    if (d->m_cursor_row >= d->m_string_list.count())
        d->m_cursor_row = d->m_string_list.count()-1;
    QString currentLine = d->m_string_list.at( d->m_cursor_row );

    // Get clicked char position
    if (currentLine.length())
    {
        p.setX(p.x()-m_metrics.leftBearing(currentLine.at(0)));
        d->m_cursor_character = 0;
        int limit = currentLine.length();
        int width = 0;
        int rightSpace = 0;
        int leftSpace = 0;
        while (width < p.x() && d->m_cursor_character < limit)
        {
            width = m_metrics.width(currentLine, ++(d->m_cursor_character));
            rightSpace = width - p.x();
        }
        if (d->m_cursor_character > 0)
        {
            width = m_metrics.width(currentLine, --(d->m_cursor_character));
            leftSpace = p.x() - width;
        }
        if (leftSpace > rightSpace)
            ++(d->m_cursor_character);
    }
    else
        p.setX(0);

    d->command = 0;

    this->update();
}

QColor TextItem::color() const
{
    return m_color;
}

void TextItem::setColor(const QColor & color)
{
    DEFAULT_COLOR = color;
    QUndoCommand * undo = new TextColorUndoCommand(color, this);
    PLE_PostUndoCommand(undo);
}

QFont TextItem::font() const
{
    return m_font;
}

void TextItem::setFont(const QFont & font)
{
    DEFAULT_FONT = font;
    QUndoCommand * undo = new TextFontUndoCommand(font, this);
    PLE_PostUndoCommand(undo);
}

QStringList TextItem::text() const
{
    return d->m_string_list;
}

QString TextItem::textMultiline() const
{
    return d->m_string_list.join(QLatin1String("\n"));
}

void TextItem::setText(const QStringList & textList)
{
    QUndoCommand * undo = new TextChangeUndoCommand(textList, this);
    PLE_PostUndoCommand(undo);
}

void TextItem::setText(const QString & text)
{
    QString temp = text;
    temp.remove(QLatin1Char('\t'));
    this->setText(temp.split(QLatin1Char('\n')));
}

QPainterPath TextItem::itemShape() const
{
    if (cropShape().isEmpty())
        return m_complete_path;
    else
        return m_complete_path & this->cropShape();
}

QPainterPath TextItem::itemOpaqueArea() const
{
    if (cropShape().isEmpty())
        return m_text_path;
    else
        return m_text_path & this->cropShape();
}

QPainterPath TextItem::itemDrawArea() const
{
    return m_complete_path;
}

void TextItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if (!m_text_path.isEmpty())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        if (this->cropShape().isEmpty())
            painter->fillPath(m_text_path, m_color);
        else
            painter->fillPath(m_text_path & this->cropShape(), m_color);
        painter->restore();
    }

    if (d->m_cursorIsVisible)
    {
        painter->save();
        painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter->setPen(Qt::gray);
        int y = m_metrics.lineSpacing() * d->m_cursor_row;
        int x = 0;
        if ( d->m_string_list.count() > d->m_cursor_row && !d->m_string_list.at(d->m_cursor_row).isEmpty() )
        {
            x = m_metrics.width(d->m_string_list.at(d->m_cursor_row),
                                d->m_cursor_character)
                - m_metrics.leftBearing(d->m_string_list.at(d->m_cursor_row).at(0));
        }
        painter->drawLine(x, y, x, y+m_metrics.lineSpacing());
        painter->restore();
    }
    AbstractPhoto::paint(painter, option, widget);
}

QDomDocument TextItem::toSvg() const
{
    QDomDocument document = AbstractPhoto::toSvg();
    QDomElement result = document.firstChildElement();
    result.setAttribute(QLatin1String("class"), QLatin1String("TextItem"));

    // 'defs' tag
    QDomElement defs = document.createElement(QLatin1String("defs"));
    defs.setAttribute(QLatin1String("class"), QLatin1String("data"));
    result.appendChild(defs);

    // 'defs'-> ple:'data'
    QDomElement appNS = document.createElementNS(PhotoLayoutsEditor::uri(), QLatin1String("data"));
    appNS.setPrefix(PhotoLayoutsEditor::name());
    defs.appendChild(appNS);

    // 'defs'-> ple:'data' -> 'text'
    QDomElement text = document.createElement(QLatin1String("text"));
    text.appendChild(document.createTextNode(d->m_string_list.join(QLatin1Char('\n'))));
    text.setPrefix(PhotoLayoutsEditor::name());
    appNS.appendChild(text);

    // 'defs'-> ple:'data' -> 'color'
    QDomElement color = document.createElement(QLatin1String("color"));
    color.setPrefix(PhotoLayoutsEditor::name());
    color.setAttribute(QLatin1String("name"), m_color.name());
    appNS.appendChild(color);

    // 'defs'-> ple:'data' -> 'font'
    QDomElement font = document.createElement(QLatin1String("font"));
    font.setPrefix(PhotoLayoutsEditor::name());
    font.setAttribute(QLatin1String("data"), m_font.toString());
    appNS.appendChild(font);

    return document;
}

QDomDocument TextItem::toTemplateSvg() const
{
    QDomDocument document = AbstractPhoto::toSvg();
    QDomElement result = document.firstChildElement();
    result.setAttribute(QLatin1String("class"), QLatin1String("TextItem"));

    // 'defs' tag
    QDomElement defs = document.createElement(QLatin1String("defs"));
    defs.setAttribute(QLatin1String("class"), QLatin1String("data"));
    result.appendChild(defs);

    // 'defs'-> ple:'data'
    QDomElement appNS = document.createElementNS(PhotoLayoutsEditor::uri(), QLatin1String("data"));
    appNS.setPrefix(PhotoLayoutsEditor::name());
    defs.appendChild(appNS);

    // 'defs'-> ple:'data' -> 'text'
    QDomElement text = document.createElement(QLatin1String("text"));
    text.appendChild(document.createTextNode(d->m_string_list.join(QLatin1String("\n"))));
    text.setPrefix(PhotoLayoutsEditor::name());
    appNS.appendChild(text);

    // 'defs'-> ple:'data' -> 'color'
    QDomElement color = document.createElement(QLatin1String("color"));
    color.setPrefix(PhotoLayoutsEditor::name());
    color.setAttribute(QLatin1String("name"), m_color.name());
    appNS.appendChild(color);

    // 'defs'-> ple:'data' -> 'font'
    QDomElement font = document.createElement(QLatin1String("font"));
    font.setPrefix(PhotoLayoutsEditor::name());
    font.setAttribute(QLatin1String("data"), m_font.toString());
    appNS.appendChild(font);

    return document;
}

QDomDocument TextItem::svgVisibleArea() const
{
    QDomDocument document = PhotoLayoutsEditor::pathToSvg(m_text_path);
    document.firstChildElement(QLatin1String("path")).setAttribute(QLatin1String("fill"), m_color.name());
    return document;
}

QDomDocument TextItem::svgTemplateArea() const
{
    QDomDocument document = PhotoLayoutsEditor::pathToSvg(m_text_path);
    document.firstChildElement(QLatin1String("path")).setAttribute(QLatin1String("fill"), m_color.name());
    return document;
}

TextItem * TextItem::fromSvg(QDomElement & element)
{
    TextItem * result = new TextItem();
    if (result->AbstractPhoto::fromSvg(element))
    {
        QDomElement defs = element.firstChildElement(QLatin1String("defs"));
        while (!defs.isNull() && defs.attribute(QLatin1String("class")) != QLatin1String("data"))
            defs = defs.nextSiblingElement(QLatin1String("defs"));
        IS_NULL(defs);

        QDomElement data = defs.firstChildElement(QLatin1String("data"));
        IS_NULL(data);

        // text
        QDomElement text = data.firstChildElement(QLatin1String("text"));
        IS_NULL(text);
        QDomNode textValue = text.firstChild();
        while (!textValue.isNull() && !textValue.isText())
            textValue = textValue.nextSibling();
        IS_NULL(textValue);
        result->d->m_string_list = textValue.toText().data().remove(QLatin1Char('\t')).split(QLatin1Char('\n'));

        // Color
        QDomElement color = data.firstChildElement(QLatin1String("color"));
        IS_NULL(color);
        result->m_color = QColor(color.attribute(QLatin1String("name")));

        // Font
        QDomElement font = data.firstChildElement(QLatin1String("font"));
        IS_NULL(font);
        result->m_font.fromString(font.attribute(QLatin1String("data")));

        result->refresh();
        return result;
    }
_delete:
    delete result;
    return 0;
}

void TextItem::refreshItem()
{
    m_metrics = QFontMetrics(m_font);
    m_text_path = QPainterPath();
    int i = 1;
    int maxBearing = 0;
    int maxWidth = 0;
    const int lineSpacing = m_metrics.lineSpacing();
    foreach(QString string, d->m_string_list)
    {
        if (string.length())
        {
            int width = m_metrics.width(string);
            int leftBearing = -m_metrics.leftBearing(string.at(0));
            m_text_path.addText(leftBearing,
                                lineSpacing*(i)-m_metrics.descent(),
                                m_font,
                                string);
            if (maxWidth < width)
                maxWidth = width;
            if (maxBearing < leftBearing)
                maxBearing = leftBearing;
        }
        ++i;
    }
    if (maxWidth == 0)
        maxWidth = 1;
    m_complete_path = QPainterPath();
    m_complete_path.addRect(0,
                            0,
                            maxWidth + maxBearing,
                            d->m_string_list.count() * m_metrics.lineSpacing());
    this->prepareGeometryChange();
    this->updateIcon();
}

QtAbstractPropertyBrowser * TextItem::propertyBrowser()
{
    QtTreePropertyBrowser * browser = new QtTreePropertyBrowser();

    // Color
    QtColorPropertyManager * colorManager = new QtColorPropertyManager(browser);
    KColorEditorFactory * colorFactory = new KColorEditorFactory(browser);
    browser->setFactoryForManager(colorManager, colorFactory);
    QtProperty * colorProperty = colorManager->addProperty(i18n("Text color"));
    colorManager->setValue(colorProperty, m_color);
    browser->addProperty(colorProperty);
    TextColorChangeListener * colorListener = new TextColorChangeListener(this);
    colorListener->connect(browser, SIGNAL(destroyed()), SLOT(deleteLater()));
    colorListener->connect(colorManager, SIGNAL(propertyChanged(QtProperty*)), SLOT(propertyChanged(QtProperty*)));
    foreach(QtProperty* p, colorProperty->subProperties())
        p->setEnabled(false);

    // Font
    QtFontPropertyManager * fontManager = new QtFontPropertyManager(browser);
    KFontEditorFactory * fontFactory = new KFontEditorFactory(browser);
    browser->setFactoryForManager(fontManager, fontFactory);
    QtProperty * fontProperty = fontManager->addProperty(i18n("Font"));
    fontManager->setValue(fontProperty, m_font);
    browser->addProperty(fontProperty);
    TextFontChangeListener * fontListener = new TextFontChangeListener(this);
    fontListener->connect(browser, SIGNAL(destroyed()), SLOT(deleteLater()));
    fontListener->connect(fontManager, SIGNAL(propertyChanged(QtProperty*)), SLOT(propertyChanged(QtProperty*)));
    foreach(QtProperty* p, fontProperty->subProperties())
        p->setEnabled(false);

    return browser;
}

QPainterPath TextItem::getLinePath(const QString & string)
{
    QPainterPath result;
    result.addText(0, 0, m_font, string);
    return result;
}

void TextItem::setCursorPositionVisible(bool isVisible)
{
    d->m_cursorIsVisible = isVisible;
    this->update();
}

void TextItem::updateIcon()
{
    QPixmap px(50, 50);
    px.fill(Qt::transparent);
    QPainter p(&px);
    QFont f = this->font();
    f.setPixelSize(40);
    p.setFont(f);
    p.drawText(px.rect(), Qt::AlignCenter, QLatin1String("T"));
    this->setIcon(QIcon(px));
}
