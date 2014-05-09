/**
 * Copyright (C) 2014 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for license.
 */


#include "gtest/gtest.h"
#include "terminal.hh"
#include "execution.hh"
#include "dispatch.hh"
#include "testnodes.hh"

using namespace std;
using namespace librdag;
using namespace testnodes;
using ::testing::TestWithParam;
using ::testing::Values;

/*
 * Check MTIMES node behaves
 */

BINARY_NODE_TEST_SETUP(MTIMES)

INSTANTIATE_NODE_TEST_CASE_P(MTIMESTests,MTIMES,
  Values
  (
  // scalar * scalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealScalar(2.0)}, pOGNumeric{new OGRealScalar(3.0)}, pOGNumeric{new OGRealScalar(6.0)},MATHSEQUAL),
  // scalar * matrix
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealMatrix(new real16[1]{2},1,1,OWNER)}, pOGNumeric{new OGRealMatrix(new real16[2]{10,20},2,1,OWNER)},pOGNumeric{new OGRealMatrix(new real16[2]{20,40},2,1,OWNER)}, MATHSEQUAL),
   // matrix * scalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealMatrix(new real16[2]{10,20},2,1,OWNER)}, pOGNumeric{new OGRealMatrix(new real16[1]{2},1,1,OWNER)}, pOGNumeric{new OGRealMatrix(new real16[2]{20,40},2,1,OWNER)}, MATHSEQUAL),
  // matrix * vector
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealMatrix(new real16[6]{1,3,5,2,4,6},3,2,OWNER)}, pOGNumeric{new OGRealMatrix(new real16[2]{10,20},2,1,OWNER)},pOGNumeric{new OGRealMatrix(new real16[3]{50,110,170},3,1,OWNER)}, MATHSEQUAL),
  // matrix * matrix
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealMatrix(new real16[6]{1,3,5,2,4,6},3,2,OWNER)}, pOGNumeric{new OGRealMatrix(new real16[4]{10,30,20,40},2,2,OWNER)},pOGNumeric{new OGRealMatrix(new real16[6]{70,150,230,100,220,340},3,2,OWNER)}, MATHSEQUAL),
   // rscalar * cscalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexScalar({2.0,4.0})}, pOGNumeric{new OGRealScalar(10)}, pOGNumeric{new OGComplexScalar({20,40})},MATHSEQUAL),
   // cscalar * rscalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealScalar(10)}, pOGNumeric{new OGComplexScalar({2.0,4.0})}, pOGNumeric{new OGComplexScalar({20,40})},MATHSEQUAL),
   // cscalar * cscalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexScalar({2.0,4.0})}, pOGNumeric{new OGComplexScalar({3.0,5.e0})}, pOGNumeric{new OGComplexScalar({-14,22})},MATHSEQUAL),
  // rscalar * cmatrix
  new CheckBinary<MTIMES>( pOGNumeric{new OGRealScalar(10)}, pOGNumeric{new OGComplexMatrix(new complex16[2]{{1,10},{2,20}},2,1,OWNER)},pOGNumeric{new OGComplexMatrix(new complex16[2]{{10,100},{20,200}},2,1,OWNER)},MATHSEQUAL),
  // cmatrix * rscalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexMatrix(new complex16[2]{{1,10},{2,20}},2,1,OWNER)}, pOGNumeric{new OGRealScalar(10)}, pOGNumeric{new OGComplexMatrix(new complex16[2]{{10,100},{20,200}},2,1,OWNER)},MATHSEQUAL),
  // cscalar * cmatrix
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexMatrix(new complex16[1]{{2,3}},1,1,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[2]{{1,10},{2,20}},2,1,OWNER)},pOGNumeric{new OGComplexMatrix(new complex16[2]{{-28,23},{-56,46}},2,1,OWNER)},MATHSEQUAL),
  // cmatrix * cscalar
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexMatrix(new complex16[2]{{1,10},{2,20}},2,1,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[1]{{2,3}},1,1,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[2]{{-28,23},{-56,46}},2,1,OWNER)},MATHSEQUAL),
  // cmatrix * cvector
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexMatrix(new complex16[6]{{1.,10.}, {3.,30.}, {5.,50.}, {2.,20.}, {4.,40.}, {6.,60.}},3,2,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[2]{{1,10},{2,20}},2,1,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[3]{{-495.,100.}, {-1089.,220.}, {-1683.,340.}},3,1,OWNER)},MATHSEQUAL),
   // cmatrix * cmatrix
  new CheckBinary<MTIMES>( pOGNumeric{new OGComplexMatrix(new complex16[6]{{1.,10.}, {3.,30.}, {5.,50.}, {2.,20.}, {4.,40.}, {6.,60.}},3,2,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[4] {{1.,10.}, {3.,30.}, {2.,20.}, {5.,50.}},2,2,OWNER)}, pOGNumeric{new OGComplexMatrix(new complex16[6]{{-693.,140.}, {-1485.,300.}, {-2277.,460.}, {-1188.,240.}, {-2574.,520.}, {-3960.,800.}},3,2,OWNER)},MATHSEQUAL)
  )
);


TEST(MTIMESTests, CheckBadCommuteThrows) {
  pOGNumeric m1 = pOGNumeric{new OGRealMatrix(new real16[6]{1,3,5,2,4,6},3,2,OWNER)};
  pOGNumeric m2 = pOGNumeric{new OGRealMatrix(new real16[7]{10,30,20,40,50,60,70},1,7,OWNER)};
  pOGExpr node = pOGExpr{new MTIMES(m1, m2)};
  ExecutionList el = ExecutionList{node};
  Dispatcher v;
  ASSERT_THROW(
  for (auto it = el.begin(); it != el.end(); ++it)
  {
    v.dispatch(*it);
  }
  , rdag_error);
}
