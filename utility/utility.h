#pragma once
#include <algorithm>

template <typename InputIt, typename OutputIt>
OutputIt move_copy(InputIt first, InputIt last, OutputIt d_first) {
  return std::copy(std::make_move_iterator(first),
                   std::make_move_iterator(last), d_first);
}
