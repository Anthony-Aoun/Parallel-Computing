#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace
{
void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                   const Matrix &A, const Matrix &B, Matrix &C)
{
  int i, j, k;
//#pragma omp parallel for private(i, j, k) shared(A, B, C)
#pragma omp parallel for schedule(dynamic)
  for (int j = iColBlkB; j < std::min(B.nbCols, iColBlkB + szBlock); j++)
  {
    for (int k = iColBlkA; k < std::min(A.nbCols, iColBlkA + szBlock); k++)
    {
      for (int i = iRowBlkA; i < std::min(A.nbRows, iRowBlkA + szBlock); ++i)
      {
        C(i, j) += A(i, k) * B(k, j);
      }
    }
  }
}
const int szBlock = 32;
} // namespace

Matrix operator*(const Matrix &A, const Matrix &B)
{
  Matrix C(A.nbRows, B.nbCols, 0.0);
  prodSubBlocks(0, 0, 0, std::max({A.nbRows, B.nbCols, A.nbCols}), A, B, C);
  return C;
}

// j, k, i = 0.398414
// k, j, i = 0.520433
// i, j, k = 2.84505
// j, i, k = 6.7259
// i, k, j = 21.1392
// k, i, j = 21.4082

// 1 thread = 0.148237
// 2 thread = 0.147634
// 3 thread = 0.155006
// 4 thread = 0.147434
// 5 thread = 0.155684
// 6 thread = 0.15619
// 7 thread = 0.155342
// 8 thread = 0.131194