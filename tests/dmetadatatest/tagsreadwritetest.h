#ifndef TAGSREADWRITETEST_H
#define TAGSREADWRITETEST_H

// Qt includes

#include <QObject>
#include <QStringList>

class TagsReadWriteTest : public QObject
{
    Q_OBJECT
public:


private Q_SLOTS:

    void initTestCase();
    /**
     * @brief testSimpleReadAfterWrite - default read and write
     * Description:
     * Load default values, write then read a set of tags
     * Results: Values must match
     */
    void testSimpleReadAfterWrite();

    /**
     * @brief testWriteToDisabledNamespaces - test if implementation
     *                                     will not write tags to disabled namespaces
     * Description: make a custom settings container with one disabled and one
     *     enabled namespace. Call setImageTagPaths. Read the result of both
     *     namespaces
     * Results: The result of read from disabled namespace should be empty
     *          The result of read from other namespace should be initial tag paths
     */
    void testWriteToDisabledNamespaces();

    /**
     * @brief testReadFromDisabledNamespaces - test if disabled namespaces are ignored
     * Description: Write tagSet1 to first, disable namespace, write tagSet2 to second
     *              enabled namespace
     * Results: The call of getImageTagsPaths should return tagSet2
     */
    void testReadFromDisabledNamespaces();

    /**
     * @brief testTagSeparatorWrite - test if implementation replace tag separator on write
     */
    void testTagSeparatorWrite();

    /**
     * @brief testTagSeparatorRead - test if implementation replace tag separator on read
     * Description:
     * Results:
     */
    void testTagSeparatorRead();

    /**
     * @brief testTagReadAlternativeNameSpace - test if implementation read the alternative
     *                                          namespace if default has no tags
     * Description:
     * Results:
     */
    void testTagReadAlternativeNameSpace();


private:
    QStringList tagSet1;
    QStringList tagSet2;
    QStringList tagSet3;
};

#endif /* TAGSREADWRITETEST_H */
