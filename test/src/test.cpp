#include "test.hpp"
#include "test_framework.hpp"
#include "test_harness.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <optional>
#include <string_view>
#include <utility>

namespace
{

using SuiteFunction = void (*)();

struct SuiteEntry
{
    const char* name;
    SuiteFunction function;
};

std::optional<std::pair<std::size_t, std::string_view>>
FindInvalidSuiteEntry(
    const SuiteEntry* entries,
    std::size_t count)
{
    for (std::size_t index = 0; index < count; ++index)
    {
        if (entries[index].name == nullptr)
        {
            return std::pair{index, std::string_view("null name")};
        }
        if (entries[index].name[0] == '\0')
        {
            return std::pair{index, std::string_view("empty name")};
        }
        if (entries[index].function == nullptr)
        {
            return std::pair{index, std::string_view("null function")};
        }

        for (std::size_t previous = 0; previous < index; ++previous)
        {
            if (std::string_view(entries[index].name) ==
                std::string_view(entries[previous].name))
            {
                return std::pair{index, std::string_view("duplicate name")};
            }
            if (entries[index].function == entries[previous].function)
            {
                return std::pair{
                    index, std::string_view("duplicate function")};
            }
        }
    }
    return std::nullopt;
}

constexpr std::size_t kExpectedSuiteCount = 52;

/* C++ cannot enumerate test declarations, so this table is the manifest. */
constexpr SuiteEntry kSuites[] = {
    {"TestHarness", test::TestHarness::TestSuite},

    /* Data types. */
    {"Test_Nuo_Pair", test::Test_Nuo_Pair::test_nuo_pair},
    {"TestNuoString", test::TestNuoString::test_nuo_string},
    {"TestNuoAny", test::TestNuoAny::test_nuo_any},
    {"TestNuoOptional", test::TestNuoOptional::test_nuo_optional},
    {"TestNuoTuple", test::TestNuoTuple::TestNuoTupleSuite},
    {"TestNuoVariant", test::TestNuoVariant::TestNuoVariantSuite},
    {"TestNuoSourceLocation", test::TestNuoSourceLocation::TestSuite},

    /* Associative containers. */
    {"TestNuoMap", test::TestNuoMap::test_nuo_map},
    {"TestNuoMultimap", test::TestNuoMultimap::test_nuo_multimap},
    {"TestNuoRbTree", test::TestNuoRbTree::test_nuo_rb_tree},
    {"Test_Nuo_Set", test::Test_Nuo_Set::test_nuo_set},
    {"TestNuoUnorderedMap",
     test::TestNuoUnorderedMap::test_nuo_unordered_map},
    {"TestNuoUnorderedMultimap",
     test::TestNuoUnorderedMultimap::test_nuo_unordered_multimap},
    {"TestNuoUnorderedSet",
     test::TestNuoUnorderedSet::test_nuo_unordered_set},
    {"TestNuoUnorderedMultiset",
     test::TestNuoUnorderedMultiset::test_nuo_unordered_multiset},

    /* Sequence containers. */
    {"TestNuoArray", test::TestNuoArray::test_nuo_array},
    {"TestNuoDeque", test::TestNuoDeque::test_nuo_deque},
    {"TestNuoForwardList", test::TestNuoForwardList::test_nuo_forward_list},
    {"TestNuoList", test::TestNuoList::test_nuo_list},
    {"TestNuoQueue", test::TestNuoQueue::test_nuo_queue},
    {"TestNuoStack", test::TestNuoStack::test_nuo_stack},
    {"Test_Nuo_Vector", test::Test_Nuo_Vector::test_nuo_vector},
    {"TestNuoVectorBool", test::TestNuoVectorBool::TestSuite},
    {"TestNuoPriorityQueue",
     test::TestNuoPriorityQueue::TestNuoPriorityQueueSuite},
    {"TestNuoStringView", test::TestNuoStringView::test_nuo_string_view},
    {"TestNuoBitset", test::TestNuoBitset::TestSuite},
    {"TestNuoSpan", test::TestNuoSpan::TestSuite},

    /* Algorithms. */
    {"Test_Nuo_Max", test::Test_Nuo_Max::test_nuo_max},
    {"Test_Nuo_Min", test::Test_Nuo_Min::test_nuo_min},
    {"TestNuoAlgorithm", test::TestNuoAlgorithm::TestNuoAlgorithmSuite},
    {"TestNuoSequenceOperations",
     test::TestNuoSequenceOperations::TestSuite},
    {"TestNuoSetOperations", test::TestNuoSetOperations::TestSuite},
    {"TestNuoNumeric", test::TestNuoNumeric::TestSuite},

    /* Bit utilities. */
    {"TestNuoBit", test::TestNuoBit::TestSuite},

    /* Memory. */
    {"TestNuoUniquePtr", test::TestNuoUniquePtr::TestSuite},
    {"TestNuoSharedPtr", test::TestNuoSharedPtr::TestSuite},
    {"TestNuoMemory", test::TestNuoMemory::TestSuite},
    {"TestNuoAllocatorTraits", test::TestNuoAllocatorTraits::TestSuite},
    {"TestNuoMemoryUtilities",
     test::TestNuoMemoryUtilities::TestSuite},
    {"TestNuoUsesAllocatorConstruction",
     test::TestNuoUsesAllocatorConstruction::TestSuite},
    {"TestNuoAllocateShared", test::TestNuoAllocateShared::TestSuite},

    /* Functional utilities. */
    {"TestNuoFunction", test::TestNuoFunction::TestSuite},

    /* Extra components. */
    {"TestNuoPoolAllocator", test::TestNuoPoolAllocator::TestSuite},
    {"TestNuoFunctional", test::TestNuoFunctional::TestSuite},
    {"TestNuoIterator", test::TestNuoIterator::TestSuite},
    {"TestNuoUtility", test::TestNuoUtility::TestSuite},
    {"TestNuoBigInteger", test::TestNuoBigInteger::TestSuite},
    {"TestNuoComplex", test::TestNuoComplex::TestSuite},
    {"TestNuoMatrix", test::TestNuoMatrix::TestSuite},
    {"TestNuoPolynomial", test::TestNuoPolynomial::TestSuite},
    {"TestFraction", test::TestFraction::test_fraction},
};

static_assert(std::size(kSuites) == kExpectedSuiteCount);

}  /* namespace */

int main()
{
    const auto invalid_suite = FindInvalidSuiteEntry(
        kSuites, std::size(kSuites));
    if (invalid_suite.has_value())
    {
        std::cerr << "[FAIL] invalid suite registry entry "
                  << invalid_suite->first << ": "
                  << invalid_suite->second;
        if (kSuites[invalid_suite->first].name != nullptr)
        {
            std::cerr << ": " << kSuites[invalid_suite->first].name;
        }
        std::cerr << '\n';
        return EXIT_FAILURE;
    }

    const std::size_t failure_count_before = test::ProcessFailureCount();
    test::TestContext context;
    for (const SuiteEntry& suite : kSuites)
    {
        context.RunSuite(suite.name, suite.function);
    }

    const bool had_process_failures =
        test::HasProcessFailuresSince(failure_count_before);
    return context.HasFailures() || had_process_failures
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
