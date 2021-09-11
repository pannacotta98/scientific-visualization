#pragma once

#include <modules/tnm067lab1/tnm067lab1moduledefine.h>
#include <inviwo/core/util/glm.h>


namespace inviwo {

template <typename T>
struct float_type {
    using type = double;
};

template <>
struct float_type<float> {
    using type = float;
};
template <>
struct float_type<vec3> {
    using type = float;
};
template <>
struct float_type<vec2> {
    using type = float;
};
template <>
struct float_type<vec4> {
    using type = float;
};

namespace TNM067 {
namespace Interpolation {

#define ENABLE_LINEAR_UNITTEST 1
template <typename T, typename F = double>
T linear(const T& a, const T& b, F x) {
    if (x <= 0) return a;
    if (x >= 1) return b;

    return (1.0 - x)*a + x*b;
}

// clang-format off
    /*
     2------3
     |      |
    y|  •   |
     |      |
     0------1
        x
    */
    // clang format on
#define ENABLE_BILINEAR_UNITTEST 1
template<typename T, typename F = double> 
T bilinear(const std::array<T, 4> &v, F x, F y) {
    x = glm::clamp(x, decltype(x)(0), decltype(x)(1));
    y = glm::clamp(y, decltype(y)(0), decltype(y)(1));
    return linear(linear(v[0], v[1], x), linear(v[2], v[3], x), y);
}


    // clang-format off
    /* 
    a--•----b------c
    0  x    1      2
    */
// clang-format on
#define ENABLE_QUADRATIC_UNITTEST 1
template <typename T, typename F = double>
T quadratic(const T& a, const T& b, const T& c, F x) {
    x = glm::clamp(x, decltype(x)(0), decltype(x)(1));
    return (1.0-x)*(1-2.0*x)*a + 4.0*x*(1.0-x)*b + x*(2.0*x-1.0)*c;
}

// clang-format off
    /* 
    6-------7-------8
    |       |       |
    |       |       |
    |       |       |
    3-------4-------5
    |       |       |
   y|  •    |       |
    |       |       |
    0-------1-------2
    0  x    1       2
    */
// clang-format on
#define ENABLE_BIQUADRATIC_UNITTEST 1
template <typename T, typename F = double>
T biQuadratic(const std::array<T, 9>& v, F x, F y) {
    x = glm::clamp(x, decltype(x)(0), decltype(x)(1));
    y = glm::clamp(y, decltype(y)(0), decltype(y)(1));
    return quadratic(quadratic(v[0], v[1], v[2], x), quadratic(v[3], v[4], v[5], x), quadratic(v[6], v[7], v[8], x), y);
}

// clang-format off
    /*
     2---------3
     |'-.      |
     |   -,    |
   y |  •  -,  |
     |       -,|
     0---------1
        x
    */
// clang-format on
#define ENABLE_BARYCENTRIC_UNITTEST 1
template <typename T, typename F = double>
T barycentric(const std::array<T, 4>& v, F x, F y) {
    F alpha = 1.0 - x - y;

    return alpha * v[0] + y * v[1] + x * v[2];
}

}  // namespace Interpolation
}  // namespace TNM067
}  // namespace inviwo
