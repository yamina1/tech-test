#include "SerialPricer.h"
#include <stdexcept>

// Concrete pricers live in ../pricers
#include "../Pricers/GovBondPricingEngine.h"
#include "../Pricers/CorpBondPricingEngine.h"
#include "../Pricers/FxPricingEngine.h"

SerialPricer::~SerialPricer() {

}

/*void SerialPricer::loadPricers() {
    PricingConfigLoader pricingConfigLoader;
    pricingConfigLoader.setConfigFile("./PricingConfig/PricingEngines.xml");
    PricingEngineConfig pricerConfig = pricingConfigLoader.loadConfig();
    
    for (const auto& configItem : pricerConfig) {
        throw std::runtime_error("Not implemented");
    }
}*/

void SerialPricer::loadPricers() {
    /*
     * Reuse:
     * SerialPricer may be used multiple times, so we defensively clear
     * any previously created pricing engines before reloading.
     */
    pricers_.clear();

    /*
     * Reuse from Exercise 3:
     * PricingConfigLoader already knows how to read PricingEngines.xml
     * and return a list of PricingEngineConfigItem objects.
     */
    PricingConfigLoader pricingConfigLoader;
    pricingConfigLoader.setConfigFile("./PricingConfig/PricingEngines.xml");
    PricingEngineConfig pricerConfig = pricingConfigLoader.loadConfig();

    /*
     * For each configuration item:
     *  - create exactly one pricing engine
     *  - register it against the trade type
     *
     * This ensures pricing engines are reused across trades of the same type.
     */
    for (const auto& configItem : pricerConfig) {
        const std::string& tradeType = configItem.getTradeType();
        const std::string& typeName  = configItem.getTypeName();

        IPricingEngine* engine = nullptr;


         // Reuse / compatibility:
		// Config can contain fully-qualified names (e.g. "HmxLabs.TechTest.Pricers.GovBondPricingEngine").
		// In C++ we only care about the class name, so take the last token after '.' (and '::' just in case).
		auto shortType = typeName;
		auto dotPos = shortType.find_last_of('.');
		if (dotPos != std::string::npos) shortType = shortType.substr(dotPos + 1);
		auto colPos = shortType.find_last_of(':'); // handles "Namespace::Class"
		if (colPos != std::string::npos) shortType = shortType.substr(colPos + 1);

        /*
         * Simple factory pattern:
         * C++ has no runtime reflection here, so we explicitly map
         * configured type names to concrete pricing engine classes.
         *
         * The "assembly" field from the config is ignored in C++
         * (it is relevant only in managed languages).
         */
        

		/*if (typeName == "GovBondPricingEngine") {
			engine = new GovBondPricingEngine();
		}
		else if (typeName == "CorpBondPricingEngine") {
			engine = new CorpBondPricingEngine();
		}
		else if (typeName == "FxPricingEngine") {
			engine = new FxPricingEngine();
		}
		else {
			throw std::runtime_error("Unknown pricing engine type: " + typeName);
		}*/

		if (shortType == "GovBondPricingEngine") {
			engine = new GovBondPricingEngine();
		}
		else if (shortType == "CorpBondPricingEngine") {
			engine = new CorpBondPricingEngine();
		}
		else if (shortType == "FxPricingEngine") {
			engine = new FxPricingEngine();
		}
		else {
			throw std::runtime_error("Unknown pricing engine type: " + typeName);
		}

        /*
         * Register the pricing engine for this trade type.
         * The price() function later retrieves the engine using
         * trade->getTradeType().
         */
        pricers_[tradeType] = engine;
    }
}






void SerialPricer::price(const std::vector<std::vector<ITrade*>>& tradeContainers, 
                         IScalarResultReceiver* resultReceiver) {
    loadPricers();
    
    for (const auto& tradeContainer : tradeContainers) {
        for (ITrade* trade : tradeContainer) {
            std::string tradeType = trade->getTradeType();
            if (pricers_.find(tradeType) == pricers_.end()) {
                resultReceiver->addError(trade->getTradeId(), "No Pricing Engines available for this trade type");
                continue;
            }
            
            IPricingEngine* pricer = pricers_[tradeType];
            pricer->price(trade, resultReceiver);
        }
    }
}
