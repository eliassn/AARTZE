#include "StyleSet.hpp"

namespace ui2 {

static StyleSet g_style;

const StyleSet& GetStyle() { return g_style; }
void SetStyle(const StyleSet& s) { g_style = s; }

}

