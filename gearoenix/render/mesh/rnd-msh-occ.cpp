#include "rnd-msh-occ.hpp"
#include "../../system/file/sys-fl-file.hpp"
#include "../buffer/rnd-buf-mesh.hpp"
#include "../rnd-engine.hpp"
#include "../shader/rnd-shd-shader.hpp"

gearoenix::render::mesh::Occ::Occ(system::file::File* f, Engine* e, core::EndCaller<core::EndCallerIgnore> c)
{
    shader::Id sid;
    f->read(sid);
#ifdef DEBUG_MODE
    if (sid != shader::DEPTH_POS) {
        GXLOGF("Unexpected");
    }
#endif
    unsigned int vec = shader::Shader::get_vertex_real_count(sid);
    buf = e->create_mesh(vec, f, c);
}

gearoenix::render::mesh::Occ::~Occ()
{
    delete buf;
}
