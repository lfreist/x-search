// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/results/base/Result.h>

namespace xs::result {

// ----- count results ---------------------------------------------------------
class CountMatchesResult : public base::CountResult {};

// ----- count matching lines --------------------------------------------------
// we straight up inherit CountMatchesResult because all we need is a different
//  type, while functionalities and methods remain the same...
class CountLinesResult : public base::CountResult {};

// ----- search match byte offsets ---------------------------------------------
class MatchByteOffsetsResult : public base::ContainerResult<uint64_t> {};

// ----- search line byte offsets ----------------------------------------------
class LineByteOffsetsResult : public base::ContainerResult<uint64_t> {};

// ----- search indices of matching lines --------------------------------------
class LineIndicesResult : public base::ContainerResult<uint64_t> {};

// ----- search matching lines -------------------------------------------------
class LinesResult : public base::ContainerResult<std::string> {};

}  // namespace xs::result