//
// Created by danie on 10/31/2022.
//

#ifndef TRADINGENGINE_VECTORRETURNABLE_H
#define TRADINGENGINE_VECTORRETURNABLE_H

#include <crow.h>
#include <nlohmann/json.hpp>
#include <utility>

template<class T, class = std::enable_if_t<std::is_base_of_v<crow::returnable, T>>>
class VectorReturnable : public crow::returnable {
public:
    explicit VectorReturnable(std::vector<T> data, std::string name) :
            crow::returnable("application/json"), m_data{std::move(data)}, m_name{std::move(name)} {};

    [[nodiscard]]
    std::string dump() const override {
        auto j = toJson();
        return j.dump();
    }

    [[nodiscard]]
    nlohmann::json toJson() const {
        std::vector<nlohmann::json> jsonData{};
        jsonData.reserve(m_data.size());

        for (auto& d: m_data) {
            jsonData.push_back(d.toJson());
        }

        nlohmann::json j = {
                {m_name, jsonData}
        };

        return j;
    }

    [[nodiscard]]
    nlohmann::json toFlattenJson() const {
        std::vector<nlohmann::json> jsonData{};
        jsonData.reserve(m_data.size());

        for (auto& d: m_data) {
            jsonData.push_back(d.toJson());
        }

        return {m_name, jsonData};
    }

private:
    std::vector<T> m_data;
    std::string m_name;
};

#endif //TRADINGENGINE_VECTORRETURNABLE_H
