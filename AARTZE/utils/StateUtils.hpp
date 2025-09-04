

// File: utils/StateUtils.hpp
#pragma once

#include <string>
#include <unordered_map>

#include "../components/StateComponent.hpp"

inline StateComponent::State ParseStateFromString(const std::string& str)
{
    static const std::unordered_map<std::string, StateComponent::State> stateMap = {
        {"Calm", StateComponent::State::Calm},
        {"Suspicious", StateComponent::State::Suspicious},
        {"Alerted", StateComponent::State::Alerted},
        {"Panicked", StateComponent::State::Panicked},
        {"Injured", StateComponent::State::Injured},
        {"Sleeping", StateComponent::State::Sleeping},
        {"Working", StateComponent::State::Working},
        {"Loitering", StateComponent::State::Loitering},
    };

    auto it = stateMap.find(str);
    if (it != stateMap.end())
    {
        return it->second;
    }

    // Default fallback if invalid string
    return StateComponent::State::Calm;
}
