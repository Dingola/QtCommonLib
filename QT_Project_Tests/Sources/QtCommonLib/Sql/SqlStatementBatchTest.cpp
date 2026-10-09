/**
 * @file SqlStatementBatchTest.cpp
 * @brief Verifies SQL statement validation, ordered execution, and failure reporting.
 */

#include <gtest/gtest.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "QtCommonLib/Sql/SqlConnection.h"
#include "QtCommonLib/Sql/SqlStatementBatch.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

using QtCommonLib::SqlConnection;
using QtCommonLib::SqlStatementBatch;
using QtCommonLib::SqlStatementBatchResult;
using QtCommonLib::TestFileSystem;

/** @test Verifies that every statement is executed in the provided order. */
TEST(SqlStatementBatchTest, ExecutesStatementsInOrder)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path = file_system.nonexistent_path(QStringLiteral("batch.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqlConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    const QStringList statements{
        QStringLiteral("CREATE TABLE records(id INTEGER PRIMARY KEY, value TEXT NOT NULL)"),
        QStringLiteral("INSERT INTO records(value) VALUES('first')"),
        QStringLiteral("INSERT INTO records(value) VALUES('second')")};

    const SqlStatementBatch batch(statements);
    const SqlStatementBatchResult result = batch.execute(connection.database());

    EXPECT_TRUE(result.successful);
    EXPECT_EQ(result.failed_statement_index, -1);
    EXPECT_TRUE(result.failed_statement.isEmpty());
    EXPECT_EQ(result.error.type(), QSqlError::NoError);
    QSqlQuery query(connection.database());
    ASSERT_TRUE(query.exec(QStringLiteral("SELECT value FROM records ORDER BY id")));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), QStringLiteral("first"));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), QStringLiteral("second"));
}

/** @test Verifies that execution stops at the first failure and reports its context. */
TEST(SqlStatementBatchTest, StopsAtFirstFailureAndReportsContext)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path =
        file_system.nonexistent_path(QStringLiteral("failed-batch.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqlConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    const QString failed_statement = QStringLiteral("INVALID SQL");
    const QStringList statements{
        QStringLiteral("CREATE TABLE records(value TEXT NOT NULL)"), failed_statement,
        QStringLiteral("INSERT INTO records(value) VALUES('not-executed')")};

    const SqlStatementBatch batch(statements);
    const SqlStatementBatchResult result = batch.execute(connection.database());

    EXPECT_FALSE(result.successful);
    EXPECT_EQ(result.failed_statement_index, 1);
    EXPECT_EQ(result.failed_statement, failed_statement);
    EXPECT_NE(result.error.type(), QSqlError::NoError);
    QSqlQuery query(connection.database());
    ASSERT_TRUE(query.exec(QStringLiteral("SELECT COUNT(*) FROM records")));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 0);
}

/** @test Verifies that empty statements reject the complete batch before execution starts. */
TEST(SqlStatementBatchTest, RejectsEmptyStatementBeforeExecution)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path =
        file_system.nonexistent_path(QStringLiteral("invalid-batch.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqlConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    const QStringList statements{QStringLiteral("CREATE TABLE records(value TEXT NOT NULL)"),
                                 QStringLiteral("   ")};
    const SqlStatementBatch batch(statements);

    const SqlStatementBatchResult result = batch.execute(connection.database());

    EXPECT_FALSE(result.successful);
    EXPECT_EQ(result.failed_statement_index, 1);
    EXPECT_EQ(result.failed_statement, QStringLiteral("   "));
    EXPECT_EQ(result.error.type(), QSqlError::StatementError);
    QSqlQuery query(connection.database());
    ASSERT_TRUE(query.exec(QStringLiteral(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='records'")));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 0);
}
