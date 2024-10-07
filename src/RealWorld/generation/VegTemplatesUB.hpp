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

// NOLINTBEGIN: Executed only in compile time

struct String {
    glm::uint sBegin{}; // Index of first symbol
    glm::uint sCount{}; // Number of symbols
    glm::uint pBegin{}; // Index of first parameter
    glm::uint pCount{}; // Number of parameters
};

struct Condition {
    glm::uint paramIndex{};
    float border{};
};

struct RuleBodies {
    Condition cond{};
    glm::uint firstIndex[2]{}; // 0 <=> cond false, 1 <=> cond true
    glm::uint bodyCount[2]{};  // 0 <=> cond false, 1 <=> cond true
};

constexpr int k_rewriteableSymbolCount = 3;

struct VegTemplate {
    String axiom;
    glm::uint iterCount;
    float tropismFactor;
    std::array<RuleBodies, k_rewriteableSymbolCount> rules;
};

struct VegRasterTemplate {
    float noiseScale;
    float branchRadiusFactor;
    float maxLeafStrength;
};

enum class Symbol : uint8_t {
    Twig,
    Branch,
    Stem,
    Rotate,
    TropismRotate,
    Flip,
    Push,
    Pop,
    Density,
    Stiffness,
    WallType,
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
    case '<':
    case '>':
    case '^': return TropismRotate;
    case 'F': return Flip;
    case '[': return Push;
    case ']': return Pop;
    case 'D': return Density;
    case 'I': return Stiffness;
    case 'W': return WallType;
    default:  throw std::runtime_error{"Unknown symbol"};
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
    case TropismRotate:  return 1;
    case Flip:           return 0;
    case Push:           return 0;
    case Pop:            return 0;
    case Density:        return 1;
    case Stiffness:      return 1;
    case WallType:       return 1;
    default:             throw std::runtime_error{"Unknown symbol"};
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

constexpr int k_vegTemplatesSymbolCount   = 384;
constexpr int k_totalRewriteRuleBodyCount = 48;

struct VegTemplatesUB {
    constexpr VegTemplatesUB(
        std::string_view s, std::span<const SymbolParam> params,
        std::span<const float> probs, std::span<const String> bodies,
        std::span<const VegTemplate> ts, std::span<const VegRasterTemplate> rts
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
        assert(rts.size() == rasterTmplts.size());
        std::copy(s.begin(), s.end(), symbols.data());
        std::copy(params.begin(), params.end(), symbolParams.data());
        std::copy(probs.begin(), probs.end(), ruleProbs.data());
        std::copy(bodies.begin(), bodies.end(), ruleBodies.data());
        std::copy(ts.begin(), ts.end(), tmplts.data());
        std::copy(rts.begin(), rts.end(), rasterTmplts.data());
        std::for_each(symbols.begin(), symbols.begin() + s.size(), [](char& c) {
            c = std::to_underlying(toSymbol(c));
        });
    }

    std::array<char, k_vegTemplatesSymbolCount> symbols{};
    std::array<SymbolParam, k_vegTemplatesSymbolCount> symbolParams{};
    std::array<float, k_totalRewriteRuleBodyCount> ruleProbs{};
    std::array<String, k_totalRewriteRuleBodyCount> ruleBodies{};
    std::array<VegTemplate, k_lSystemSpeciesCount> tmplts{};
    std::array<VegRasterTemplate, k_lSystemSpeciesCount> rasterTmplts{};
};

template<typename T>
concept SymbolParamType = std::is_same_v<T, float> || std::is_same_v<T, int> ||
                          std::is_same_v<T, glm::uint>;

struct DefaultParams {
    glm::vec2 branchSizeTi;
    float turnAngle;
    float density;
    float stiffness;
    Wall wallType;
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
    constexpr void decode(const DefaultParams* def, Wall wallType, Types... args) {
        decode(def, static_cast<glm::uint>(std::to_underlying(wallType)), args...);
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
        case '<': params.emplace_back(0.25f); break;
        case '>': params.emplace_back(-0.25f); break;
        case '^': params.emplace_back(0.0f); break;
        case 'F':
        case '[':
        case ']': break;
        case 'D': params.emplace_back(def->density); break;
        case 'I': params.emplace_back(def->stiffness); break;
        case 'W': params.emplace_back(std::to_underlying(def->wallType)); break;
        default:  throw std::runtime_error{"Unknown symbol"};
        }
    }

    int missingParamCount = 0;
};

constexpr VegTemplatesUB composeVegTemplates() {
    std::string symbols;
    std::vector<SymbolParam> params;
    std::vector<float> probs;
    std::vector<String> bodies;
    std::vector<VegTemplate> tmplts;
    std::vector<VegRasterTemplate> rasterTmplts;

    auto addString = [&symbols, &params](const ParamString& str) -> String {
        glm::uint sBegin = symbols.size();
        symbols.append(str.str);
        glm::uint pBegin = params.size();
        for (auto param : str.params) { params.emplace_back(param); }
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
        DefaultParams def{
            .branchSizeTi = {0.5f, 3.5f},
            .turnAngle    = 0.08f,
            .density      = 3.0f,
            .stiffness    = 0.0625f,
            .wallType     = Wall::OakWood
        };
        auto t0 = addString({'t', .125, 2.});
        auto t1 = addString({def, 'b', .25, .5, "F-[T]+", .16, "[T]"});
        auto t2 = addString({def, 'b', .25, .5, "F-[T]+", 0.04, "[T]+[T]"});
        auto t3 = addString({def, 'b', .25, .5, "F-[T]+", 0.12, "[T]+[T]"});

        auto b0 = addString({'b', 0.25, .25});
        auto b1 = addString({'b', 0.25, .125});

        tmplts.push_back(VegTemplate{
            .axiom         = addString({def, "DIWT", .75, 3.5}),
            .iterCount     = 4,
            .tropismFactor = 4.0f,
            .rules =
                {addProbRuleBodies(
                     {1, 3.5f}, {{1.f, t0}},
                     {{.1f, t1}, {.35f, t2}, {.35f, t3}, {.1f, t0}}
                 ),
                 addProbRuleBodies({1, 5.f}, {{1.f, b0}}, {{1.f, b1}})}
        });
        rasterTmplts.push_back(VegRasterTemplate{
            .noiseScale = 1.0f / 16.0f, .branchRadiusFactor = 3.0f, .maxLeafStrength = 7.0f
        });
    }

    { // Acacia
        DefaultParams def{
            .branchSizeTi = {.5, 35.0},
            .turnAngle    = 0.08f,
            .density      = 2.0f,
            .stiffness    = 0.125f,
            .wallType     = Wall::AcaciaWood
        };
        auto t0 = addString({def, 't', .125, 5.0});
        auto t1 = addString({def, "B+", +0.03, "[^+T][^-T]"});
        auto t2 = addString({def, "B+", +0.03, "[^+T][^-T", .375, 25., "]"});
        auto t3 = addString({def, "B+", -0.03, "[^+T", .375, 25., "][^-T]"});

        auto b0 = addString({def, 'b', .25, 5.});

        tmplts.push_back(VegTemplate{
            .axiom         = addString({def, "DIWB", .75, 80., "[+T][-T]"}),
            .iterCount     = 5,
            .tropismFactor = -0.8f,
            .rules =
                {addProbRuleBodies(
                     {1, 35.0f}, {{1.0f, t0}},
                     {{0.8f, t1}, {0.05f, t2}, {0.05f, t3}, {0.1f, t0}}
                 ),
                 addProbRuleBodies({1, 50.0f}, {{1.0f, b0}}, {{0.9f, b0}})}
        });
        rasterTmplts.push_back(VegRasterTemplate{
            .noiseScale = 1.0f / 8.0f, .branchRadiusFactor = 2.5f, .maxLeafStrength = 3.0f
        });
    }

    { // Spruce
        DefaultParams def{
            .branchSizeTi = {.5, 4.},
            .turnAngle    = 0.18f,
            .density      = 2.0f,
            .stiffness    = 0.125f,
            .wallType     = Wall::ConiferousWood
        };
        auto t0 = addString({def, 't', .0, 2.5});

        auto b0 = addString({def, 'b', .125, 2.0});
        auto b1 = addString(
            {def, "s", .125, 2.5, "[+T+", -0.04, "T][^B", .5, 6.0, "][-T+",
             0.04, "T]"}
        );
        auto b2 =
            addString({def, "s", .125, 2.5, "[F+T+", -0.04, "T][^B", .5, 6.0, "]"});

        auto s0 = addString({def, 's', .25, 1.5});

        tmplts.push_back(VegTemplate{
            .axiom         = addString({def, "DIWB", .5, 7.5}),
            .iterCount     = 6,
            .tropismFactor = 0.0f,
            .rules =
                {addProbRuleBodies({1, 15.0f}, {{1.0f, t0}}, {{1.0f, t0}}),
                 addProbRuleBodies(
                     {1, 6.0f}, {{0.95f, b0}}, {{0.7f, b1}, {0.2f, b2}, {0.05f, b0}}
                 ),
                 addProbRuleBodies({1, 40.0f}, {{0.95f, s0}}, {{0.8f, s0}})}
        });
        rasterTmplts.push_back(VegRasterTemplate{
            .noiseScale = 1.0f / 8.0f, .branchRadiusFactor = 5.0f, .maxLeafStrength = 4.0f
        });
    }

    { // Willow
        DefaultParams def{
            .branchSizeTi = {0.5f, 12.0f},
            .turnAngle    = 0.08f,
            .density      = 2.0f,
            .stiffness    = 0.125f,
            .wallType     = Wall::OakWood
        };
        auto t0 = addString({def, 't', .0, 5.});

        auto b0 = addString(
            {def, 's', .0, 2.5, "[FI", 0.002f, 'W', Wall::Withy, "[+", -0.23,
             "T][+", 0.1, "T][+", 0.26, "T]]"}
        );
        auto b1 = addString(
            {def, 's', .0, 2.5, "[FI", 0.002f, 'W', Wall::Withy, "[+", -0.34,
             "T][+", 0.13, "T][+", 0.11, "T][+", 0.21, "T]]"}
        );

        auto s0 = addString({def, 's', 0.25, 5.});
        auto s1 = addString(
            {def, 's', 0.25, 0., "F[+", -0.1, "B", .75, 2.5, "][+", 0.03, "B",
             1.25, 4.0, "]"}
        );

        tmplts.push_back(VegTemplate{
            .axiom         = addString({def, "DIWS", 1.25, 25.}),
            .iterCount     = 6,
            .tropismFactor = 0.0f,
            .rules =
                {addProbRuleBodies({1, 27.f}, {{1.f, t0}}, {}),
                 addProbRuleBodies({1, 0.f}, {}, {{.35f, b0}, {.65f, b1}}),
                 addProbRuleBodies({1, 15.f}, {{1.f, s0}}, {{.9f, s1}, {.1f, s0}})}
        });
        rasterTmplts.push_back(VegRasterTemplate{
            .noiseScale = 0.0f, .branchRadiusFactor = 3.0f, .maxLeafStrength = 0.0f
        });
    }

    { // Cactus
        DefaultParams def{
            .branchSizeTi = {1.5f, 7.5f},
            .turnAngle    = 0.22f,
            .density      = 2.0f,
            .stiffness    = 0.25f,
            .wallType     = Wall::Cactus
        };

        auto t0 = addString({def, 't', 0.125, .5});

        auto b0 = addString({def, 'b', 0.125, 5.});
        auto b1 = addString(
            {def, 's', 0.25, 0., "F[-T", 1.5, 3.5, "+T][B", 2.0, 12.0, "]"}
        );
        auto b2 = addString(
            {def, 's', 0.25, 0., "F[-T", 1.5, 3.5, "+T][B", 2.0, 12.0, "][+T",
             1.5, 3.0, "-T", 1.5, 5.5, "]"}
        );

        tmplts.push_back(VegTemplate{
            .axiom         = addString({def, "DIWB", 2.25, 15.}),
            .iterCount     = 5,
            .tropismFactor = 0.0f,
            .rules =
                {addProbRuleBodies({1, 0.f}, {}, {{.2f, t0}}),
                 addProbRuleBodies(
                     {1, 15.f}, {{.9f, b0}}, {{.4f, b1}, {.1f, b2}, {.4f, b0}}
                 )}
        });
        rasterTmplts.push_back(VegRasterTemplate{
            .noiseScale = 1.0f / 16.0f, .branchRadiusFactor = 3.0f, .maxLeafStrength = 0.0f
        });
    }

    { // Palm tree
        DefaultParams def{
            .branchSizeTi = {0.5f, 19.f},
            .turnAngle    = 0.25f,
            .density      = 1.0f,
            .stiffness    = 0.045f,
            .wallType     = Wall::PalmWood
        };
        auto t0 = addString({def, 't', .0, 2.5});

        auto b0 = addString({def, 'b', .125, 6.0});
        auto b1 = addString({def, 'b', .125, 5.0, "[+", -0.2, "T]"});
        auto b2 = addString({def, 'b', .125, 5.0, "[+", +0.2, "T]"});
        auto b3 = addString({def, 'b', .125, 5.0, "[+", -0.09, "T]"});
        auto b4 = addString({def, 'b', .125, 5.0, "[+", +0.09, "T]"});

        tmplts.push_back(VegTemplate{
            .axiom = addString({def, "DIW^B", .75, 3.5, 'D', 0.05, 'I', 2.0}),
            .iterCount     = 8,
            .tropismFactor = 0.0f,
            .rules =
                {addProbRuleBodies({1, 32.0f}, {{0.95f, t0}}, {{0.4f, t0}}),
                 addProbRuleBodies(
                     {1, 6.0f}, {{0.9f, b0}},
                     {{0.2f, b0}, {0.2f, b1}, {0.2f, b2}, {0.2f, b3}, {0.2f, b4}}
                 )}
        });
        rasterTmplts.push_back(VegRasterTemplate{
            .noiseScale = 1.0f / 4.0f, .branchRadiusFactor = 5.0f, .maxLeafStrength = 5.0f
        });
    }

    return VegTemplatesUB{std::string_view{symbols}, std::span{params},
                          std::span{probs},          std::span{bodies},
                          std::span{tmplts},         std::span{rasterTmplts}};
}

// NOLINTEND

} // namespace veg_templates

constexpr veg_templates::VegTemplatesUB k_vegTemplatesUB{
    veg_templates::composeVegTemplates()
};

} // namespace rw
