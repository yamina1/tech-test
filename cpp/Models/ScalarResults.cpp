#include "ScalarResults.h"
#include <stdexcept>

/*ScalarResults::~ScalarResults() = default;

std::optional<ScalarResult> ScalarResults::operator[](const std::string& tradeId) const {
    if (!containsTrade(tradeId)) {
        return std::nullopt;
    }

    std::optional<double> priceResult = std::nullopt;
    std::optional<std::string> error = std::nullopt;

    auto resultIt = results_.find(tradeId);
    if (resultIt != results_.end()) {
        priceResult = resultIt->second;
    }

    auto errorIt = errors_.find(tradeId);
    if (errorIt != errors_.end()) {
        error = errorIt->second;
    }

    return ScalarResult(tradeId, priceResult, error);
}

bool ScalarResults::containsTrade(const std::string& tradeId) const {
    return results_.find(tradeId) != results_.end() || errors_.find(tradeId) != errors_.end();
}

void ScalarResults::addResult(const std::string& tradeId, double result) {
    results_[tradeId] = result;
}

void ScalarResults::addError(const std::string& tradeId, const std::string& error) {
    errors_[tradeId] = error;
}

ScalarResults::Iterator& ScalarResults::Iterator::operator++() {
    throw std::runtime_error("Iterator not implemented");
}

ScalarResult ScalarResults::Iterator::operator*() const {
    throw std::runtime_error("Iterator not implemented");
}

bool ScalarResults::Iterator::operator!=(const Iterator& other) const {
    throw std::runtime_error("Iterator not implemented");
}

ScalarResults::Iterator ScalarResults::begin() const {
    throw std::runtime_error("Not implemented");
}

ScalarResults::Iterator ScalarResults::end() const {
    throw std::runtime_error("Not implemented");
}*/

//==========================

/*
 * Destructor
 * Nothing special to clean up.
*/ 
ScalarResults::~ScalarResults() = default;

/*
 * operator[]
 *
 * Reuse:
 * This function already merges results_ and errors_ into a ScalarResult.
 * The iterator will reuse this instead of duplicating merge logic.
 */
std::optional<ScalarResult> ScalarResults::operator[](const std::string& tradeId) const {
    if (!containsTrade(tradeId)) {
        return std::nullopt;
    }

    std::optional<double> value;
    std::optional<std::string> error;

    auto rIt = results_.find(tradeId);
    if (rIt != results_.end()) {
        value = rIt->second;
    }

    auto eIt = errors_.find(tradeId);
    if (eIt != errors_.end()) {
        error = eIt->second;
    }

    return ScalarResult(tradeId, value, error);
}

/*
 * containsTrade
 *
 * Simple helper reused by operator[].
 */
bool ScalarResults::containsTrade(const std::string& tradeId) const {
    return results_.find(tradeId) != results_.end()
        || errors_.find(tradeId) != errors_.end();
}

/*
 * addResult
 *
 * Stores a successful pricing result.
 */
void ScalarResults::addResult(const std::string& tradeId, double result) {
    results_[tradeId] = result;
}

/*
 * addError
 *
 * Stores a pricing error.
 */
void ScalarResults::addError(const std::string& tradeId, const std::string& error) {
    errors_[tradeId] = error;
}

/*
 * getAllTradeIds
 *
 * Reuse helper for Exercise 5:
 * Builds a list of unique tradeIds from results_ and errors_.
 * This avoids duplicating logic inside the iterator.
 */
std::vector<std::string> ScalarResults::getAllTradeIds() const {
    std::vector<std::string> ids;

    // First add tradeIds that have results
    for (const auto& r : results_) {
        ids.push_back(r.first);
    }

    // Then add tradeIds that only have errors
    for (const auto& e : errors_) {
        if (results_.find(e.first) == results_.end()) {
            ids.push_back(e.first);
        }
    }

    return ids;
}

/*
 * ===== Iterator implementation (Exercise 5) =====
 *
 * Purpose:
 * Allow ScalarResults to be used in a range-for loop.
 *
 * Reuse strategy:
 * - Use existing storage (results_ and errors_)
 * - Use getAllTradeIds() to enumerate tradeIds
 * - Use operator[] to build ScalarResult
 * No logic is duplicated.
 */

ScalarResults::Iterator::Iterator(const ScalarResults* parent, size_t index)
    : parent_(parent), index_(index) {}

ScalarResults::Iterator& ScalarResults::Iterator::operator++() {
    ++index_;
    return *this;
}

ScalarResult ScalarResults::Iterator::operator*() const {
    // Reuse: delegate ScalarResult creation to operator[]
    auto ids = parent_->getAllTradeIds();
    return parent_->operator[](ids[index_]).value();
}

bool ScalarResults::Iterator::operator!=(const Iterator& other) const {
    // Range-for only checks against end()
    return index_ != other.index_;
}

/*
 * begin / end
 *
 * Entry points for range-for iteration.
 */
ScalarResults::Iterator ScalarResults::begin() const {
    return Iterator(this, 0);
}

ScalarResults::Iterator ScalarResults::end() const {
    return Iterator(this, getAllTradeIds().size());
}

