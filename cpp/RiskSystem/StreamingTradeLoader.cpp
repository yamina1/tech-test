#include "StreamingTradeLoader.h"
#include "../Loaders/BondTradeLoader.h"
#include "../Loaders/FxTradeLoader.h"
#include "PricingConfigLoader.h"
#include <stdexcept>

#include "../Pricers/GovBondPricingEngine.h"
#include "../Pricers/CorpBondPricingEngine.h"
#include "../Pricers/FxPricingEngine.h"

#include <iostream>   // ✅ REQUIRED for std::cout / std::endl


/*std::vector<ITradeLoader*> StreamingTradeLoader::getTradeLoaders() {
    std::vector<ITradeLoader*> loaders;
    
    BondTradeLoader* bondLoader = new BondTradeLoader();
    bondLoader->setDataFile("TradeData/BondTrades.dat");
    loaders.push_back(bondLoader);
    
    FxTradeLoader* fxLoader = new FxTradeLoader();
    fxLoader->setDataFile("TradeData/FxTrades.dat");
    loaders.push_back(fxLoader);
    
    return loaders;
}

void StreamingTradeLoader::loadPricers() {
    throw std::runtime_error("Not implemented");
}

StreamingTradeLoader::~StreamingTradeLoader() {
    
}

void StreamingTradeLoader::loadAndPrice(IScalarResultReceiver* resultReceiver) {
    throw std::runtime_error("Not implemented");
}
*/


/*
 * Reuse:
 * We reuse the existing trade loaders exactly as they are.
 * No parsing logic is duplicated.
 */
std::vector<ITradeLoader*> StreamingTradeLoader::getTradeLoaders() {
    std::vector<ITradeLoader*> loaders;

    BondTradeLoader* bondLoader = new BondTradeLoader();
    bondLoader->setDataFile("TradeData/BondTrades.dat");
    loaders.push_back(bondLoader);

    FxTradeLoader* fxLoader = new FxTradeLoader();
    fxLoader->setDataFile("TradeData/FxTrades.dat");
    loaders.push_back(fxLoader);

    return loaders;
}

/*
 * Reuse:
 * This is the same pricing-engine setup logic as SerialPricer.
 * We reuse the config file and engine mapping.
 */
void StreamingTradeLoader::loadPricers() {
    PricingConfigLoader loader;
    loader.setConfigFile("./PricingConfig/PricingEngines.xml");
    PricingEngineConfig config = loader.loadConfig();
    

    for (const auto& item : config) {
        IPricingEngine* engine = nullptr;

        std::string typeName = item.getTypeName();
        auto dotPos = typeName.find_last_of('.');
        if (dotPos != std::string::npos) {
            typeName = typeName.substr(dotPos + 1);
        }

        if (typeName == "GovBondPricingEngine") {
            engine = new GovBondPricingEngine();
        } else if (typeName == "CorpBondPricingEngine") {
            engine = new CorpBondPricingEngine();
        } else if (typeName == "FxPricingEngine") {
            engine = new FxPricingEngine();
        } else {
            throw std::runtime_error("Unknown pricing engine type: " + typeName);
        }

        pricers_[item.getTradeType()] = engine;
    }
}

StreamingTradeLoader::~StreamingTradeLoader() {
    // Clean up pricing engines
    for (auto& p : pricers_) {
        delete p.second;
    }
}

/*
 * Core of Exercise 7
 *
 * Reuse strategy:
 * - Reuse existing loaders to read trades
 * - Reuse pricing engines to price trades
 * - Price each trade immediately
 * - Delete trades immediately to free memory
 */
void StreamingTradeLoader::loadAndPrice(IScalarResultReceiver* resultReceiver) {
    loadPricers();

    auto loaders = getTradeLoaders();

    for (ITradeLoader* loader : loaders) {
		

        // Reuse existing batch interface,
        // but process trades immediately instead of storing them.
        auto trades = loader->loadTrades();

        for (ITrade* trade : trades) {
			
			// To be removed: Debug / proof of streaming: trade is handled immediately
			//std::cout << "Streaming trade loaded: " << trade->getTradeId() << std::endl;
			
            //auto it = pricers_.find(trade->getType());
            //auto it = pricers_.find(trade->getInstrument());
            auto it = pricers_.find(trade->getTradeType());

            if (it != pricers_.end()) {
                it->second->price(trade, resultReceiver);
            } else {
                resultReceiver->addError(
                    trade->getTradeId(),
                    "No Pricing Engines available for this trade type"
                );
            }

            // Key streaming step:
            // trade is discarded immediately after pricing
            
            //To be removed: Debug / proof of streaming: trade is released immediately
			//std::cout << "Streaming trade released: " << trade->getTradeId() << std::endl;

            delete trade;
        }

        delete loader;
    }
}
