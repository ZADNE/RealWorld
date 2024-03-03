/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>
#include <span>
#include <string_view>

#include <RealWorld/constants/generation.hpp>
#include <RealWorld/constants/tile.hpp>

namespace rw {

struct String {
    glm::uint begin;
    glm::uint count;
};

struct ProbabilisticRewriteRuleBodies {
    glm::uint firstIndex;
    glm::uint bodyCount;
};

constexpr int k_rewriteableSymbolCount = 1;

struct alignas(sizeof(glm::vec2)) VegTemplate {
    String axiom;
    glm::vec2 initSizeTi;
    glm::vec2 sizeChange;
    glm::vec2 densityStiffness;
    Wall wallType;
    glm::uint iterCount;
    std::array<ProbabilisticRewriteRuleBodies, k_rewriteableSymbolCount> rules;
};

constexpr int k_totalRewriteRuleBodyCount = 3;

enum class Symbol : uint8_t {
    Branch,
    Stem,
    LeftTurn,
    RightTurn,
    Push,
    Pop
};

constexpr Symbol toSymbol(char c) {
    using enum Symbol;
    switch (c) {
    case 'B': return Branch;
    case 'S': return Stem;
    case '-': return LeftTurn;
    case '+': return RightTurn;
    case '[': return Push;
    case ']': return Pop;
    default:  throw std::exception{"Unknown symbol"};
    }
}

union SymbolParam {
    glm::uint u{};
    int i;
    float f;
};

struct VegTemplatesUB {
    constexpr VegTemplatesUB(
        std::string_view s, std::span<const SymbolParam> params,
        std::span<const float> probs, std::span<const String> bodies,
        std::span<const VegTemplate> ts
    ) {
        assert(s.size() <= symbols.size());
        assert(s.size() == params.size());
        assert(probs.size() == ruleProbs.size());
        assert(bodies.size() == ruleBodies.size());
        assert(ts.size() == tmplts.size());
        std::copy(s.begin(), s.end(), symbols.data());
        std::copy(params.begin(), params.end(), symbolParams.data());
        std::copy(probs.begin(), probs.end(), ruleProbs.data());
        std::copy(bodies.begin(), bodies.end(), ruleBodies.data());
        std::copy(ts.begin(), ts.end(), tmplts.data());
        std::for_each(symbols.begin(), symbols.begin() + s.size(), [](char& c) {
            c = std::to_underlying(toSymbol(c));
        });
    }

    std::array<char, k_vegTemplatesSymbolCount> symbols{};
    std::array<SymbolParam, k_vegTemplatesSymbolCount> symbolParams{};
    std::array<float, k_totalRewriteRuleBodyCount> ruleProbs{};
    std::array<String, k_totalRewriteRuleBodyCount> ruleBodies{};
    std::array<VegTemplate, k_vegTemplateCount> tmplts{};
};

constexpr VegTemplatesUB composeVegTemplates() {
    using namespace std::string_view_literals;
    std::string symbols;
    std::vector<SymbolParam> params;
    std::vector<float> probs;
    std::vector<String> bodies;
    std::vector<VegTemplate> tmplts;

    struct ParamStringView {
        constexpr ParamStringView(std::string_view strr)
            : str(strr) {}

        std::string_view str;
        std::span<const SymbolParam> params;
    };

    auto addString = [&symbols, &params](const ParamStringView& str) -> String {
        assert(str.str.size() == str.params.size() || str.params.size() == 0);
        glm::uint offset = symbols.size();
        symbols.append(str.str);
        if (!str.params.empty()) {
            params.append_range(str.params);
        } else {
            params.resize(params.size() + str.str.size(), SymbolParam{.f = 0.07f}); // TODO
        }
        return String{offset, static_cast<glm::uint>(str.str.size())};
    };

    auto addProbRuleBodies =
        [&probs, &bodies](std::initializer_list<std::pair<float, String>> probBodies
        ) -> ProbabilisticRewriteRuleBodies {
        glm::uint first = probs.size();
        for (const auto& pair : probBodies) {
            probs.emplace_back(pair.first);
            bodies.emplace_back(pair.second);
        }
        return ProbabilisticRewriteRuleBodies{
            .firstIndex = first,
            .bodyCount  = static_cast<glm::uint>(probBodies.size())
        };
    };

    auto addTemplate =
        [addString, &tmplts](
            const ParamStringView& axiom, glm::vec2 initSizeTi,
            glm::vec2 sizeChange, glm::vec2 densityStiffness, Wall wallType,
            glm::uint iterCount,
            const std::array<ProbabilisticRewriteRuleBodies, k_rewriteableSymbolCount>& rewriteBodies
        ) {
            tmplts.emplace_back(
                addString(axiom), initSizeTi, sizeChange, densityStiffness,
                wallType, iterCount, rewriteBodies
            );
        };

    auto oakRuleStr    = addString({"SS-[-B+B+B]+[+B-B-B]"sv});
    auto acaciaRuleStr = addString({"S[+B][-B]"sv});
    addTemplate(
        {"B"sv}, {2, 10.0}, {0.875, 1.0}, {4.0, 1.0}, Wall::OakWood, 2,
        {addProbRuleBodies({std::make_pair(1.0f, oakRuleStr)})}
    );
    addTemplate(
        {"B"sv}, {1.75, 30.0}, {0.75, 0.75}, {4.0, 0.5}, Wall::AcaciaWood, 5,
        {addProbRuleBodies({std::make_pair(1.0f, acaciaRuleStr)})}
    );

    addTemplate(
        {"B"sv}, {1.75, 30.0}, {0.75, 0.75}, {4.0, 0.5}, Wall::AcaciaWood, 5,
        {addProbRuleBodies({std::make_pair(1.0f, acaciaRuleStr)})}
    );

    return VegTemplatesUB{
        std::string_view{symbols}, std::span{params}, std::span{probs},
        std::span{bodies}, std::span{tmplts}
    };
}

constexpr VegTemplatesUB k_vegTemplatesUB{composeVegTemplates()};

} // namespace rw
