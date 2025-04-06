/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <format>
#include <string>

namespace rw {

constexpr int k_versionMajor = 0;
constexpr int k_versionMinor = 2;
constexpr int k_versionPatch = 0;

/**
 * @brief Is integer representation of RealWorld's version
 */
constexpr int k_version = k_versionMajor * 1'000'000 + k_versionMinor * 1'000 +
                          k_versionPatch;

inline std::string versionString() {
    return std::format("v{}.{}.{}", k_versionMajor, k_versionMinor, k_versionPatch);
}

} // namespace rw
