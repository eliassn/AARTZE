#pragma once
#include <memory>
#include "Dock.hpp"
#include "RendererGL.hpp"

struct GLFWwindow;
class OutlinerPanel; class DetailsPanel; class ContentPanel; class SlotPreview;

namespace ui2 {

class EditorShell {
public:
    void Initialize(GLFWwindow* window);
    void Resize(int w,int h);
    void Tick(float dt);
    void Render();
    void Shutdown();
private:
    GLFWwindow* m_window{nullptr};
    int m_w{1280}, m_h{720};
    RendererGL m_renderer;
    DrawList m_draw;
    std::shared_ptr<DockSpace>    m_dock;
    std::shared_ptr<OutlinerPanel> m_left;
    std::shared_ptr<DetailsPanel>  m_right;
    std::shared_ptr<ContentPanel>  m_bottom;
    std::shared_ptr<SlotPreview>   m_preview;
    // Simple tabs (stubs)
    std::shared_ptr<TabBar> m_tbTop;
};

}
