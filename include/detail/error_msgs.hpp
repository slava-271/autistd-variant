#ifndef SLAVE_DETAIL_ERROR_MSGS_HPP
#define SLAVE_DETAIL_ERROR_MSGS_HPP

#include <string_view>

namespace slave::detail {

inline constexpr std::string_view index_error_msg =
    "slave: alternative index is out of bounds";

inline constexpr std::string_view nothrow_constr_error_msg =
    "slave: type must be nothrow constructible from given arguments";

inline constexpr std::string_view nothrow_dest_error_msg =
    "slave: type must be nothrow destructible";

inline constexpr std::string_view invalid_tag_error_msg =
    "slave: tag value exceeds maximum number of alternatives";

inline constexpr std::string_view variant_count_error =
  "variant should be with more than one type!";

inline constexpr std::string_view switch_count_error =
  "dispatch_switch shouldn't be called with 0 switch threshold. mistake in the development";

} 

#endif
