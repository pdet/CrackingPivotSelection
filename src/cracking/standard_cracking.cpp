#include "standard_cracking.h"
#include "stdio.h"

int64_t crackPieceMiddleQuery(IndexEntry*& c, int64_t posL, int64_t posH, int64_t* sum, int64_t pivot){
    while(posL <= posH){
        while(posL<=posH && c[posL]<pivot){
            *sum += c[posL].m_key;
            posL++;
        }
        while(posL<=posH && c[posH]>=pivot){
            *sum += c[posH].m_key;
            posH--;
        }
        if(posL<posH)
            exchange(c, posL, posH);
    }
    return posL-1;
}

int64_t crackPieceWithBothQueryPredicate(IndexEntry*& c, int64_t posL, int64_t posH,  int64_t low, int64_t high, int64_t* sum, int64_t pivot){
    while(posL <= posH){
        while(posL<=posH && c[posL]<pivot){
            if(c[posL]>=low && c[posL]<high)
                *sum += c[posL].m_key;
            posL++;
        }
        while(posL<=posH && c[posH]>=pivot){
            if(c[posH]>=low && c[posH]<high)
                *sum += c[posH].m_key;
            posH--;
        }
        if(posL<posH){
            exchange(c, posL, posH);
        }
    }
    return posL-1;
}

int64_t crackPieceWithLeftPredicate(IndexEntry*& c, int64_t posL, int64_t posH,  int64_t low, int64_t* sum, int64_t pivot){
    while(posL <= posH){
        while(posL<=posH && c[posL]<pivot){
            if(c[posL]>=low)
                *sum += c[posL].m_key;
            posL++;
        }
        while(posL<=posH && c[posH]>=pivot){
            if(c[posH]>=low)
                *sum += c[posH].m_key;
            posH--;
        }
        if(posL<posH){
            exchange(c, posL, posH);
        }
    }
    return posL-1;
}

int64_t crackPieceWithRightPredicate(IndexEntry*& c, int64_t posL, int64_t posH, int64_t high, int64_t* sum, int64_t pivot){
    while(posL <= posH){
        while(posL<=posH && c[posL]<pivot){
            if(c[posL]<high)
                *sum += c[posL].m_key;
            posL++;
        }
        while(posL<=posH && c[posH]>=pivot){
            if(c[posH]<high)
                *sum += c[posH].m_key;
            posH--;
        }
        if(posL<posH){
            exchange(c, posL, posH);
        }
    }
    return posL-1;
}

int64_t crackPieceOutsideQuery(IndexEntry*& c, int64_t posL, int64_t posH, int64_t pivot){
    while (posL <= posH) {
        if(c[posL] < pivot)
            posL++;
        else {
            while (posH >= posL && (c[posH] >= pivot))
                posH--;
            if(posL < posH){
                exchange(c, posL,posH);
                posH++;
                posL--;
            }
        }
    }
    posL--;
    if(posL < 0)
        posL = 0;
    return posL;
}

int64_t scan_middle_pieces (IndexEntry*& c, int64_t posL, int64_t posH){
    int64_t sum =0;
    for (size_t i = posL; i < posH; ++i)
        sum+=c[i].m_key;
    return sum;
}

int64_t scan_left_piece (IndexEntry*& c, int64_t posL, int64_t posH, int64_t low){
    int64_t sum =0;
    for (size_t i = posL; i < posH; ++i)
        if(c[i]>=low)
            sum+=c[i].m_key;
    return sum;
}

int64_t scan_right_piece (IndexEntry*& c, int64_t posL, int64_t posH, int64_t high){
    int64_t sum =0;
    for (size_t i = posL; i <= posH; ++i)
        if(c[i]<high)
            sum+=c[i].m_key;
    return sum;
}

int64_t scan_left_right_piece (IndexEntry*& c, int64_t posL, int64_t posH, int64_t low, int64_t high){
    int64_t sum =0;
    for (size_t i = posL; i <= posH; ++i)
        if(c[i]>=low && c[i]<high)
            sum+=c[i].m_key;
    return sum;
}
