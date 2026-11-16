/**
 *  @author    Dubsky Tomas
 */
export module realworld:utility.HotReloadIdentifier;

export namespace rw {

/**
 * @brief Lists groups of pipelines with respect to hot reloading
 */
enum class HotReloadIdentifier {
    Default,
    WorldGeneration
};

} // namespace rw
