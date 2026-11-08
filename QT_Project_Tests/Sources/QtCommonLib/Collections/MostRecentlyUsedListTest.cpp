/**
 * @file MostRecentlyUsedListTest.cpp
 * @brief Tests key-based ordering, deduplication, limits, and removal.
 */

#include "QtCommonLib/Collections/MostRecentlyUsedListTest.h"

#include <QString>
#include <QVector>

#include "QtCommonLib/Collections/MostRecentlyUsedList.h"

namespace
{
/**
 * @struct Entry
 * @brief Represents a test value with an independently selectable identity.
 */
struct Entry {
        QString key;    ///< Stable identity used by the MRU collection.
        QString value;  ///< Payload associated with the stable identity.
};

/**
 * @brief Creates an MRU collection keyed by Entry::key.
 * @param maximum_size Maximum retained value count.
 * @return Empty collection configured for Entry values.
 */
[[nodiscard]] auto make_list(qsizetype maximum_size = 0)
    -> QtCommonLib::MostRecentlyUsedList<Entry, QString>
{
    QtCommonLib::MostRecentlyUsedList<Entry, QString> result(
        [](const Entry& entry) { return entry.key; }, maximum_size);
    return result;
}
}  // namespace

/**
 * @test Verifies that existing keys are replaced and moved to the most-recent position.
 */
TEST_F(MostRecentlyUsedListTest, InsertsUpdatesAndMovesValuesToFront)
{
    auto list = make_list();
    list.insert_or_update({QStringLiteral("a"), QStringLiteral("first")});
    list.insert_or_update({QStringLiteral("b"), QStringLiteral("second")});
    list.insert_or_update({QStringLiteral("a"), QStringLiteral("updated")});

    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list.items().at(0).key, QStringLiteral("a"));
    EXPECT_EQ(list.items().at(0).value, QStringLiteral("updated"));
    EXPECT_EQ(list.items().at(1).key, QStringLiteral("b"));
}

/**
 * @test Verifies that the configured limit evicts least-recent values and can shrink at runtime.
 */
TEST_F(MostRecentlyUsedListTest, EnforcesMaximumSize)
{
    auto list = make_list(3);
    list.insert_or_update({QStringLiteral("a"), QString()});
    list.insert_or_update({QStringLiteral("b"), QString()});
    list.insert_or_update({QStringLiteral("c"), QString()});
    list.insert_or_update({QStringLiteral("d"), QString()});

    ASSERT_EQ(list.size(), 3);
    EXPECT_FALSE(list.contains(QStringLiteral("a")));
    EXPECT_EQ(list.items().at(0).key, QStringLiteral("d"));

    list.set_maximum_size(1);

    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.items().front().key, QStringLiteral("d"));
}

/**
 * @test Verifies that persisted order is retained while duplicate keys are discarded.
 */
TEST_F(MostRecentlyUsedListTest, AssignsOrderedUniqueValues)
{
    auto list = make_list(2);
    list.assign({{QStringLiteral("a"), QStringLiteral("newest")},
                 {QStringLiteral("b"), QStringLiteral("middle")},
                 {QStringLiteral("a"), QStringLiteral("duplicate")},
                 {QStringLiteral("c"), QStringLiteral("oldest")}});

    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list.items().at(0).value, QStringLiteral("newest"));
    EXPECT_EQ(list.items().at(1).key, QStringLiteral("b"));
}

/**
 * @test Verifies that key equality is independently configurable from the stored value type.
 */
TEST_F(MostRecentlyUsedListTest, UsesConfiguredKeyEquality)
{
    QtCommonLib::MostRecentlyUsedList<Entry, QString> list(
        [](const Entry& entry) { return entry.key; }, 0,
        [](const QString& left, const QString& right) {
            return left.compare(right, Qt::CaseInsensitive) == 0;
        });
    list.insert_or_update({QStringLiteral("Log.LOG"), QStringLiteral("first")});
    list.insert_or_update({QStringLiteral("log.log"), QStringLiteral("updated")});

    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.items().front().value, QStringLiteral("updated"));
    EXPECT_TRUE(list.contains(QStringLiteral("LOG.log")));
}

/**
 * @test Verifies that values can be removed individually or cleared as a group.
 */
TEST_F(MostRecentlyUsedListTest, RemovesAndClearsValues)
{
    auto list = make_list();
    list.insert_or_update({QStringLiteral("a"), QString()});
    list.insert_or_update({QStringLiteral("b"), QString()});

    EXPECT_TRUE(list.remove(QStringLiteral("a")));
    EXPECT_FALSE(list.remove(QStringLiteral("missing")));
    ASSERT_EQ(list.size(), 1);

    list.clear();

    EXPECT_TRUE(list.is_empty());
}
