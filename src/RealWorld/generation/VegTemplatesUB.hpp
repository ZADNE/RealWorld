/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>
#include <span>
#include <string_view>

#include <RealWorld/constants/generation.hpp>

namespace rw {

struct String {
    glm::uint begin;
    glm::uint count;
};

struct ProbabilisticRewriteRuleBodies {
    glm::uint firstIndex;
    glm::uint bodyCount;
};

constexpr char k_firstRewriteableSymbol = 'B';
constexpr char k_lastRewriteableSymbol  = 'B';
constexpr int k_rewriteableSymbolCount  = k_lastRewriteableSymbol -
                                         k_firstRewriteableSymbol + 1;
static_assert(k_rewriteableSymbolCount == 1, "Fix GLSL");

struct VegTemplate {
    String axiom;
    std::array<ProbabilisticRewriteRuleBodies, k_rewriteableSymbolCount> rules;
    float alpha;
    glm::uint iterCount;
};

constexpr int k_totalRewriteRuleBodyCount = 3;

struct VegTemplatesUB {
    constexpr VegTemplatesUB(
        std::string_view s, std::span<const float> probs,
        std::span<const String> bodies, std::span<const VegTemplate> ts
    ) {
        assert(s.size() <= symbols.size());
        assert(probs.size() == ruleProbs.size());
        assert(bodies.size() == ruleBodies.size());
        assert(ts.size() == tmplts.size());
        std::copy(s.begin(), s.end(), symbols.data());
        std::copy(probs.begin(), probs.end(), ruleProbs.data());
        std::copy(bodies.begin(), bodies.end(), ruleBodies.data());
        std::copy(ts.begin(), ts.end(), tmplts.data());
        std::for_each(symbols.begin(), symbols.end(), [](char& c) {
            c -= k_firstRewriteableSymbol;
        });
    }

    std::array<char, k_vegTemplatesSymbolCount> symbols{};
    std::array<float, k_totalRewriteRuleBodyCount> ruleProbs{};
    std::array<String, k_totalRewriteRuleBodyCount> ruleBodies{};
    std::array<VegTemplate, k_vegTemplateCount> tmplts{};
};

} // namespace rw
