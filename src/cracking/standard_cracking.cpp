#include "standard_cracking.h"
#include "stdio.h"

int PIVOT_RANDOM_WITHIN_PREDICATE_PIECE,PIVOT_APPROX_MEDIAN_WITHIN_PREDICATE_PIECE,PIVOT_MEDIAN_WITHIN_PREDICATE_PIECE;
int PIVOT_TYPE;

int64_t crackInTwoItemWise(IndexEntry*& c, int64_t posL, int64_t posH,  int64_t low, int64_t high, IndexEntry*& view, int64_t& view_size, int64_t pivot){
    view = (IndexEntry*)malloc((posH-posL+1)*sizeof(IndexEntry));
    int64_t size = 0;
    while(posL <= posH){
        while(posL<=posH && c[posL]<pivot){
            if(c[posL]>=low && c[posL]<high)
                view[size++] = c[posL];
            posL++;
        }
        while(posL<=posH && c[posH]>=pivot){
            if(c[posH]>=low && c[posH]<high)
                view[size++] = c[posH];
            posH--;
        }
        if(posL<posH)
            exchange(c, posL, posH);
    }
    view_size = size;
    // Return Offset
    return posL-1;
}

AvlTree standardCrackingWithinPiece(IndexEntry *&c, int64_t dataSize, AvlTree T, int64_t lowKey, int64_t highKey, QueryOutput *qo){
    IntPair p1,p2;

    p1 = FindNeighborsLT(lowKey, T, dataSize-1);
    p2 = FindNeighborsLT(highKey, T, dataSize-1);
    int64_t pivot_1 = 0, pivot_2 = 0, offset_1 = 0,offset_2 = 0;

    if(p1->first==p2->first && p1->second==p2->second){
        if (PIVOT_TYPE == PIVOT_RANDOM_WITHIN_PREDICATE_PIECE){
            pivot_1 = (rand() % (p1->second-p1->first)) + p1->first;
            pivot_1 = c[pivot_1].m_key;
        }
        else if (PIVOT_TYPE == PIVOT_APPROX_MEDIAN_WITHIN_PREDICATE_PIECE){
            pivot_1 =  c[(p1->first+p1->second)/2].m_key;
        }
        else if (PIVOT_TYPE == PIVOT_MEDIAN_WITHIN_PREDICATE_PIECE){
            pivot_1 =  find_median(c,  p1->first,  p1->second);
        }
        offset_1 = crackInTwoItemWise(c, p1->first, p1->second, lowKey, highKey,qo->view1, qo->view_size1,pivot_1);
        T = Insert(offset_1, pivot_1, T);
    }
    else{
        if (PIVOT_TYPE == PIVOT_RANDOM_WITHIN_PREDICATE_PIECE){
            if (p1->second !=p1->first)
                pivot_1 = (rand() %  (p1->second-p1->first)) + p1->first;
            else
                pivot_1 = p1->first;
            if (p2->second !=p2->first)
                pivot_2 = (rand() %  (p2->second-p2->first)) + p2->first;
            else
                pivot_2 = p1->first;
            pivot_1 = c[pivot_1].m_key;
            pivot_2 = c[pivot_2].m_key;
        }
        else if (PIVOT_TYPE == PIVOT_APPROX_MEDIAN_WITHIN_PREDICATE_PIECE){
            pivot_1 = c[(p1->first+p1->second)/2].m_key;
            pivot_2 = c[(p2->first+p2->second)/2].m_key;
        }
        else if (PIVOT_TYPE == PIVOT_MEDIAN_WITHIN_PREDICATE_PIECE){
            pivot_1 =  find_median(c,  p1->first,  p1->second);
            pivot_2 =  find_median(c,  p2->first,  p2->second);
        }
        offset_1 = crackInTwoItemWise(c, p1->first, p1->second, lowKey, highKey,qo->view1, qo->view_size1,pivot_1);
        qo->middlePart = &c[p1->second+1];
        int size2 = p2->first-p1->second-1;
        qo->middlePart_size = size2;
        offset_2= crackInTwoItemWise(c, p2->first, p2->second,  lowKey, highKey,qo->view2, qo->view_size2,pivot_2);
        T = Insert(offset_1, pivot_1, T);
        T = Insert(offset_2, pivot_2, T);
    }
    free(p1);
    free(p2);
    return T;
}

int crackInTwoItemWise(IndexEntry*& c, int64_t posL, int64_t posH, int64_t med){
    int x1 = posL, x2 = posH;
    while (x1 <= x2) {
        if(c[x1] < med)
            x1++;
        else {
            while (x2 >= x1 && (c[x2] >= med))
                x2--;
            if(x1 < x2){
                exchange(c, x1,x2);
                x1++;
                x2--;
            }
        }
    }
    if(x1 < x2)
        printf("Not all elements were inspected!");
    x1--;
    if(x1 < 0)
        x1 = 0;
    return x1;
}

AvlTree standardCracking(IndexEntry*& c, int dataSize, AvlTree T, int lowKey, int highKey){
    IntPair p1;
    IntPair pivot_pair = NULL ;
    p1 = FindNeighborsLT(lowKey, T, dataSize-1);
    pivot_pair = (IntPair) malloc(sizeof(struct int_pair));
    pivot_pair->first = crackInTwoItemWise(c, p1->first, p1->second, lowKey);
    T = Insert(pivot_pair->first, lowKey, T);
    p1 = FindNeighborsLT(highKey, T, dataSize-1);
    pivot_pair = (IntPair) malloc(sizeof(struct int_pair));
    pivot_pair->first = crackInTwoItemWise(c, p1->first, p1->second, lowKey);
    T = Insert(pivot_pair->first, lowKey, T);

    free(p1);
    if(pivot_pair) {
        free(pivot_pair);
        pivot_pair = NULL;
    }
    return T;
}
