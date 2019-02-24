
#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(const Mesh& m, const Material& mat) {
    componentType = "meshRenderer";

    mesh = m;
    material = mat;
}

MeshRenderer* MeshRenderer::clone() const {
    return new MeshRenderer(*this);
}