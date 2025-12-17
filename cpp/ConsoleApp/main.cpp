#include "../RiskSystem/SerialTradeLoader.h"
#include "../RiskSystem/StreamingTradeLoader.h"
#include "../Models/ScalarResults.h"
#include "../RiskSystem/SerialPricer.h"
#include "../RiskSystem/ParallelPricer.h"
#include "../RiskSystem/ScreenResultPrinter.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int _getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

int main(int argc, char* argv[]) {
    /*SerialTradeLoader tradeLoader;
    
    auto allTrades = tradeLoader.loadTrades();
    
    ScalarResults results;
    
    //SerialPricer pricer;
    ParallelPricer pricer;
    
    pricer.price(allTrades, &results);*/
    
    ScalarResults results;

	// Use streaming loader: trades are loaded and priced immediately
	StreamingTradeLoader streamingLoader;
	streamingLoader.loadAndPrice(&results);
		
    ScreenResultPrinter screenPrinter;
    screenPrinter.printResults(results);
    
    std::cout << "Press any key to exit.." << std::endl;
    _getch();
    
    return 0;
}
