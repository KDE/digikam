#ifndef TAGSREADWRITETEST_H
#define TAGSREADWRITETEST_H

// Qt includes

#include <QObject>

class TagsReadWriteTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSimpleReadAfterWrite();
};

#endif /* TAGSREADWRITETEST_H */
