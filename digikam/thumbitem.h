#ifndef THUMBITEM_H
#define THUMBITEM_H

#include <qrect.h>
#include <qstring.h>

class QPixmap;
class QPainter;
class QColorGroup;

class ThumbView;
class ThumbItemLineEdit;
class ThumbItemPrivate;

class ThumbItem {

    friend class ThumbView;
    friend class ThumbItemLineEdit;

public:

    ThumbItem(ThumbView* parent,
              const QString& text,
              const QPixmap& pixmap);
    virtual ~ThumbItem();

    QPixmap *pixmap() const;
    QString text() const;

    ThumbItem *nextItem();
    ThumbItem *prevItem();
    
    int x() const;
    int y() const;
    int width() const;
    int height() const;

    QRect rect();
    QRect textRect(bool relative=true);
    QRect pixmapRect(bool relative=true);

    bool move(int x, int y);

    void setSelected(bool val, bool cb=true);
    bool isSelected();

    virtual void setPixmap(const QPixmap& pixmap);
    virtual void setText(const QString& text);
    void repaint();
    
    ThumbView* iconView();

    void rename();
    virtual int compare(ThumbItem *item);

    virtual QString key() const;
    
protected:

    virtual void calcRect();
    void setRect(const QRect& rect);
    void setTextRect(const QRect& rect);
    void setPixmapRect(const QRect& rect);

    virtual void paintItem(QPainter *p, const QColorGroup& cg);

    void renameItem();
    void cancelRenameItem();

private:

    ThumbItemPrivate *d;
    ThumbView *view;
    ThumbItem *next;
    ThumbItem *prev;
    ThumbItemLineEdit *renameBox;


};

#endif
