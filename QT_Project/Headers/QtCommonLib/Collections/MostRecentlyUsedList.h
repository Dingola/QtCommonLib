#pragma once

#include <QVector>
#include <QtGlobal>
#include <functional>
#include <utility>

namespace QtCommonLib
{
/**
 * @file MostRecentlyUsedList.h
 * @brief Declares a key-based, bounded most-recently-used value collection.
 */

/**
 * @class MostRecentlyUsedList
 * @brief Stores unique values from most recently used to least recently used.
 * @tparam T Stored value type.
 * @tparam Key Stable key type used to identify values.
 *
 * The collection owns no persistence or domain behavior. Inserting an existing key replaces its
 * value and moves it to the front. A maximum size of zero keeps the collection unbounded.
 */
template<typename T, typename Key = T>
class MostRecentlyUsedList final
{
    public:
        /**
         * @brief Function type that extracts the stable key from a stored value.
         */
        using KeySelector = std::function<Key(const T&)>;

        /**
         * @brief Function type that compares two stable keys for equality.
         */
        using KeyEqual = std::function<bool(const Key&, const Key&)>;

        /**
         * @brief Constructs an empty MRU collection.
         * @param key_selector Function returning the stable key for a value.
         * @param maximum_size Maximum retained item count, or zero for no limit.
         * @param key_equal Function comparing two keys for equality.
         */
        explicit MostRecentlyUsedList(KeySelector key_selector, qsizetype maximum_size = 0,
                                      KeyEqual key_equal = std::equal_to<Key>{})
            : m_key_selector(std::move(key_selector)),
              m_key_equal(std::move(key_equal)),
              m_maximum_size(normalize_maximum_size(maximum_size))
        {}

        /**
         * @brief Returns the stored values in most-recently-used order.
         * @return Read-only reference to the owned values.
         */
        [[nodiscard]] auto items() const -> const QVector<T>&
        {
            return m_items;
        }

        /**
         * @brief Returns the current number of stored values.
         * @return Number of stored values.
         */
        [[nodiscard]] auto size() const -> qsizetype
        {
            return m_items.size();
        }

        /**
         * @brief Reports whether the collection contains no values.
         * @return True when the collection is empty; otherwise false.
         */
        [[nodiscard]] auto is_empty() const -> bool
        {
            return m_items.isEmpty();
        }

        /**
         * @brief Returns the configured maximum number of retained values.
         * @return Maximum value count, or zero when the collection is unbounded.
         */
        [[nodiscard]] auto maximum_size() const -> qsizetype
        {
            return m_maximum_size;
        }

        /**
         * @brief Changes and immediately enforces the maximum retained value count.
         * @param maximum_size Maximum count, or zero for no limit.
         */
        auto set_maximum_size(qsizetype maximum_size) -> void
        {
            m_maximum_size = normalize_maximum_size(maximum_size);
            enforce_maximum_size();
        }

        /**
         * @brief Inserts or replaces a value and marks it as most recently used.
         * @param value Value to insert or update.
         */
        auto insert_or_update(T value) -> void
        {
            const Key key = m_key_selector(value);
            const qsizetype existing_index = find_index(key);
            if (existing_index >= 0)
            {
                m_items.removeAt(existing_index);
            }
            m_items.prepend(std::move(value));
            enforce_maximum_size();
        }

        /**
         * @brief Replaces the collection with an already ordered MRU sequence.
         * @param values Values ordered from most recently used to least recently used.
         *
         * For duplicate keys, the first value in the supplied order is retained.
         */
        auto assign(QVector<T> values) -> void
        {
            m_items.clear();
            m_items.reserve(values.size());
            for (T& value: values)
            {
                const Key key = m_key_selector(value);
                if (find_index(key) < 0)
                {
                    m_items.append(std::move(value));
                }
            }
            enforce_maximum_size();
        }

        /**
         * @brief Removes the value identified by a key.
         * @param key Key identifying the value to remove.
         * @return True when a value was removed.
         */
        auto remove(const Key& key) -> bool
        {
            const qsizetype index = find_index(key);
            const bool removed = index >= 0;
            if (removed)
            {
                m_items.removeAt(index);
            }
            return removed;
        }

        /**
         * @brief Removes all stored values.
         */
        auto clear() -> void
        {
            m_items.clear();
        }

        /**
         * @brief Reports whether a key is present.
         * @param key Key to locate.
         * @return True when the key is present.
         */
        [[nodiscard]] auto contains(const Key& key) const -> bool
        {
            const bool result = find_index(key) >= 0;
            return result;
        }

    private:
        /**
         * @brief Converts negative limits to the documented unbounded value.
         * @param maximum_size Requested maximum size.
         * @return Non-negative maximum size.
         */
        [[nodiscard]] static auto normalize_maximum_size(qsizetype maximum_size) -> qsizetype
        {
            const qsizetype result = maximum_size > 0 ? maximum_size : 0;
            return result;
        }

        /**
         * @brief Finds a value by key.
         * @param key Key to locate.
         * @return Matching index, or minus one when absent.
         */
        [[nodiscard]] auto find_index(const Key& key) const -> qsizetype
        {
            qsizetype result = -1;
            for (qsizetype index = 0; index < m_items.size() && result < 0; ++index)
            {
                if (m_key_equal(m_key_selector(m_items.at(index)), key))
                {
                    result = index;
                }
            }
            return result;
        }

        /**
         * @brief Removes least-recently-used values until the configured limit is satisfied.
         */
        auto enforce_maximum_size() -> void
        {
            while (m_maximum_size > 0 && m_items.size() > m_maximum_size)
            {
                m_items.removeLast();
            }
        }

    private:
        KeySelector m_key_selector;
        KeyEqual m_key_equal;
        qsizetype m_maximum_size{0};
        QVector<T> m_items;
};
}  // namespace QtCommonLib
