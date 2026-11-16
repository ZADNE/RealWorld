/**
 *  @author    Dubsky Tomas
 */
export module realworld:simulation.vegetation.BranchRange;

export namespace rw {

/**
 * @brief Represents a continuous range of branches in the branch buffer
 */
struct BranchRange {
    int begin{};
    int count{};
};

} // namespace rw
