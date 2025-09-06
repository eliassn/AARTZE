#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace aartze::input {

enum class Device { Keyboard, Mouse, Pad };

struct ActionBinding {
    Device device{Device::Keyboard};
    int code{0};
};

struct AxisBinding {
    Device device{Device::Keyboard};
    int code{0};
    float scale{1.0f};
};

struct Map {
    std::unordered_map<std::string, std::vector<ActionBinding>> actions;  // name -> bindings
    std::unordered_map<std::string, std::vector<AxisBinding>> axes;       // name -> bindings
};

class InputSystem {
public:
    void ensureMap(const std::string& name);
    void setActiveMap(const std::string& name);
    const std::string& activeMap() const { return active_map_; }

    // Bindings
    bool bindAction(const std::string& map, const std::string& action,
                    Device dev, int code);
    bool bindAxis(const std::string& map, const std::string& axis,
                  Device dev, int code, float scale);

    // Feeds (from UI)
    void feedKey(int key, bool down);
    void feedMouseButton(int btn, bool down);
    void feedMouseMove(float dx, float dy);

    // Queries (aggregated on the active map)
    bool  action(const std::string& actionName) const;
    float axis(const std::string& axisName) const;

    void clear();
    void update(); // no-op for now; reserved for gainput integration

private:
    std::unordered_map<std::string, Map> maps_;
    std::string active_map_;

    // Simple state for manual feed
    std::unordered_map<int, bool> key_state_;
    std::unordered_map<int, bool> mouse_state_;
    float mouse_dx_{0.f}, mouse_dy_{0.f};
};

// Global accessor used by pybind bridge and engine
InputSystem& Get();

// Helpers
Device DeviceFromString(const std::string& s);

} // namespace aartze::input

