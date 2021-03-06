/**
 * Copyright (C) 2014 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for licence.
 *
 */

#include "dispatch.hh"
#include "runners.hh"
#include "expression.hh"
#include "iss.hh"
#include "terminal.hh"
#include "uncopyable.hh"
#include "lapack.hh"
#include "debug.h"

using namespace std;

/**
 *  Unit contains code for LU node runners
 */
namespace librdag {

template<typename T> void lu_dense_runner(RegContainer& reg, shared_ptr<const OGMatrix<T>> arg)
{
  int4 m = arg->getRows();
  int4 n = arg->getCols();
  int4 lda = m;
  int4 minmn = m > n ? n : m;
  int4 mn = m * n;
  int4 info = 0;

// Sizes of output
//   L = [m x minmn ]
//   U = [minmn x n]

  unique_ptr<T[]> Lptr(new T[m*minmn]());
  unique_ptr<T[]> Uptr(new T[minmn*n]());
  unique_ptr<T[]> Aptr(new T[mn]());
  unique_ptr<int4[]> ipivptr(new int4[minmn]());

  T * L = Lptr.get();
  T * U = Uptr.get();

  // copy A else it's destroyed
  T * A = Aptr.get();
  std::memcpy(A, arg->getData(), sizeof(T)*mn);

  // create pivot vector
  int4 * ipiv = ipivptr.get();

  // call lapack
  try
  {
    lapack::xgetrf<T, lapack::OnInputCheck::isfinite>(&m, &n, A, &lda, ipiv, &info);
  }
  catch (rdag_recoverable_error& e)
  {
    // failed as system is singular TODO: this will end up in logs (MAT-369) and userland (MAT-370).
    cerr << "Warning: singular system detected in matrix decomposition." << std::endl;
    cerr << "---> LAPACK details: " << e.what() << std::endl;
  }
  // Else, exception propagates, stack unwinds

  // The following is adapted from DOGMAv1

  // extract U, get triangle, then square
  // U strides in 'minmn', A strides in 'm'
  int4 lim = minmn > n ? n : minmn;
  for (int4 i = 0; i < lim - 1; i++)
  {
    int4 mi = m * i;
    int4 ni = minmn * i;
    for (int4 j = 0; j <= i; j++)
    {
      U[ni + j] = A[mi + j];
    }
  }
  for (int4 i = lim - 1; i < n; i++)
  {
    int4 mi = m * i;
    int4 ni = minmn * i;
    for (int4 j = 0; j < minmn; j++)
    {
      U[ni + j] = A[mi + j];
    }
  }

  // Transpose the pivot... create as permutation
  unique_ptr<int4[]> permptr (new int4[m]);
  int4 * perm = permptr.get();
  // 1) turn into 0 based indexing
  for (int4 i = 0; i < minmn; i++)
  {
    ipiv[i] -= 1;
  }
  // 2) 0:m-1 range vector, will be permuted in a tick
  for (int4 i = 0; i < m; i++)
  {
    perm[i] = i;
  }
  // 3) apply permutation to range indexed vector, just walk through in order and apply the swaps
  int4 swp;
  for (int4 i = 0; i < minmn; i++)
  {
    int4 piv = ipiv[i]; // get pivot at index "i"
    // apply the pivot by swapping the corresponding "row" indices in the perm index vector
    if (piv != i)
    {
      swp = perm[piv];
      perm[piv] = perm[i];
      perm[i] = swp;
    }
  }

  // kill triu of A, write 1 onto diag of A too
  A[0] = 1.e0;
  for (int4 i = 1; i < minmn; i++)
  {
    A[m*i+i] = 1.e0;
    for (int4 j = 0; j < i; j++)
    {
      A[i*m+j]=0.e0;
    }
  }

  // apply pivot during assign to L
  for (int4 i = 0; i < m; i++)
  {
    int4 permi = perm[i];
    int4 row = perm[permi];
    if(row==i)
    {
      for (int4 j = 0; j < minmn; j++)
      {
        int4 jm = j*m;
        L[jm+i] = A[jm+permi];
      }
    }
    else
    {
      for (int4 j = 0; j < minmn; j++)
      {
        int4 jm = j*m;
        L[jm+i] = A[jm+row];
      }
    }
  }

  OGNumeric::Ptr cL = makeConcreteDenseMatrix(Lptr.release(), m, minmn, OWNER);
  OGNumeric::Ptr cU = makeConcreteDenseMatrix(Uptr.release(), minmn, n, OWNER);

  reg.push_back(cL);
  reg.push_back(cU);
}

void *
LURunner::run(RegContainer& reg, OGRealScalar::Ptr arg) const
{
  // single real space lu is just l=1, u=value
  reg.push_back(OGRealScalar::create(1.e0));
  if(arg->getValue()==0.e0)
  {
    cerr << "Warning: singular system detected in LU decomposition" << std::endl;
  }
  reg.push_back(OGRealScalar::create(arg->getValue()));
  return nullptr;
}
void *
LURunner::run(RegContainer& reg, OGRealDenseMatrix::Ptr arg) const
{
  lu_dense_runner<real8>(reg, arg);
  return nullptr;
}

void *
LURunner::run(RegContainer& reg, OGComplexDenseMatrix::Ptr arg) const
{
  lu_dense_runner<complex16>(reg, arg);
  return nullptr;
}

} // end namespace
