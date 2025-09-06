#include "InputSystem.h"
#include <algorithm>

namespace aartze::input {

static InputSystem g_input_instance;

InputSystem& Get() { return g_input_instance; }

Device DeviceFromString(const std::string& s){
    if (s == "mouse") return Device::Mouse;
    if (s == "pad" || s == "gamepad") return Device::Pad;
    return Device::Keyboard;
}

void InputSystem::ensureMap(const std::string& name){
    if (maps_.find(name) == maps_.end()) maps_.emplace(name, Map{});
    if (active_map_.empty()) active_map_ = name;
}

void InputSystem::setActiveMap(const std::string& name){ ensureMap(name); active_map_ = name; }

bool InputSystem::bindAction(const std::string& map, const std::string& action, Device dev, int code){
    ensureMap(map);
    maps_[map].actions[action].push_back(ActionBinding{dev, code});
    return true;
}

bool InputSystem::bindAxis(const std::string& map, const std::string& axis, Device dev, int code, float scale){
    ensureMap(map);
    maps_[map].axes[axis].push_back(AxisBinding{dev, code, scale});
    return true;
}

void InputSystem::feedKey(int key, bool down){ key_state_[key] = down; }
void InputSystem::feedMouseButton(int btn, bool down){ mouse_state_[btn] = down; }
void InputSystem::feedMouseMove(float dx, float dy){ mouse_dx_ = dx; mouse_dy_ = dy; }

bool InputSystem::action(const std::string& actionName) const{
    auto itM = maps_.find(active_map_);
    if (itM == maps_.end()) return false;
    auto itA = itM->second.actions.find(actionName);
    if (itA == itM->second.actions.end()) return false;
    for (const auto& b : itA->second){
        if (b.device == Device::Keyboard){
            auto it = key_state_.find(b.code); if (it!=key_state_.end() && it->second) return true;
        } else if (b.device == Device::Mouse){
            auto it = mouse_state_.find(b.code); if (it!=mouse_state_.end() && it->second) return true;
        } else {
            // Pad not wired yet in stub
        }
    }
    return false;
}

float InputSystem::axis(const std::string& axisName) const{
    float v = 0.f;
    auto itM = maps_.find(active_map_);
    if (itM == maps_.end()) return 0.f;
    auto itX = itM->second.axes.find(axisName);
    if (itX == itM->second.axes.end()){
        // expose raw mouse axes as built-ins
        if (axisName == "mouse_x") return mouse_dx_;
        if (axisName == "mouse_y") return mouse_dy_;
        return 0.f;
    }
    for (const auto& b : itX->second){
        if (b.device == Device::Keyboard){
            auto it = key_state_.find(b.code); if (it!=key_state_.end() && it->second) v += b.scale;
        } else if (b.device == Device::Mouse){
            // For mouse axes, code 0: x, 1: y
            if (b.code == 0) v += mouse_dx_ * b.scale; else if (b.code == 1) v += mouse_dy_ * b.scale;
        } else {
            // Pad not wired in this stub
        }
    }
    return v;
}

void InputSystem::clear(){ key_state_.clear(); mouse_state_.clear(); mouse_dx_ = mouse_dy_ = 0.f; }
void InputSystem::update(){ /* reserved for gainput-driven manager */ }

} // namespace aartze::input

