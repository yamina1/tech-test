#include "ScreenResultPrinter.h"
#include <iostream>

/*void ScreenResultPrinter::printResults(ScalarResults& results) {
    for (const auto& result : results) {
        // Write code here to print out the results such that we have:
        // TradeID : Result : Error
        // If there is no result then the output should be:
        // TradeID : Error
        // If there is no error the output should be:
        // TradeID : Result
    }
}
*/

void ScreenResultPrinter::printResults(ScalarResults& results) {

    // Reuse:
    // ScalarResults already implements iteration (Exercise 5),
    // so we can directly use a range-for loop.
    for (const auto& result : results) {

        // Reuse:
        // ScalarResult already exposes the trade identifier.
        // Always print the TradeId first.
        std::cout << result.getTradeId();

        // Reuse:
        // ScalarResult stores the pricing result as std::optional<double>.
        // Only print it if a result exists.
        if (result.getResult().has_value()) {
            std::cout << " : " << result.getResult().value();
        }

        // Reuse:
        // ScalarResult stores errors as std::optional<std::string>.
        // Only print it if an error exists.
        if (result.getError().has_value()) {
            std::cout << " : " << result.getError().value();
        }

        // One output line per trade, as required by the exercise.
        std::cout << std::endl;
    }
}
