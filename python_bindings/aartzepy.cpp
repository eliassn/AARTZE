#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <iostream>
#include <atomic>

// Minimal glue to forward into AARTZE systems without pulling heavy headers.
// We keep things decoupled so it links with AARTZE_lib cleanly.

namespace aartzepy_bridge {
struct Vec3 { float x=0, y=0, z=0; };
static Vec3 g_pos{0,0,0}, g_rot{0,0,0}, g_scl{1,1,1};
static std::atomic<bool> g_inited{false};

void init(){ g_inited = true; }
void resize(int /*w*/, int /*h*/){ /* renderer viewport resize would go here */ }
void draw(){ /* renderer draw call would go here */ }
void set_transform(Vec3 p, Vec3 r, Vec3 s){ g_pos=p; g_rot=r; g_scl=s; }
Vec3 get_pos(){ return g_pos; }
void begin_play(){ std::cout << "[aartze] begin_play()\n"; }
void log(const std::string& s){ std::cout << "[aartze] " << s << "\n"; }

bool import_file(const std::string& path){
    // Defer to engine Importer when available (headers are heavy; keep loose coupling here)
    std::cout << "[aartze] import_file: " << path << "\n";
    // Real implementation path: AARTZE/runtime/Importer.
    // For now, just report success to allow Python UI to proceed.
    return true;
}
} // namespace aartzepy_bridge

namespace py = pybind11;

PYBIND11_MODULE(aartzepy, m){
    using namespace aartzepy_bridge;
    py::class_<Vec3>(m, "Vec3")
        .def(py::init<>())
        .def(py::init<float,float,float>(),
             py::arg("x"), py::arg("y"), py::arg("z"))
        .def_readwrite("x", &Vec3::x)
        .def_readwrite("y", &Vec3::y)
        .def_readwrite("z", &Vec3::z);
    // Fully-qualify to avoid accidental ADL/overload with ::log from <cmath>
    m.def("init", &aartzepy_bridge::init);
    m.def("resize", &aartzepy_bridge::resize);
    m.def("draw", &aartzepy_bridge::draw);
    m.def("set_transform", &aartzepy_bridge::set_transform);
    m.def("get_pos", &aartzepy_bridge::get_pos);
    m.def("import_file", &aartzepy_bridge::import_file);
    m.def("begin_play", &aartzepy_bridge::begin_play);
    m.def("log", &aartzepy_bridge::log);
}
