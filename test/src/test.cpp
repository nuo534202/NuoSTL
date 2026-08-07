#include "test.hpp"

#include <stdio.h>

int main()
{
    /* Core */
    /* Data Types */
    test::Test_Nuo_Pair::test_nuo_pair();

    /* Associative Containers */
    test::TestNuoMap::test_nuo_map();
    test::TestNuoMultimap::test_nuo_multimap();
    test::TestNuoRbTree::test_nuo_rb_tree();
    test::Test_Nuo_Set::test_nuo_set();
    test::TestNuoUnorderedMap::test_nuo_unordered_map();
    test::TestNuoUnorderedMultimap::test_nuo_unordered_multimap();

    /* Sequence Containers */
    test::TestNuoArray::test_nuo_array();
    test::TestNuoDeque::test_nuo_deque();
    test::TestNuoForwardList::test_nuo_forward_list();
    test::TestNuoList::test_nuo_list();
    test::TestNuoQueue::test_nuo_queue();
    test::TestNuoStack::test_nuo_stack();
    test::Test_Nuo_Vector::test_nuo_vector();
    
    /* Algorithms */
    test::Test_Nuo_Max::test_nuo_max();
    test::Test_Nuo_Min::test_nuo_min();

    /* Extra */
    test::TestFraction::test_fraction();
    return 0;
}