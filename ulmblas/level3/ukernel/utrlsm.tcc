#ifndef ULMBLAS_LEVEL3_UKERNEL_UTRLSM_TCC
#define ULMBLAS_LEVEL3_UKERNEL_UTRLSM_TCC 1

#include <ulmblas/auxiliary/printmatrix.h>

#include <ulmblas/level3/ukernel/utrlsm.h>
#include <ulmblas/level1extensions/gecopy.h>
#include <ulmblas/level1/scal.h>

namespace ulmBLAS {

//
//  Buffered variant.  Used for zero padded panels.
//
template <typename IndexType, typename T, typename TC>
void
utrlsm(IndexType    mr,
       IndexType    nr,
       const T      *A,
       const T      *B,
       TC           *C,
       IndexType    incRowC,
       IndexType    incColC)
{
    const IndexType MR = ugemm_mr<T>();
    const IndexType NR = ugemm_nr<T>();

    T   A_[MR*MR];
    T   B_[MR*NR];
    T   C_[MR*NR];

    scal(MR*MR, T(0), A_, IndexType(1));
    scal(MR*NR, T(0), B_, IndexType(1));

    gecopy(mr, mr, A, IndexType(1), MR, A_, IndexType(1), MR);
    gecopy(mr, nr, B, NR, IndexType(1), B_, NR, IndexType(1));

    utrlsm(A_, B_, C_, IndexType(1), MR);
    gecopy(mr, nr, C_, IndexType(1), MR, C, incRowC, incColC);
}

//
//  Buffered variant.  Used if the result A^(-1)*B needs to be upcasted for
//  computing C <- A^(-1)*B
//
template <typename T, typename TC, typename IndexType>
void
utrlsm(const T     *A,
       const T     *B,
       TC          *C,
       IndexType   incRowC,
       IndexType   incColC)
{
    const IndexType MR = ugemm_mr<T>();
    const IndexType NR = ugemm_nr<T>();

    utrlsm(MR, NR, A, B, C, incRowC, incColC);
}

//
//  Unbuffered variant.
//
template <typename IndexType, typename T>
void
utrlsm(const T     *A,
       const T     *B,
       T           *C,
       IndexType   incRowC,
       IndexType   incColC)
{
    const IndexType MR = ugemm_mr<T>();
    const IndexType NR = ugemm_nr<T>();

    T   C_[MR*NR];

    for (IndexType i=0; i<MR; ++i) {
        for (IndexType j=0; j<NR; ++j) {
            C_[i+j*MR] = B[i*NR+j];
        }
    }

    for (IndexType i=0; i<MR; ++i) {
        for (IndexType j=0; j<NR; ++j) {
            C_[i+j*MR] *= A[i];
            for (IndexType l=i+1; l<MR; ++l) {
                C_[l+j*MR] -= A[l]*C_[i+j*MR];
            }
        }
        A += MR;
    }

    gecopy(MR, NR, C_, IndexType(1), MR, C, incRowC, incColC);
}

} // namespace ulmBLAS

#endif // ULMBLAS_LEVEL3_UKERNEL_UTRLSM_TCC