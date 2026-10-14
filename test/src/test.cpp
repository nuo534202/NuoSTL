#include "test.hpp"

#include <stdio.h>

int main()
{
    /* Core */
    /* Data Types */
    test::Test_Nuo_Pair::test_nuo_pair();
    test::TestNuoString::test_nuo_string();
    test::TestNuoAny::test_nuo_any();
    test::TestNuoOptional::test_nuo_optional();
    test::TestNuoTuple::TestNuoTupleSuite();
    test::TestNuoVariant::TestNuoVariantSuite();

    /* Associative Containers */
    test::TestNuoMap::test_nuo_map();
    test::TestNuoMultimap::test_nuo_multimap();
    test::TestNuoRbTree::test_nuo_rb_tree();
    test::Test_Nuo_Set::test_nuo_set();
    test::TestNuoUnorderedMap::test_nuo_unordered_map();
    test::TestNuoUnorderedMultimap::test_nuo_unordered_multimap();
    test::TestNuoUnorderedSet::test_nuo_unordered_set();
    test::TestNuoUnorderedMultiset::test_nuo_unordered_multiset();

    /* Sequence Containers */
    test::TestNuoArray::test_nuo_array();
    test::TestNuoDeque::test_nuo_deque();
    test::TestNuoForwardList::test_nuo_forward_list();
    test::TestNuoList::test_nuo_list();
    test::TestNuoQueue::test_nuo_queue();
    test::TestNuoStack::test_nuo_stack();
    test::Test_Nuo_Vector::test_nuo_vector();
    test::TestNuoVectorBool::TestSuite();
    test::TestNuoPriorityQueue::TestNuoPriorityQueueSuite();
    test::TestNuoStringView::test_nuo_string_view();
    test::TestNuoBitset::TestSuite();
    test::TestNuoSpan::TestSuite();
    
    /* Algorithms */
    test::Test_Nuo_Max::test_nuo_max();
    test::Test_Nuo_Min::test_nuo_min();
    test::TestNuoAlgorithm::TestNuoAlgorithmSuite();
    test::TestNuoSequenceOperations::TestSuite();
    test::TestNuoSetOperations::TestSuite();
    test::TestNuoNumeric::TestSuite();

    /* Memory */
    test::TestNuoUniquePtr::TestSuite();
    test::TestNuoSharedPtr::TestSuite();
    test::TestNuoMemory::TestSuite();
    test::TestNuoAllocatorTraits::TestSuite();

    /* Functional */
    test::TestNuoFunction::TestSuite();

    /* Extra */
    test::TestNuoPoolAllocator::TestSuite();
    test::TestNuoBigInteger::TestSuite();
    test::TestNuoComplex::TestSuite();
    test::TestNuoMatrix::TestSuite();
    test::TestNuoPolynomial::TestSuite();
    test::TestFraction::test_fraction();
    return 0;
}
