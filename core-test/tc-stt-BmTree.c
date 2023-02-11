// Wanda
#include "bemage-stt.h"
#include "bemage-test.h"

#include <stdlib.h>
#include <stdio.h>

START_TEST(test_BmTree_init)
{
    BmCode* domains= BmCode_initialize( newBmCodeBasic(3, 0), 3, 2, 3, 2);
    BmTree* tree= newBmTree( domains, 4);
    deleteBmCode( domains );

    ck_assert_uint_eq( BmCode_size(tree->input), (uint)3 );
    ck_assert_uint_eq( tree->capacity, (uint)1 );
    ck_assert_uint_eq( tree->size, (uint)0 );
    ck_assert_uint_eq( tree->optionBound, (uint)5 );
    
    //for( uint i = 1 ; i <= 2 ; ++i )
    //    ck_assert_uint_eq( tree->branches[0][i], (uint)0 );

    BmCode* st= newBmCodeBasic(3, 0);
    BmCode_setCodeFirst(tree->input, st);
    while( BmCode_isIncluding(tree->input, st) )
    {
        ck_assert_uint_eq( BmTree_at(tree, st), (uint)0 );
        BmCode_nextCode(tree->input, st);
    }

    deleteBmCode( st );
    deleteBmTree( tree );
}
END_TEST


START_TEST(test_BmTree_treeConstruction)
{
    BmCode* domains= BmCode_initialize( newBmCodeBasic(3, 0), 3, 2, 3, 2);
    BmTree* tree= newBmTree( domains, 4);
    deleteBmCode( domains );
    
    /* Tree:
     *  root   :           (2)
     *         :          / | \ 
     *         :         1  2  3
     *         :        /   |   \ 
     *  b1, b2 :      (1)  -1-   (3) 
     *         :     / |          | \ 
     *         :    1  2          1  2
     *         :   /   |          |   \ 
     *         : -3-  -1-        -2-  -4-
     */

    uint root= BmTree_newBranch( tree, 2, 1 );

    ck_assert_uint_eq( root, (uint)0 );
    ck_assert_uint_eq( BmTree_branchVariable(tree, root ), (uint)2 );
    ck_assert_uint_eq( BmTree_branchSize(tree, root ), (uint)1 );

    uint b1=  BmTree_newBranch( tree, 1, 3 );
    BmTree_branch_option_output( tree, b1, 2, 1 );
    
    ck_assert_uint_eq( b1, (uint)1 );
    ck_assert_uint_eq( BmTree_branchVariable(tree, b1 ), (uint)1 );
    ck_assert_uint_eq( BmTree_branchSize(tree, b1 ), (uint)2 );

    BmTree_branch_option_connect( tree, root, 1, b1 );
    
    ck_assert_uint_eq( BmTree_branchSize(tree, root ), (uint)2 );

    uint b2=  BmTree_newBranch( tree, 3, 2 );
    BmTree_branch_option_output( tree, b2, 2, 4 );
    
    ck_assert_uint_eq( b2, (uint)2 );
    ck_assert_uint_eq( BmTree_branchVariable(tree, b2 ), (uint)3 );
    ck_assert_uint_eq( BmTree_branchSize(tree, b2 ), (uint)2 );

    BmTree_branch_option_connect( tree, root, 3, b2 );
    
    ck_assert_uint_eq( BmTree_branchSize(tree, root ), (uint)3 );

    ck_assert_str_eq(
        BmTree_wording(tree),
        "input: [2, 3, 2], size: 3\n\
0. input(2): [branch(1), leaf(1), branch(2)]\n\
1. input(1): [leaf(3), leaf(1)]\n\
2. input(3): [leaf(2), leaf(4)]" );

    deleteBmTree( tree );
}
END_TEST

START_TEST(test_BmTree_optionSelection)
{
    BmCode* domains= BmCode_initialize( newBmCodeBasic(3, 0), 3, 2, 3, 2);
    BmTree* tree= newBmTree( domains, 4);
    deleteBmCode( domains );

    uint root= BmTree_newBranch( tree, 2, 1 );
    uint b1=  BmTree_newBranch( tree, 1, 3 );
    BmTree_branch_option_connect( tree, root, 1, b1 );
    uint b2=  BmTree_newBranch( tree, 3, 2 );
    BmTree_branch_option_output( tree, b2, 2, 4 );
    BmTree_branch_option_connect( tree, root, 3, b2 );
    uint b3=  BmTree_newBranch( tree, 3, 1 );
    BmTree_branch_option_output( tree, b3, 2, 3 );
    BmTree_branch_option_connect( tree, b1, 2, b3 );
    

    /* Tree:
     *  root   :           (2)
     *         :          / | \ 
     *         :         1  2  3
     *         :        /   |   \ 
     *  b1, b2 :      (1)  -1-   (3) 
     *         :     / |          | \ 
     *         :    1  2          1  2
     *         :   /   |          |   \ 
     *  b3     : -3-  (3)        -2-  -4-
     *         :       | \
     *         :       1  2
     *         :       |   \
     *         :      -1-  -3-
     */

    ck_assert_str_eq(
        BmTree_wording(tree),
        "input: [2, 3, 2], size: 4\n\
0. input(2): [branch(1), leaf(1), branch(2)]\n\
1. input(1): [leaf(3), branch(3)]\n\
2. input(3): [leaf(2), leaf(4)]\n\
3. input(3): [leaf(1), leaf(3)]" );

    BmCode* st= newBmCode(3, 2, 1, 1);
    ck_assert_uint_eq( BmTree_at(tree, st), (uint)1 );
    
    BmCode_initialize(st, 3, 1, 1, 1);
    ck_assert_uint_eq( BmTree_at(tree, st), (uint)3 );
    
    BmCode_initialize(st, 3, 1, 2, 2);
    ck_assert_uint_eq( BmTree_at(tree, st), (uint)1 );
    
    BmCode_initialize(st, 3, 1, 3, 2);
    ck_assert_uint_eq( BmTree_at(tree, st), (uint)4 );

    deleteBmCode( st );
    deleteBmTree( tree );
}
END_TEST

START_TEST(test_BmTree_constructionFromExemple)
{
    BmCode* domains= BmCode_initialize( newBmCodeBasic(3, 0), 3, 2, 3, 2);
    BmTree* tree= newBmTree( domains, 4);
    deleteBmCode( domains );

    ck_assert_uint_eq( tree->size, (uint)0 );

    char buffer[1024];
    ck_assert_str_eq(
        BmTree_wording(tree),
        "input: [2, 3, 2], size: 0" );

    /* Tree:
     *                    (2)
     *                   / | \ 
     *                  1  2  3
     *                 /   |   \ 
     *               -1-  -1-   -1-
     */

    BmTree_initializeWhith_on( tree, 2, 1 );
    
    strcpy( buffer, "");
    ck_assert_str_eq(
        BmTree_wording(tree),
        "input: [2, 3, 2], size: 1\n\
0. input(2): [leaf(1), leaf(1), leaf(1)]" );

    /* Tree: + [0, 1, 1] on 3
     *
     *               (2)
     *              / | \ 
     *             1  2  3
     *            /   |   \ 
     *          (3)  -1-   -1- 
     *         / |          
     *        1  2          
     *       /   |          
     *     -3-  -1-        
     */

    BmCode *code= newBmCode(3,  0, 1, 1 );
    BmTree_at_set( tree, code, 3 );

    strcpy(buffer, BmTree_wording(tree));
    ck_assert_str_eq(
        buffer,
        "input: [2, 3, 2], size: 2\n\
0. input(2): [branch(1), leaf(1), leaf(1)]\n\
1. input(3): [leaf(3), leaf(1)]" );

    /* Tree: + [2, 0, 2] on 4
     *
     *               (2)       
     *              / | \     
     *             1  2  3    
     *            /   |   \    
     *          (3)  (1)   (1) 
     *         / |    | \    \  \
     *        1  2    1  2    1  2
     *       /   |    |  |    |   \
     *     -3-  (1)  -1- |   -1-  | 
     *         / |      (3)      (3)
     *       -1--4-      | \      | \
     *                  -1--4-   -1--4-
     * 
     */

    BmCode_initialize( code, 3, 2, 0, 2 );
    uint deadBranches= BmTree_at_set( tree, code, 4 );

    ck_assert_uint_eq( deadBranches, 0 );

    strcpy(buffer, BmTree_wording(tree));
    ck_assert_str_eq(
        buffer,
        "input: [2, 3, 2], size: 7\n\
0. input(2): [branch(1), branch(3), branch(5)]\n\
1. input(3): [leaf(3), branch(2)]\n\
2. input(1): [leaf(1), leaf(4)]\n\
3. input(1): [leaf(1), branch(4)]\n\
4. input(3): [leaf(1), leaf(4)]\n\
5. input(1): [leaf(1), branch(6)]\n\
6. input(3): [leaf(1), leaf(4)]" );

    deleteBmCode(code);
    deleteBmTree(tree);
}
END_TEST

START_TEST(test_BmTree_printConditions)
{
    BmCode* domains= BmCode_initialize( newBmCodeBasic(3, 0), 3, 2, 3, 2);
    BmTree* tree= newBmTree( domains, 4);
    deleteBmCode( domains );
    
    /* Tree:
     *  root   :           (2)
     *         :          / | \ 
     *         :         1  2  3
     *         :        /   |   \ 
     *  b1, b2 :      (1)  -1-   (3) 
     *         :     / |          | \ 
     *         :    1  2          1  2
     *         :   /   |          |   \ 
     *         : -3-  -1-        -2-  -4-
     */

    uint root= BmTree_newBranch( tree, 2, 1 );

    uint b1=  BmTree_newBranch( tree, 1, 3 );
    BmTree_branch_option_output( tree, b1, 2, 1 );

    BmTree_branch_option_connect( tree, root, 1, b1 );

    uint b2=  BmTree_newBranch( tree, 3, 2 );
    BmTree_branch_option_output( tree, b2, 2, 4 );

    BmTree_branch_option_connect( tree, root, 3, b2 );

    char buffer[1024]= "";
    ck_assert_str_eq(
        BmTree_print(tree, buffer),
        "input: [2, 3, 2], size: 3\n\
0. input(2): [branch(1), leaf(1), branch(2)]\n\
1. input(1): [leaf(3), leaf(1)]\n\
2. input(3): [leaf(2), leaf(4)]" );

    strcpy(buffer, "");
    BmBench* collection= BmTree_asNewBench( tree );

    strcpy(buffer, "");
    BmBench_printConditions(collection, buffer);
    ck_assert_str_eq(
        buffer,
        "{[0, 2, 0]:1, [1, 1, 0]:3, [2, 1, 0]:1, [0, 3, 1]:2, [0, 3, 2]:4}"
    );

    BmBench_sortOnItem( collection );
    BmBench_print( collection, buffer);
    ck_assert_str_eq(
        buffer,
        "{[0, 2, 0]:1, [0, 3, 1]:2, [0, 3, 2]:4, [1, 1, 0]:3, [2, 1, 0]:1}"
    );
    deleteBmBench(collection);

    strcpy(buffer, "");
    BmTree_printConditions(tree, buffer);
    ck_assert_str_eq(
        buffer,
        "{[0, 2, 0]:1, [1, 1, 0]:3, [2, 1, 0]:1, [0, 3, 1]:2, [0, 3, 2]:4}"
    );

    strcpy(buffer, "");
    BmCode* code= newBmCode(3, 0, 3, 1);
    ck_assert_uint_eq( BmTree_at( tree, code ), 2 ); 

    deleteBmTree( tree );
}
END_TEST

START_TEST(test_BmTree_deadBranch)
{
}
END_TEST

START_TEST(test_BmTree_optimization)
{
}
END_TEST

/*
 *       Test case scenario
 ***********************************************************************************/
TCase * test_case_BmTree(void)
{
    TCase *tc= tcase_create("BmTree");

    tcase_add_test(tc, test_BmTree_init);
    tcase_add_test(tc, test_BmTree_treeConstruction);
    tcase_add_test(tc, test_BmTree_optionSelection);
    tcase_add_test(tc, test_BmTree_constructionFromExemple);
    tcase_add_test(tc, test_BmTree_printConditions);
    tcase_add_test(tc, test_BmTree_deadBranch);
    tcase_add_test(tc, test_BmTree_optimization);

    return tc;
}
