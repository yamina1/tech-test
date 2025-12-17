/*#ifndef SCALARRESULTS_H
#define SCALARRESULTS_H

#include "IScalarResultReceiver.h"
#include "ScalarResult.h"
#include <map>
#include <vector>
#include <optional>
#include <string>
#include <iterator>

class ScalarResults : public IScalarResultReceiver {
public:
    virtual ~ScalarResults();
    std::optional<ScalarResult> operator[](const std::string& tradeId) const;

    bool containsTrade(const std::string& tradeId) const;

    virtual void addResult(const std::string& tradeId, double result) override;

    virtual void addError(const std::string& tradeId, const std::string& error) override;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ScalarResult;
        using difference_type = std::ptrdiff_t;
        using pointer = ScalarResult*;
        using reference = ScalarResult&;

        Iterator() = default;

        // Iterator must be constructable from ScalarResults parent
        Iterator& operator++();
        ScalarResult operator*() const;
        bool operator!=(const Iterator& other) const;
    };

    Iterator begin() const;
    Iterator end() const;

private:
    std::map<std::string, double> results_;
    std::map<std::string, std::string> errors_;
};*/

#ifndef SCALARRESULTS_H
#define SCALARRESULTS_H

#include "IScalarResultReceiver.h"
#include "ScalarResult.h"

#include <map>
#include <vector>
#include <optional>
#include <string>

class ScalarResults : public IScalarResultReceiver {
public:

     virtual ~ScalarResults();
    // Reuse: already-existing merge logic for results + errors
    std::optional<ScalarResult> operator[](const std::string& tradeId) const;

    bool containsTrade(const std::string& tradeId) const;
    // Reuse: existing insertion points used by pricing system
    void addResult(const std::string& tradeId, double result);
    void addError(const std::string& tradeId, const std::string& error);

    /*
     * Iterator
     *
     * Simple forward iterator so ScalarResults can be used in range-for.
     * It reuses existing storage (results_ / errors_) and operator[].
     */
    class Iterator {
    public:
        Iterator(const ScalarResults* parent, size_t index);

        Iterator& operator++();
        ScalarResult operator*() const;
        bool operator!=(const Iterator& other) const;

    private:
        // Reuse: iterator only stores parent + index
        const ScalarResults* parent_;
        size_t index_;
    };

    // Entry points for range-for
    Iterator begin() const;
    Iterator end() const;

private:
    // Existing storage (unchanged)
    std::map<std::string, double> results_;
    std::map<std::string, std::string> errors_;

    // Reuse helper: builds union of tradeIds from results_ and errors_
    std::vector<std::string> getAllTradeIds() const;
};

#endif // SCALARRESULTS_H
