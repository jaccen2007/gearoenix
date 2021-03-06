#ifndef GEAROENIX_MATH_NUMERIC_HPP
#define GEAROENIX_MATH_NUMERIC_HPP

#include "../core/cr-types.hpp"
#include <cmath>

// Maybe in future needed
#define GX_SIGNEDNESS_COMPATIBILITY_CHECK(ReceivedType, FinalType)                                            \
    static_assert(!std::numeric_limits<ReceivedType>::is_signed || std::numeric_limits<FinalType>::is_signed, \
        "This functionality only works when the signedness compatibility is available.");

#define GX_SIGNEDNESS_CHECK(Type)                       \
    static_assert(std::numeric_limits<Type>::is_signed, \
        "This functionality only works when the signedness compatibility is available.");

namespace gearoenix::math {
class Numeric {
public:
    // It will raise given number to the nearest bigger or equal value that is in power of 2
    template <typename T>
    [[nodiscard]] constexpr static T raise_p2(T v) noexcept;

    template <typename T>
    [[nodiscard]] constexpr static T raise_p2(T v, T maximum, T minimum) noexcept;

    template <typename T>
    [[nodiscard]] constexpr static T maximum(T a, T b) noexcept;

    template <typename T>
    [[nodiscard]] constexpr static T minimum(T a, T b) noexcept;

    /// On failure it returns static_cast<T>(-1).
    template <typename T>
    [[nodiscard]] constexpr static T floor_log2(T a) noexcept;

    [[nodiscard]] static constexpr float radical_inverse_vdc(std::uint32_t bits) noexcept;

    [[nodiscard]] static constexpr float geometry_schlick_ggx(float n_dot_v, float roughness) noexcept;
};
}

template <typename T>
constexpr T gearoenix::math::Numeric::raise_p2(const T v) noexcept
{
    unsigned long long i = 1;
    while (v > static_cast<T>(i)) {
        i <<= 1;
    }
    return static_cast<T>(i);
}

template <typename T>
constexpr T gearoenix::math::Numeric::raise_p2(const T v, const T maximum, const T minimum) noexcept
{
    if (v > maximum)
        return maximum;
    if (v < minimum)
        return minimum;
    return raise_p2(v);
}

template <typename T>
constexpr T gearoenix::math::Numeric::maximum(const T a, const T b) noexcept
{

    if (std::is_floating_point<T>::value) {
        if (std::isnan(a))
            return b;
        if (std::isnan(b))
            return a;
    }
    return GX_MAX(a, b);
}

template <typename T>
constexpr T gearoenix::math::Numeric::minimum(const T a, const T b) noexcept
{

    if (std::is_floating_point<T>::value) {
        if (std::isnan(a))
            return b;
        if (std::isnan(b))
            return a;
    }
    return GX_MIN(a, b);
}

template <typename T>
constexpr T gearoenix::math::Numeric::floor_log2(const T a) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "Only integer number can be used by this function.");
    for (T i = 0; i < static_cast<T>(sizeof(T) * 8); ++i) {
        if (((a >> i) & 1) == 1) {
            return i;
        }
    }
    if (a == 0)
        return 0;
    return static_cast<T>(-1);
}

constexpr float gearoenix::math::Numeric::radical_inverse_vdc(std::uint32_t bits) noexcept
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10f; // / 0x100000000 // TODO
}

constexpr float gearoenix::math::Numeric::geometry_schlick_ggx(const float n_dot_v, const float roughness) noexcept
{
    float a = roughness;
    float k = (a * a) / 2.0f;
    float nom = n_dot_v;
    float denom = n_dot_v * (1.0f - k) + k;
    return nom / denom;
}

#endif