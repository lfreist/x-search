// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ExternSearcher.h>
#include <xsearch/FilePreprocessing.h>
#include <xsearch/MetaFile.h>
#include <xsearch/pipeline/Task.h>
#include <xsearch/pipeline/TaskManager.h>
#include <xsearch/pipeline/processors/decompressors.h>
#include <xsearch/pipeline/reader/BlockReader.h>
#include <xsearch/pipeline/result_collectors/grep.h>
#include <xsearch/pipeline/searchers/Searchers.h>
#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/utils/IOColor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/TSQueue.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>