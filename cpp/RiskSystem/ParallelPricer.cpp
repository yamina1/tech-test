/*
#include "ParallelPricer.h"
#include <stdexcept>

ParallelPricer::~ParallelPricer() {

}

void ParallelPricer::loadPricers() {
    PricingConfigLoader pricingConfigLoader;
    pricingConfigLoader.setConfigFile("./PricingConfig/PricingEngines.xml");
    PricingEngineConfig pricerConfig = pricingConfigLoader.loadConfig();
    
    for (const auto& configItem : pricerConfig) {
        throw std::runtime_error("Not implemented");
    }
}

void ParallelPricer::price(const std::vector<std::vector<ITrade*>>& tradeContainers, 
                           IScalarResultReceiver* resultReceiver) {
    throw std::runtime_error("Not implemented");
}
*/

#include "ParallelPricer.h"
#include <stdexcept>
#include <future>

// Reuse:
// Concrete pricing engines already implemented in the system.
// The same pricing engines are reused here to ensure consistent pricing logic
// between serial and parallel execution.
#include "../Pricers/GovBondPricingEngine.h"
#include "../Pricers/CorpBondPricingEngine.h"
#include "../Pricers/FxPricingEngine.h"

ParallelPricer::~ParallelPricer() {
    // Cleanup:
    // Pricing engines are owned by this class and must be released.
    for (auto& kv : pricers_) {
        delete kv.second;
    }
    pricers_.clear();
}

void ParallelPricer::loadPricers() {
    // Reuse:
    // Pricing engine configuration is loaded using the same mechanism
    // as the serial pricer to ensure identical configuration behaviour.
    pricers_.clear();

    PricingConfigLoader pricingConfigLoader;
    pricingConfigLoader.setConfigFile("./PricingConfig/PricingEngines.xml");
    PricingEngineConfig pricerConfig = pricingConfigLoader.loadConfig();

    for (const auto& configItem : pricerConfig) {
        const std::string& tradeType = configItem.getTradeType();
        std::string typeName = configItem.getTypeName();

        // Compatibility:
        // Configuration values may include fully-qualified names.
        // Only the concrete class name is required for the C++ implementation.
        auto dotPos = typeName.find_last_of('.');
        if (dotPos != std::string::npos) {
            typeName = typeName.substr(dotPos + 1);
        }

        IPricingEngine* engine = nullptr;

        // Factory mapping:
        // Map configured engine type to the corresponding C++ implementation.
        if (typeName == "GovBondPricingEngine") {
            engine = new GovBondPricingEngine();
        } else if (typeName == "CorpBondPricingEngine") {
            engine = new CorpBondPricingEngine();
        } else if (typeName == "FxPricingEngine") {
            engine = new FxPricingEngine();
        } else {
            throw std::runtime_error(
                "Unknown pricing engine type: " + configItem.getTypeName());
        }

        // Store pricing engine by trade type for fast lookup during pricing.
        pricers_[tradeType] = engine;
    }
}

void ParallelPricer::price(
    const std::vector<std::vector<ITrade*>>& tradeContainers,
    IScalarResultReceiver* resultReceiver) {

    if (resultReceiver == nullptr) {
        throw std::invalid_argument("resultReceiver cannot be null");
    }

    // Load pricing engines once before starting parallel execution.
    loadPricers();

    // Thread-safe adapter:
    // Result insertion is protected by a mutex to avoid data races,
    // while pricing calculations themselves run without locking.
    class LockedReceiver : public IScalarResultReceiver {
    public:
        LockedReceiver(IScalarResultReceiver* inner, std::mutex& m)
            : inner_(inner), m_(m) {}

        void addResult(const std::string& tradeId, double result) override {
            std::lock_guard<std::mutex> lock(m_);
            inner_->addResult(tradeId, result);
        }

        void addError(const std::string& tradeId, const std::string& error) override {
            std::lock_guard<std::mutex> lock(m_);
            inner_->addError(tradeId, error);
        }

    private:
        IScalarResultReceiver* inner_;
        std::mutex& m_;
    };

    LockedReceiver lockedReceiver(resultReceiver, resultMutex_);

    // Parallel execution strategy:
    // Each trade pricing calculation is launched as an asynchronous task.
    // Tasks execute in parallel and results are collected once all tasks finish.
    std::vector<std::future<void>> tasks;

    for (const auto& tradeContainer : tradeContainers) {
        for (ITrade* trade : tradeContainer) {
            tasks.push_back(std::async(std::launch::async,
                [this, trade, &lockedReceiver]() {

                    //const std::string tradeType = trade->getInstrument();
                    const std::string tradeType = trade->getTradeType();

                    auto it = pricers_.find(tradeType);
                    if (it == pricers_.end()) {
                        // Error handling:
                        // If no pricing engine exists for the trade type,
                        // record an error instead of pricing.
                        lockedReceiver.addError(
                            trade->getTradeId(),
                            "No Pricing Engines available for this trade type");
                        return;
                    }

                    // Pricing:
                    // Pricing engine execution occurs without locking
                    // to allow maximum parallelism.
                    it->second->price(trade, &lockedReceiver);
                }));
        }
    }

    // Synchronisation point:
    // The function blocks until all pricing tasks have completed,
    // ensuring all results are available before returning.
    for (auto& f : tasks) {
        f.get();
    }
}
