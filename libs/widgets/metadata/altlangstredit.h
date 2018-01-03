/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALTLANGSTREDIT_H
#define ALTLANGSTREDIT_H

// Qt includes

#include <QWidget>
#include <QString>
#include <QTextEdit>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"

namespace Digikam
{

class DIGIKAM_EXPORT AltLangStrEdit : public QWidget
{
    Q_OBJECT

public:

    AltLangStrEdit(QWidget* const parent);
    ~AltLangStrEdit();

    void setTitle(const QString& title);
    void setPlaceholderText(const QString& msg);

    void    setCurrentLanguageCode(const QString& lang);
    QString currentLanguageCode() const;

    QString languageCode(int index) const;

    void setValues(const MetaEngine::AltLangMap& values);
    MetaEngine::AltLangMap& values() const;

    /**
     * Fix lines visibile in text editor to lines. If zero, do not fix layout to number of lines visible.
     */
    void setLinesVisible(uint lines);
    uint linesVisible() const;

    QString defaultAltLang()   const;
    bool    asDefaultAltLang() const;

    /**
     * Reset widget, clear all entries
     */
    void reset();

    /**
     * Ensure that the current language is added to the list of entries,
     * even if the text is empty.
     * signalValueAdded() will be emitted.
     */
    void addCurrent();

    QTextEdit* textEdit() const;

Q_SIGNALS:

    /// Emitted when the user changes the text for the current language.
    void signalModified(const QString& lang, const QString& text);

    /// Emitted when the current language changed.
    void signalSelectionChanged(const QString& lang);

    /// Emitted when an entry for a new language is added.
    void signalValueAdded(const QString& lang, const QString& text);

    /// Emitted when the entry for a language is removed.
    void signalValueDeleted(const QString& lang);

protected Q_SLOTS:

    void slotTextChanged();
    void slotSelectionChanged();
    void slotDeleteValue();

protected:

    void loadLangAltListEntries();

    virtual void changeEvent(QEvent* e);

private:

    class Private;
    Private* const d;

    friend class Private;
};

} // namespace Digikam

#endif // ALTLANGSTREDIT_H
