/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <algorithm>
#include <array>
#include <span>
#include <string_view>
#include <vector>

#include <RealWorld/constants/generation.hpp>
#include <RealWorld/constants/tile.hpp>

namespace rw {
namespace veg_templates {

struct String {
    glm::uint sBegin; // Index of first symbol
    glm::uint sCount; // Number of symbols
    glm::uint pBegin; // Index of first parameter
    glm::uint pCount; // Number of parameters
};

struct Condition {
    glm::uint paramIndex;
    float border;
};

struct RuleBodies {
    Condition cond{};
    glm::uint firstIndex[2]{}; // 0 <=> cond false, 1 <=> cond true
    glm::uint bodyCount[2]{};  // 0 <=> cond false, 1 <=> cond true
};

constexpr int k_rewriteableSymbolCount = 3;

struct alignas(sizeof(glm::vec2)) VegTemplate {
    String axiom;
    glm::vec2 densityStiffness;
    Wall wallType;
    glm::uint iterCount;
    std::array<RuleBodies, k_rewriteableSymbolCount> rules;
};

constexpr int k_totalRewriteRuleBodyCount = 16;

enum class Symbol : uint8_t {
    Twig,
    Branch,
    Stem,
    Rotate,
    Flip,
    Push,
    Pop,
    TwigAdditive   = Twig | 128,
    BranchAdditive = Branch | 128,
    StemAdditive   = Stem | 128
};

constexpr Symbol toSymbol(char c) {
    using enum Symbol;
    switch (c) {
    case 'T': return Twig;
    case 't': return TwigAdditive;
    case 'B': return Branch;
    case 'b': return BranchAdditive;
    case 'S': return Stem;
    case 's': return StemAdditive;
    case '+':
    case '-': return Rotate;
    case 'F': return Flip;
    case '[': return Push;
    case ']': return Pop;
    default:  throw std::exception{"Unknown symbol"};
    }
}

constexpr int paramCount(Symbol s) {
    using enum Symbol;
    switch (s) {
    case Twig:
    case TwigAdditive:   return 2;
    case Branch:
    case BranchAdditive: return 2;
    case Stem:
    case StemAdditive:   return 2;
    case Rotate:         return 1;
    case Flip:           return 0;
    case Push:           return 0;
    case Pop:            return 0;
    default:             throw std::exception{"Unknown symbol"};
    }
}
constexpr int paramCount(char c) {
    return paramCount(toSymbol(c));
}

union SymbolParam {
    constexpr SymbolParam() {}
    constexpr SymbolParam(glm::uint uu)
        : u(uu) {}
    constexpr SymbolParam(int ii)
        : i(ii) {}
    constexpr SymbolParam(float ff)
        : f(ff) {}

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
        int expectedParamCount = 0;
        for (const auto& c : s) {
            expectedParamCount += paramCount(toSymbol(c));
        }
        assert(expectedParamCount == params.size());
        assert(probs.size() <= ruleProbs.size());
        assert(bodies.size() <= ruleBodies.size());
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

template<typename T>
concept SymbolParamType = std::is_same_v<T, float> || std::is_same_v<T, int> ||
                          std::is_same_v<T, glm::uint>;

struct DefaultParams {
    glm::vec2 branchSizeTi;
    float turnAngle;
};

// std::strlen is not constexpr...
constexpr int strlen(const char* str) {
    int length = 0;
    while (*(str++) != '\0') { length++; }
    return length;
}

struct ParamString {
    template<typename... Types>
    constexpr ParamString(const DefaultParams& def, Types... args) {
        decode(&def, args...);
    }

    template<typename... Types>
    constexpr ParamString(Types... args) {
        decode(nullptr, args...);
    }

    std::string str;
    std::vector<SymbolParam> params;

private:
    template<typename... Types>
    constexpr void decode(const DefaultParams* def, const char* toAppend, Types... args) {
        if (missingParamCount > 0) {
            appendDefaultParams(str.back(), def);
        }
        auto toAppendLength = strlen(toAppend);
        str.append(toAppend, toAppend + toAppendLength);
        for (size_t i = 0; i < toAppendLength - 1; ++i) {
            appendDefaultParams(toAppend[i], def);
        }
        missingParamCount = paramCount(str.back());
        decode(def, args...);
    }

    template<typename... Types>
    constexpr void decode(const DefaultParams* def, char c, Types... args) {
        if (missingParamCount > 0) {
            appendDefaultParams(str.back(), def);
        }
        str.push_back(c);
        missingParamCount = paramCount(str.back());
        decode(def, args...);
    }

    template<SymbolParamType ParamType, typename... Types>
    constexpr void decode(const DefaultParams* def, ParamType param, Types... args) {
        assert(missingParamCount > 0);
        params.emplace_back(param);
        missingParamCount--;
        decode(def, args...);
    }

    template<typename... Types>
    constexpr void decode(const DefaultParams* def, double param, Types... args) {
        decode(def, static_cast<float>(param), args...);
    }

    template<typename... Types>
    constexpr void decode(const DefaultParams* def) {
        if (missingParamCount > 0) {
            assert(missingParamCount == paramCount(str.back()));
            appendDefaultParams(str.back(), def);
        }
    }

    constexpr void appendDefaultParams(char c, const DefaultParams* def) {
        switch (c) {
        case 'T':
        case 't':
        case 'B':
        case 'b':
        case 'S':
        case 's':
            params.emplace_back(def->branchSizeTi.x);
            params.emplace_back(def->branchSizeTi.y);
            break;
        case '+': params.emplace_back(def->turnAngle); break;
        case '-': params.emplace_back(-def->turnAngle); break;
        case 'F':
        case '[':
        case ']': break;
        default:  throw std::exception{"Unknown symbol"};
        }
    }

    int missingParamCount = 0;
};

constexpr VegTemplatesUB composeVegTemplates() {
    using namespace std::string_view_literals;
    std::string symbols;
    std::vector<SymbolParam> params;
    std::vector<float> probs;
    std::vector<String> bodies;
    std::vector<VegTemplate> tmplts;

    auto addString = [&symbols, &params](const ParamString& str) -> String {
        glm::uint sBegin = symbols.size();
        symbols.append(str.str);
        glm::uint pBegin = params.size();
        params.append_range(str.params);
        return String{
            sBegin, static_cast<glm::uint>(str.str.size()), pBegin,
            static_cast<glm::uint>(str.params.size())
        };
    };

    auto addProbRuleBodies =
        [&probs, &bodies](
            Condition cond,
            std::initializer_list<std::pair<float, String>> condFalseProbBodies,
            std::initializer_list<std::pair<float, String>> condTrueProbBodies
        ) -> RuleBodies {
        glm::uint condFalseFirst = probs.size();
        for (const auto& pair : condFalseProbBodies) {
            probs.emplace_back(pair.first);
            bodies.emplace_back(pair.second);
        }
        glm::uint condTrueFirst = probs.size();
        for (const auto& pair : condTrueProbBodies) {
            probs.emplace_back(pair.first);
            bodies.emplace_back(pair.second);
        }
        return RuleBodies{
            .cond       = cond,
            .firstIndex = {condFalseFirst, condTrueFirst},
            .bodyCount =
                {static_cast<glm::uint>(condFalseProbBodies.size()),
                 static_cast<glm::uint>(condTrueProbBodies.size())}
        };
    };

    { // Oak
        DefaultParams oakDefParams{.branchSizeTi = {.5, 18.}, .turnAngle = .08};
        auto t0 = addString({'t', .125, 4.});
        auto t1 = addString({oakDefParams, 'b', .25, 4., "F-[T]+", .16, "[T]"});
        auto t2 =
            addString({oakDefParams, 'b', .25, 4., "F-[T]+", 0.04, "[T]+[T]"});
        auto t3 =
            addString({oakDefParams, 'b', .25, 4., "F-[T]+", 0.12, "[T]+[T]"});

        auto b0 = addString({'b', 0.25, 3.});
        auto b1 = addString({'b', 0.25, 2.});

        tmplts.push_back(VegTemplate{
            .axiom            = addString({'T', 0.5f, 18.0f}),
            .densityStiffness = {4., .0625},
            .wallType         = Wall::OakWood,
            .iterCount        = 5,
            .rules =
                {addProbRuleBodies(
                     {1, 18.f}, {{1.f, t0}},
                     {{.1f, t1}, {.35f, t2}, {.35f, t3}, {.1f, t0}}
                 ),
                 addProbRuleBodies({1, 25.f}, {{1.f, b0}}, {{1.f, b1}}),
                 addProbRuleBodies({0, 0.f}, {}, {})}
        });
    }

    { // Acacia
        DefaultParams acaDefParams{.branchSizeTi = {1.75, 30.0}, .turnAngle = 0.08f};
        auto acaciaRuleStr = addString({acaDefParams, "S[+B][-B]"});

        tmplts.push_back(VegTemplate{
            .axiom            = addString({acaDefParams, 'B'}),
            .densityStiffness = {4.0, 0.5},
            .wallType         = Wall::AcaciaWood,
            .iterCount        = 5,
            .rules =
                {addProbRuleBodies({1, 20.0f}, {}, {}),
                 addProbRuleBodies({1, 20.0f}, {}, {{1.0f, acaciaRuleStr}})}
        });
    }

    tmplts.push_back(VegTemplate{});

    return VegTemplatesUB{
        std::string_view{symbols}, std::span{params}, std::span{probs},
        std::span{bodies}, std::span{tmplts}
    };
}
} // namespace veg_templates

constexpr veg_templates::VegTemplatesUB k_vegTemplatesUB{
    veg_templates::composeVegTemplates()
};

} // namespace rw
