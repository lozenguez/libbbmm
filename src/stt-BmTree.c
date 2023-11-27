
#include "bbmm-structures.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- *
 *   B b M m   S E L E C T O R                                           *
 * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- *
 *
 * BmTree is built as an array of branches.
 * a branch is an array of positive interger (uint) modeling an option value ( in [1, optionSize], 0 is reserved for no ouput ) or the next branch to look at with the next state variable.
 * - option=  branch[i] if  branch[i] <= self->optionSize 
 * - nextBranchId= branch[i] - (self->optionSize+1) else
 */

/* Private Methods */

uint _BmTree_at_readOrder_set_fromBranch( BmTree* self, BmCode* code, BmCode* codeOrder, uint output, uint iBranch ); // cf. BmTree_at_set, but starting from a given branch.


/* Constructor Destructor */
BmTree* newBmTree( uint binarySpaceSize, uint optionSize )
{
    return BmTree_createWhith( newEmpty(BmTree),
            newBmCode_all(binarySpaceSize, 2), optionSize );
}

BmTree* newBmTreeWith( BmCode* newSpace, uint optionSize )
{
    return BmTree_createWhith( newEmpty(BmTree), newSpace, optionSize );
}

void deleteBmTree(BmTree* self)
{
    BmTree_destroy(self);
    free(self);
}

void BmTree_delete(BmTree* self)
{
    for( uint i= 0 ; i <=  self->size ; ++i )
        free( self->branches[i] );
    free( self->branches );
    free(self);
}

/* Protected - to use with precaution */
BmTree* BmTree_createWhith( BmTree* self, BmCode* newSpace, uint optionSize )
{
    self->space= newSpace;
    self->optionBound= optionSize+1;
    self->capacity= 1;
    self->branches= malloc( sizeof(int*) * self->capacity );
    self->size= 0;

    self->optionTags= newEmptyArray( uint, self->optionBound );
    self->optionValues= newEmptyArray( double, self->optionBound );

    return self;
}

BmTree* BmTree_destroy(BmTree* self)
{
    for( uint i= 0 ; i <  self->size ; ++i )
        free( self->branches[i] );
    free( self->branches );

    deleteEmptyArray( self->optionTags );
    deleteEmptyArray( self->optionValues );

    return self;
}

/* Re-Initialize */
BmTree* BmTree_reinitWhith_on(BmTree* self, uint index, int defaultOption)
{
    // free all branches:
    while( self->size > 0 )
    {
        self->size-= 1;
        free( self->branches[ self->size ] );
    }
    BmTree_newBranch( self, index, defaultOption );

    return self;
}

BmTree* BmTree_reinitOn(BmTree* self, int defaultOption)
{
    return BmTree_reinitWhith_on( self, 1, defaultOption );
}

/* Construction */
void BmTree_reziseCapacity( BmTree* self, uint newCapacity)
{
    // free exedent branches:
    while( self->size > newCapacity )
    {
        self->size-= 1;
        free( self->branches[ self->size ] );
    }

    // generate the new structure :
    uint ** newBranches= malloc( sizeof(int*) * newCapacity );

    // copy the remainding branches :
    for( uint i = 0 ; i < self->size ; ++i )
        newBranches[i]= self->branches[i];
    
    // end flip :
    free( self->branches );
    self->branches= newBranches;
    self->capacity= newCapacity;
}

void BmTree_reziseCompleteCapacity( BmTree* self)
{
    BmTree_reziseCapacity( self, BmCode_product( self->space ) );
}

void BmTree_option_set( BmTree* self, uint iOption, uint tag, double value )
{
    array_at_set( self->optionTags, iOption, tag );
    array_at_set( self->optionValues, iOption, value );
}

uint BmTree_newBranch(BmTree* self, uint iVariable, int defaultOption)
{
    int branch= self->size;
    uint bound= BmCode_at(self->space, iVariable);
    self->size+= 1;

    // Rezise the capacity
    if( self->size > self->capacity )
        BmTree_reziseCapacity( self, self->size );
    
    // Create the line:
    self->branches[branch]= malloc( sizeof(uint*)*(bound+1) );

    // Set to default:
    self->branches[branch][0]= iVariable;
    for( uint i = 1 ; i <= bound ; ++i )
        self->branches[branch][i]= defaultOption;
    
    return branch;
}

void BmTree_branch_state_connect( BmTree* self, uint branchA, uint stateA, uint branchB )
{
    self->branches[branchA][stateA]= self->optionBound+branchB;
}

void BmTree_branch_state_set( BmTree* self, uint iBranch, uint iState, uint output )
{
    self->branches[iBranch][iState]= output;
}

uint _BmTree_at_readOrder_set_fromBranch( BmTree* self, BmCode* code, BmCode* codeOrder, uint output, uint iBranch )
{
    assert( BmCode_dimention(code) == BmCode_dimention(self->space) );
    assert( iBranch < self->size );
    
    uint iVar= BmTree_branchVariable(self, iBranch);

    // Find the next significant variables (if exist)
    uint nextVariable= 0;
    for( uint i = 1 ; nextVariable == 0 && i <= BmCode_dimention(codeOrder) ; ++i )
    {
        uint iCode= BmCode_at( codeOrder, i);
        if( iCode != 0 && iCode != iVar && BmCode_at(code, iCode) != 0 )
            nextVariable= iCode;
    }

    if( BmCode_at( code, iVar) == 0 )// the ouput do not depend on this variable
    {
        assert( nextVariable != 0 ); // It must have another variable.
        // Keep going, what ever the value of this variable...
        uint count= 0;
        for( uint option= 1 ; option <= BmCode_at(self->space, iVar) ; ++option )
        {
            BmCode * codeBis= newBmCodeAs( code );
            BmCode_at_set( codeBis, iVar, option );
            count+= _BmTree_at_readOrder_set_fromBranch( self, codeBis, codeOrder, output, iBranch );
            deleteBmCode(codeBis);
        }
        return count;
    }

    // Count the number of significant variables.
    uint branchOutput= BmTree_branch_state( self, iBranch, BmCode_at(code, iVar) );
    if( nextVariable == 0 )// Only one reminding variable define the output on the branch: iBranch
    {
        BmTree_branch_state_set( self, iBranch, BmCode_at(code, iVar), output );
        if( branchOutput >= self->optionBound )
            return 1;
        return 0;
    }
    else
    {
        if( branchOutput < self->optionBound )
        {// The branch output is a leaf, we have to create a new branch on the next variable.
            uint newBranchId= BmTree_newBranch(self, nextVariable, branchOutput );
            BmTree_branch_state_connect( self, iBranch, BmCode_at(code, iVar), newBranchId );

            //recursive call:
            BmCode_at_set( code, iVar, 0 );// set the variable visited
            return _BmTree_at_readOrder_set_fromBranch( self, code, codeOrder, output, newBranchId );
        }
        else
        {
            //recursive call:
            BmCode_at_set( code, iVar, 0 );// set the variable visited
            return _BmTree_at_readOrder_set_fromBranch( self, code, codeOrder, output, branchOutput - self->optionBound );
        }   
    }
}

uint BmTree_at_set( BmTree* self, BmCode* code, uint output)
{
    // Generate a basic code order:
    uint iOrder= 1;
    BmCode* order= newBmCode_all( BmCode_dimention(code), 0 );
    for( uint i= 1 ; i <= BmCode_dimention( code ) ; ++i  )
    {
        if( BmCode_at(code, i) > 0 )
        {
            BmCode_at_set(order, iOrder, i);
            ++iOrder;
        }
    }
    return BmTree_at_readOrder_set( self, code, order, output );
}

uint BmTree_at_readOrder_set( BmTree* self, BmCode* code, BmCode* codeOrder, uint output )
{
    // If not initialized yet: 
    if( self->size == 0 )
    {
        BmTree_reinitWhith_on(self, BmCode_at( codeOrder, 1 ), 1);
    }

    // Then apply the code at output: 
    BmCode * draft= newBmCodeAs(code);
    uint r= _BmTree_at_readOrder_set_fromBranch( self, draft, codeOrder, output, 0);
    deleteBmCode(draft);
    return r;
}

/* Cleanning */
uint BmTree_cleanDeadBranches( BmTree* self)
{
    printf( "BmTree_cleanDeadBranches %d\n", self->size );
    assert(false); // TODO
    return 0;
}

uint BmTree_removeBranch(BmTree* self, uint iBranch)
{
    printf( "BmTree_cleanDeadBranches %d, %d\n", self->size, iBranch );
    assert(false); // TODO
    return 0;
}


/* Getter */
uint BmTree_branch_state( BmTree* self, uint iBranch, uint state )
{
    return self->branches[iBranch][state];
}

uint BmTree_branchVariable( BmTree* self, uint iBranch )
{
    return self->branches[iBranch][0];
}

uint BmTree_branchSize( BmTree* self, uint branch )
{
    uint iVariable= BmTree_branchVariable(self, branch);
    uint bound= BmCode_at(self->space, iVariable);
    uint count= 1;
    uint output[bound+1];
    output[0]= self->branches[branch][1];

    //For each possible output
    for( uint i= 2 ; i <= bound ; ++i )
    {
        //search if it is a new output
        uint new= 1;
        for( uint r = 0 ; new && r < count ; ++r )
            new= output[r] != self->branches[branch][i];
        
        //If yes, mark it and increase the counter.
        if( new )
        {
            output[count]= self->branches[branch][i];
            count+= 1;
        }
    }
    return count; //So, return the number of possible output on the branch...
}

uint BmTree_at( BmTree* self, BmCode* code)
{
    uint option= self->optionBound;// i.e. branch = 0
    uint deep= 1;

    if( self->size == 0 )
    {
        return 1;
    }
    while( option >= self->optionBound && deep <= BmCode_dimention(self->space) )
    {
        uint branch= option-self->optionBound;
        uint iVar= BmTree_branchVariable(self, branch);
        option= self->branches[ branch ][ BmCode_at(code, iVar) ];
        ++deep;
    }
    return option;
}

uint BmTree_tagAt( BmTree* self, BmCode* code)
{
    return array_at( self->optionTags, BmTree_at( self, code ) );
}

double BmTree_valueAt( BmTree* self, BmCode* code)
{
    assert( BmCode_isIncluding( self->space, code ) );
    return array_at( self->optionValues, BmTree_at( self, code ) );
}

uint BmTree_deepOf( BmTree* self, BmCode* code )
{
    if( self->size == 0 )
    {
        return 0;
    }
    
    assert( BmCode_dimention(code) == self->size );
    assert( BmCode_count( code, 0 ) == 0 );

    uint option= self->optionBound;// i.e. branch = 0
    uint deep= 1;

    while( option >= self->optionBound && deep <= BmCode_dimention(self->space) )
    {
        uint branch= option-self->optionBound;
        uint iVar= BmTree_branchVariable(self, branch);
        option= self->branches[ branch ][ BmCode_at(code, iVar) ];
        ++deep;
    }
    return deep;
}

/* Generating */
BmBench* BmTree_asNewBench( BmTree* self )
{
    BmBench* bench= newBmBench( self->size*2 );
    uint codeSize= BmCode_dimention(self->space)+1;

    // Process the tree:
    BmCode *conditions[self->size];
    for( uint iBranch= 0; iBranch < self->size; ++iBranch )
    {
        conditions[iBranch]= newBmCode_all( codeSize, 0);
    }
    for( uint iBranch= 0 ; iBranch < self->size ; ++iBranch )
    {
        uint branVar= BmTree_branchVariable(self, iBranch);
        uint bound= BmCode_at(self->space, branVar);
        for( uint i= 1 ; i <= bound ; ++i )
        {
            uint output= self->branches[iBranch][i];
            if( output < self->optionBound )
            {
                BmCode_at_set( conditions[iBranch], branVar, i );
                BmCode_at_set( conditions[iBranch], codeSize, output );
                BmBench_attachLast(
                    bench, newBmCodeAs( conditions[iBranch] ), array_at(self->optionValues, output) );
            }
            else
            {
                uint nextBranch= output - self->optionBound;
                BmCode_at_set( conditions[nextBranch], branVar, i );
            }
        }
    }
    for( uint i= 0; i < self->size; ++i )
    {
        deleteBmCode( conditions[i] );
    }

    // Empty tree:
    if( BmBench_size(bench) == 0 )
    {
        BmCode* zeroConf= newBmCode_all( codeSize, 0 );
        BmCode_at_set( zeroConf, codeSize, 1 );
        BmBench_attachLast( bench, zeroConf, array_at(self->optionValues, 1) );
    }

    BmBench_sort( bench, (fctptr_BmBench_compare)BmBench_isGreater );
    return bench;
}

/* Printing */
char* BmTree_printBranch( BmTree* self, uint iBranch, char* output )
{
    uint iVariable= BmTree_branchVariable(self, iBranch);
    
    strcat(output, "[");
    char sep[8]= "";
    char buffer[64];
    uint bound= BmCode_at(self->space, iVariable);
    for( uint i= 1 ; i <= bound ; ++i )
    {
        if( self->branches[iBranch][i] < self->optionBound )
            sprintf( buffer, "%sleaf(%u)", sep, self->branches[iBranch][i] );
        else
            sprintf( buffer, "%sbranch(%u)", sep, self->branches[iBranch][i]-self->optionBound  );
        strcat(output, buffer);
        strcpy(sep, ", ");
    }
    strcat(output, "]");
    return output;
}

char* BmTree_print(BmTree* self, char* output)
{
    return BmTree_print_sep( self, output, ", " );
}

char* BmTree_print_sep( BmTree* self, char* output, char* separator )
{
    // prepare options
    char** optionsStr= newEmptyArray( char*, self->optionBound );
    for( uint i= 1 ; i <= self->optionBound ; ++i )
    {
        array_at_set( optionsStr, i, newEmptyArray( char*, 32 ); )
        sprintf( array_at( optionsStr, i ), "%u", i );
    }

    // print
    BmTree_print_sep_options(self, output, separator, optionsStr);
    
    // clear
    for( uint i= 1 ; i <= self->optionBound ; ++i )
        deleteEmptyArray( array_at(optionsStr, i) );
    deleteEmptyArray( optionsStr );
    return output;
}

char* BmTree_print_sep_options( BmTree* self, char* output, char* separator, char** optionStrs )
{
    char buffer[512];

    BmCode *conditions[self->size];
    for( uint iBranch= 0; iBranch < self->size; ++iBranch )
    {
        conditions[iBranch]= newBmCode_all( BmCode_dimention(self->space), 0 );
    }

    strcat(output, "{");
    char sep[16]= "";
    for( uint iBranch= 0 ; iBranch < self->size ; ++iBranch )
    {
        uint branVar= BmTree_branchVariable(self, iBranch);
        uint bound= BmCode_at(self->space, branVar);
        for( uint i= 1 ; i <= bound ; ++i )
        {
            if( self->branches[iBranch][i] < self->optionBound )
            {
                BmCode_at_set( conditions[iBranch], branVar, i );
                char codeBuf[256]= "";
                sprintf( buffer, "%s%s:%s", sep, BmCode_print(conditions[iBranch], codeBuf), array_at( optionStrs, self->branches[iBranch][i] ) );
                strcat(output, buffer);
                strcpy(sep, separator);
            }
            else
            {
                uint nextBranch= self->branches[iBranch][i]-self->optionBound;
                BmCode_at_set( conditions[nextBranch], branVar, i );
            }
        }
    }
    strcat(output, "}");

    for( uint i= 0; i < self->size; ++i )
    {
        deleteBmCode( conditions[i] );
    }

    return output;
}


char* BmTree_printInside(BmTree* self, char* output)
{
    char buffer[512];
        
    strcat(output, "input: ");
    BmCode_print( self->space, output );
    sprintf( buffer, ", size: %u", self->size );
    strcat(output, buffer );

    for( uint ib= 0 ; ib < self->size ; ++ib )
    {
        char buffer2[128]= "";
        sprintf( buffer, "\n%u. input(%u): %s", ib, BmTree_branchVariable(self, ib),
            BmTree_printBranch( self, ib, buffer2)
        );
        strcat( output, buffer );
    }
    return output;
}
