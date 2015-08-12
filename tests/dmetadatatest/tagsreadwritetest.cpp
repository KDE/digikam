#include "tagsreadwritetest.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QTest>
#include <QStringList>
#include <QString>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

QTEST_MAIN(TagsReadWriteTest)

void TagsReadWriteTest::testSimpleReadAfterWrite()
{
    DMetadata dmeta;
    QStringList tagPaths;
    QStringList tagPaths2;

    tagPaths << QLatin1String("/root/child1/child2");
    tagPaths << QLatin1String("/root/extra/child2/triple");
    tagPaths << QLatin1String("/root/extra/ch223/triple");

    dmeta.setImageTagsPath(tagPaths);

    dmeta.getImageTagsPath(tagPaths2);

    QCOMPARE(tagPaths, tagPaths2);
}


