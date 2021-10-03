#include <modules/tnm067lab4/jacobian.h>

namespace inviwo {

namespace util {

mat2 jacobian(const ImageSampler& sampler, vec2 position, vec2 offset) {
    vec2 ox(offset.x, 0);
    vec2 oy(0, offset.y);
    mat2 J;

    dvec2 dVdx = (sampler.sample(position + ox) - sampler.sample(position - ox)) / (2.0 * offset.x);
    dvec2 dVdy = (sampler.sample(position + oy) - sampler.sample(position - oy)) / (2.0 * offset.y);

    J[0] = dVdx;
    J[1] = dVdy;

    return J;
}

}  // namespace util

}  // namespace inviwo
