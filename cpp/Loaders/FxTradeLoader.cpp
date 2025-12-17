/*#include "FxTradeLoader.h"
#include <stdexcept>

// NOTE: These methods are only here to allow the solution to compile prior to the test being completed.

std::vector<ITrade*> FxTradeLoader::loadTrades() {
    throw std::runtime_error("Not implemented");
}

std::string FxTradeLoader::getDataFile() const {
    throw std::runtime_error("Not implemented");
}

void FxTradeLoader::setDataFile(const std::string& file) {
    throw std::runtime_error("Not implemented");
}*/

#include "FxTradeLoader.h"
#include "../Models/FxTrade.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

/*
 * Reused from BondTradeLoader:
 * Trim whitespace + Windows CR/LF. Prevents hidden '\r' from breaking string equality tests.
 */
static inline void trim_in_place(std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) { s.clear(); return; }
    auto e = s.find_last_not_of(ws);
    s = s.substr(b, e - b + 1);
}

/*
 * FxTrades.dat delimiter note:
 * The FX feed uses '¬' (NOT SIGN) as a delimiter, not commas.
 * Use string-splitting (not char-based getline) because '¬' is often multi-byte in UTF-8.
 */
static std::vector<std::string> split_by_delim_str(const std::string& line, const std::string& delim) {
    std::vector<std::string> out;
    size_t start = 0;

    while (true) {
        size_t pos = line.find(delim, start);
        if (pos == std::string::npos) {
            std::string token = line.substr(start);
            trim_in_place(token);
            out.push_back(token);
            break;
        }

        std::string token = line.substr(start, pos - start);
        trim_in_place(token);
        out.push_back(token);
        start = pos + delim.size();
    }

    return out;
}

std::vector<ITrade*> FxTradeLoader::loadTrades() {
    // Reused pattern from BondTradeLoader: validate file path, open stream
    if (dataFile_.empty()) throw std::runtime_error("FX data file not set");

    std::ifstream stream(dataFile_);
    if (!stream.is_open()) throw std::runtime_error("Cannot open file: " + dataFile_);

    const std::string delim = u8"¬";

    std::vector<ITrade*> result;
    std::string line;

    /*
     * FxTrades.dat format (as you pasted):
     *
     * Line 1:  FxTrades¬YYYY-MM-DD              (file metadata)    -> skip
     * Line 2:  Type¬TradeDate¬Ccy1¬...¬TradeId  (header)           -> skip (we use fixed indices)
     * Lines :  FxSpot/FxFwd ...                 (data rows)        -> parse
     * Last :   END¬<count>                      (footer)           -> stop
     */

    // Skip metadata line
    if (!std::getline(stream, line)) return result;

    // Skip header line
    if (!std::getline(stream, line)) return result;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        auto items = split_by_delim_str(line, delim);
        if (items.empty()) continue;

        // Stop at footer: "END¬5"
        if (items[0] == "END") {
            break;
        }

        // Data lines must have 9 fields per header
        // Type, TradeDate, Ccy1, Ccy2, Amount, Rate, ValueDate, Counterparty, TradeId
        if (items.size() < 9) {
            throw std::runtime_error("Invalid FX trade line");
        }

        const std::string& type    = items[0];
        // FxTrades.dat has both TradeDate and ValueDate
		const std::string& dateStr = items[1];  // TradeDate
		const std::string& valueDateStr = items[6];  // ValueDate
        const std::string& ccy1    = items[2];
        const std::string& ccy2    = items[3];
        const std::string& amount  = items[4];
        const std::string& rate    = items[5];
        const std::string& cpty    = items[7];
        const std::string& tradeId = items[8];

        // Reused construction style from BondTradeLoader (tradeId + tradeType)
        FxTrade* trade = new FxTrade(tradeId, type);

        // Reused date parsing logic from BondTradeLoader
        std::tm tm = {};
        std::istringstream ds(dateStr);
        ds >> std::get_time(&tm, "%Y-%m-%d");
        trade->setTradeDate(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
        
        // Specific for FxTrade✅ Add ValueDate on FxTrade  
		std::tm tmVal = {};
		std::istringstream vds(valueDateStr);
		vds >> std::get_time(&tmVal, "%Y-%m-%d");
		trade->setValueDate(std::chrono::system_clock::from_time_t(std::mktime(&tmVal)));

        // Exercise requirement: Instrument = Ccy1 + Ccy2
        trade->setInstrument(ccy1 + ccy2);

        trade->setCounterparty(cpty);

        // Amount -> notional, Rate -> rate
        trade->setNotional(std::stod(amount));
        trade->setRate(std::stod(rate));

        result.push_back(trade);
    }

    return result;
}

// Reused simple getter/setter pattern from BondTradeLoader
std::string FxTradeLoader::getDataFile() const {
    return dataFile_;
}

void FxTradeLoader::setDataFile(const std::string& file) {
    dataFile_ = file;
}

