// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/Executor.h>
#include <xsearch/MetaFile.h>
#include <xsearch/results/Result.h>
#include <xsearch/results/base/Result.h>
#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/tasks/base/DataProvider.h>
#include <xsearch/tasks/base/InplaceProcessor.h>
#include <xsearch/tasks/base/ReturnProcessor.h>
#include <xsearch/tasks/defaults/processors.h>
#include <xsearch/tasks/defaults/readers.h>
#include <xsearch/tasks/defaults/searchers.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/TSQueue.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>
#include <xsearch/utils/string_utils.h>
#include <xsearch/utils/utils.h>

#include <memory>