#pragma once

/* 1. C++ STL Core Components */

/* Data Types */
#include "core/data_types/nuo_any.hpp"
#include "core/data_types/nuo_pair.hpp"
#include "core/data_types/nuo_optional.hpp"
#include "core/data_types/nuo_string.hpp"
#include "core/data_types/nuo_tuple.hpp"
#include "core/data_types/nuo_variant.hpp"

/* Associative Containers */
#include "core/assoc_cont/nuo_map.hpp"
#include "core/assoc_cont/nuo_set.hpp"
#include "core/assoc_cont/nuo_unordered_map.hpp"
#include "core/assoc_cont/nuo_unordered_set.hpp"

/* Sequence Container */
#include "core/seq_cont/nuo_array.hpp"
#include "core/seq_cont/nuo_deque.hpp"
#include "core/seq_cont/nuo_list.hpp"
#include "core/seq_cont/nuo_queue.hpp"
#include "core/seq_cont/nuo_priority_queue.hpp"
#include "core/seq_cont/nuo_stack.hpp"
#include "core/seq_cont/nuo_string_view.hpp"
#include "core/seq_cont/nuo_vector.hpp"
#include "core/seq_cont/nuo_vector_bool.hpp"

/* Algorithms */
#include "core/algorithms/nuo_max.hpp"
#include "core/algorithms/nuo_min.hpp"
#include "core/algorithms/nuo_binary_search.hpp"
#include "core/algorithms/nuo_heap.hpp"
#include "core/algorithms/nuo_sort.hpp"
#include "core/algorithms/nuo_sort_operation.hpp"
#include "core/algorithms/nuo_sequence_operations.hpp"
#include "core/algorithms/nuo_set_operations.hpp"
#include "core/algorithms/nuo_numeric.hpp"

/* Memory */
#include "core/memory/nuo_unique_ptr.hpp"
#include "core/memory/nuo_shared_ptr.hpp"

/* 2. Extra Components */
#include "extra/nuo_fraction.hpp"
#include "extra/nuo_big_integer.hpp"
#include "extra/nuo_complex.hpp"
#include "extra/nuo_matrix.hpp"
#include "extra/nuo_polynomial.hpp"

/* 3. Utils */
#include "utils/nuo_allocator.hpp"
#include "utils/nuo_block_pool.hpp"
#include "utils/nuo_pool_allocator.hpp"
#include "utils/nuo_construct.hpp"
#include "utils/nuo_destroy.hpp"
#include "utils/nuo_exceptdef.hpp"
#include "utils/nuo_functional.hpp"
#include "utils/nuo_iterator.hpp"
#include "utils/nuo_math.hpp"
#include "utils/nuo_type_traits.hpp"
#include "utils/nuo_util.hpp"
