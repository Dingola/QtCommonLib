#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QtTypes>

/**
 * @file SqlStatementBatch.h
 * @brief Declares ordered execution of multiple SQL statements.
 */

namespace QtCommonLib
{

/**
 * @struct SqlStatementBatchResult
 * @brief Describes the outcome of an ordered SQL statement batch.
 */
struct SqlStatementBatchResult {
        /** @brief Whether every statement completed successfully. */
        bool successful{true};

        /** @brief Zero-based index of the invalid or failed statement, or -1 when unavailable. */
        qsizetype failed_statement_index{-1};

        /** @brief Statement that was invalid or failed, or an empty string when unavailable. */
        QString failed_statement;

        /** @brief Validation, connection, or execution error, or no error after success. */
        QSqlError error;
};

/**
 * @class SqlStatementBatch
 * @brief Executes SQL statements in order and stops after the first failure.
 *
 * Before execution, the batch rejects empty statements and requires a valid open database.
 * SQL syntax and schema-dependent semantics are validated by the selected database driver during
 * execution. Transaction ownership and error logging remain responsibilities of the caller.
 */
class SqlStatementBatch final
{
    public:
        /**
         * @brief Creates a reusable batch that owns its ordered SQL statements.
         * @param statements Ordered SQL statements retained by the batch.
         */
        explicit SqlStatementBatch(QStringList statements);

        /**
         * @brief Validates and executes the owned statements in their list order.
         * @param database Database connection used for every statement.
         * @return Batch result containing failure context when a statement fails.
         */
        [[nodiscard]] auto execute(const QSqlDatabase& database) const -> SqlStatementBatchResult;

    private:
        /** @brief Ordered SQL statements owned by this reusable batch. */
        QStringList m_statements;
};

}  // namespace QtCommonLib
