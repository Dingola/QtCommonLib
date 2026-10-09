/**
 * @file SqlStatementBatch.cpp
 * @brief Implements ordered SQL statement execution with failure context.
 */

#include "QtCommonLib/Sql/SqlStatementBatch.h"

#include <QSqlQuery>
#include <utility>

namespace QtCommonLib
{

/**
 * @brief Creates a reusable batch that owns its ordered SQL statements.
 * @param statements Ordered SQL statements retained by the batch.
 */
SqlStatementBatch::SqlStatementBatch(QStringList statements): m_statements(std::move(statements)) {}

/**
 * @brief Validates and executes the owned statements in their list order.
 * @param database Database connection used for every statement.
 * @return Batch result containing failure context when a statement fails.
 */
auto SqlStatementBatch::execute(const QSqlDatabase& database) const -> SqlStatementBatchResult
{
    SqlStatementBatchResult result;
    for (qsizetype index = 0; index < m_statements.size() && result.successful; ++index)
    {
        const QString& statement = m_statements.at(index);
        if (statement.trimmed().isEmpty())
        {
            result.successful = false;
            result.failed_statement_index = index;
            result.failed_statement = statement;
            result.error =
                QSqlError(QStringLiteral("SQL statement batch contains an empty statement."),
                          QString(), QSqlError::StatementError);
        }
    }

    if (result.successful && (!database.isValid() || !database.isOpen()))
    {
        result.successful = false;
        result.error = database.lastError();
        if (result.error.type() == QSqlError::NoError)
        {
            result.error =
                QSqlError(QStringLiteral("SQL statement batch requires an open database."),
                          QString(), QSqlError::ConnectionError);
        }
    }

    if (result.successful)
    {
        QSqlQuery query(database);
        for (qsizetype index = 0; index < m_statements.size() && result.successful; ++index)
        {
            const QString& statement = m_statements.at(index);
            result.successful = query.exec(statement);
            if (!result.successful)
            {
                result.failed_statement_index = index;
                result.failed_statement = statement;
                result.error = query.lastError();
            }
        }
    }
    return result;
}

}  // namespace QtCommonLib
