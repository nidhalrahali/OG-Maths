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
#include "equals.hh"
#include "runtree.hh"

using namespace std;
using namespace librdag;
using namespace testnodes;
using ::testing::TestWithParam;
using ::testing::Values;

/*
 * Check LU node behaves correctly
 */

// ok cond 5x4
real16 rcondok5x4[20] =  {5,9,10,8,1,9,17,19,15,2,10,19,29,21,3,8,15,21,28,4};
complex16 ccondok5x4[20] = {{5.,-10.}, {9.,-18.}, {10.,-20.}, {8.,-16.}, {1.,-2.}, {9.,-18.}, {17.,-34.}, {19.,-38.}, {15.,-30.}, {2.,-4.}, {10.,-20.}, {19.,-38.}, {29.,-58.}, {21.,-42.}, {3.,-6.}, {8.,-16.}, {15.,-30.}, {21.,-42.}, {28.,-56.}, {4.,-8.}};

// ok cond pivot is [5,4,3,2,1], dim 5x3
real16 rpivot5_1[15] = { 1., 4., 7., -10., -13., 2., -5., 8., 11., -14., 3., 6., -9., -12., -15. };

// singular 3x3
real16 rsingular3x3[9] = {1,10,1,2,20,2,3,30,3};
complex16 csingular3x3[9] = {{1,10},{10,100},{1,10},{2,20},{20,200},{2,20},{3,30},{30,300},{3,30}};

// 1:10 vector
real16 r1_10[10]{ 1., 2., 3., 4., 5., 6., 7., 8., 9., 10 };
complex16 c1_10[10]{ {1.,10}, {2.,20}, {3.,30}, {4.,40}, {5.,50}, {6.,60}, {7.,70}, {8.,80}, {9.,90}, {10,100} };

/**
 * Holds the data used in an LU decomposition test, pointers are wired only, no ownership is taken.
 */
class LUTestDataHolder
{
  public:
    /**
     * @param input the input matrix
     * @param expectedL the expected lower matrix
     * @param expectedU the expected upper matrix
     */
    LUTestDataHolder(pOGTerminal input, pOGTerminal expectedL, pOGTerminal expectedU)
    {
      _input = input;
      _expectedL = expectedL;
      _expectedU = expectedU;
    }
  pOGTerminal getInput()
  {
    return _input;
  }
  pOGTerminal getExpectedL()
  {
    return _expectedL;
  }
  pOGTerminal getExpectedU()
  {
    return _expectedU;
  }
  private:
    pOGTerminal _input;
    pOGTerminal _expectedL;
    pOGTerminal _expectedU;
};

/**
 * Performs the actual checks on the LU decomposition.
 */
void check_lu(LUTestDataHolder data)
{
  pOGTerminal input = data.getInput();
  pOGTerminal expectedL = data.getExpectedL();
  pOGTerminal expectedU = data.getExpectedU();
  pOGExpr lu = pOGExpr{new LU(input)};
  runtree(lu);
  pOGTerminal L = pOGTerminal{lu->getRegs()[0]->asOGTerminal()};
  pOGTerminal U = pOGTerminal{lu->getRegs()[1]->asOGTerminal()};

  // check numerical answer
  EXPECT_TRUE(L->mathsequals(expectedL,1e-14,1e-14));
  EXPECT_TRUE(U->mathsequals(expectedU,1e-14,1e-14));

  pOGExpr mt = pOGExpr{new MTIMES(L, U)};
  runtree(mt);
  pOGTerminal reconstruct = mt->getRegs()[0]->asOGTerminal();

  // check L*U == A
  EXPECT_TRUE(input->mathsequals(reconstruct, 1e-14, 1e-14));
}

class LUTest: public ::testing::TestWithParam<LUTestDataHolder> {};
class LUSingularTest: public ::testing::TestWithParam<pOGTerminal> {};

TEST_P(LUTest, Exec)
{
  check_lu(GetParam());
}

TEST_P(LUSingularTest,Exec)
{
  // check warn on singular input
  pOGTerminal M = GetParam();
  pOGExpr lu = pOGExpr{new LU(M)};
  runtree(lu);
  // TODO: CHECK WARNING (once logging is in place)
}

LUTestDataHolder testData[] =
{
  // real scalar test
  LUTestDataHolder(
                  pOGTerminal{new OGRealScalar(13)},
                  pOGTerminal{new OGRealScalar(1)},
                  pOGTerminal{new OGRealScalar(13)}
                  ),
  // real 5x4 matrix test
  LUTestDataHolder(
                  pOGTerminal{new OGRealMatrix(rcondok5x4,5,4,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[20] {0.5,0.9,1,0.8,0.1,1,0.2,0,0.4,-0.2,0,1,0,0.0645161290322571,0.1290322580645162,0,0,0,1,0.1480519480519481},5,4,OWNER)},
                  pOGTerminal{new OGRealMatrix(new real16[16] {10,0,0,0,19,-0.5,0,0,29,-4.5,-6.2,0,21,-2.5,-3.4,12.4193548387096779}, 4,4, OWNER)}
                  ),
  // real 2x10 matrix test
  LUTestDataHolder(
                  pOGTerminal{new OGRealMatrix(rcondok5x4,2,10,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[4] {0.5555555555555555,1.,1.,0.},2,2,OWNER)},
                  pOGTerminal{new OGRealMatrix(new real16[20] {9,0,8,5.5555555555555554,9,-4,19,6.4444444444444446,2,13.8888888888888893,19,-0.5555555555555554,21,17.3333333333333321,8,-1.4444444444444446,21,3.3333333333333321,4,25.7777777777777786}, 2,10, OWNER)}
                  ),
  // real reverse pivot in matrix test
  LUTestDataHolder(
                  pOGTerminal{new OGRealMatrix(rpivot5_1,5,3,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[15] {-0.0769230769230769,-0.3076923076923077,-0.5384615384615385,0.7692307692307693,1.,0.0424028268551237,-0.4275618374558304,0.0212014134275618,1.,0,-0.1093167701863354,-0.0695652173913043,1.,0,0},5,3,OWNER)},
                  pOGTerminal{new OGRealMatrix(new real16[9]  {-13.,0.,0.,-14.,21.7692307692307701,0.,-15.,-0.4615384615384617,-17.0671378091872832}, 3,3, OWNER)}
                  ),
  // real horizontal vector test
  LUTestDataHolder(
                  pOGTerminal{new OGRealMatrix(r1_10,1,10,VIEWER)},
                  pOGTerminal{new OGRealScalar(1)},
                  pOGTerminal{new OGRealMatrix(r1_10,1,10, VIEWER)}
                  ),
  // real vertical vector test
  LUTestDataHolder(
                  pOGTerminal{new OGRealMatrix(r1_10,10,1,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[10]{0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0},10,1, OWNER)},
                  pOGTerminal{new OGRealScalar(10)}
                  ),
  // complex scalar test
  LUTestDataHolder(
                  pOGTerminal{new OGComplexScalar({13,37})},
                  pOGTerminal{new OGRealScalar(1)},
                  pOGTerminal{new OGComplexScalar({13,37})}
                  ),
  // complex 5x4 matrix test
  LUTestDataHolder(
                  pOGTerminal{new OGComplexMatrix(ccondok5x4,5,4,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[20] {0.5,0.9,1.,0.8,0.1,1.,0.2,0.,0.4,-0.2,0.,1.,0.,0.0645161290322568,0.1290322580645157,0.,0.,0.,1.,0.1480519480519480},5,4,OWNER)},
                  pOGTerminal{new OGComplexMatrix(new complex16[16] {{10.,-20.}, {0.,0.}, {0.,0.}, {0.,0.}, {19.,-38.}, {-0.5,1.}, {0.,0.}, {0.,0.}, {29.,-58.}, {-4.5,9.}, {-6.2000000000000171,12.4000000000000341}, {0.,0.}, {21.,-42.}, {-2.5,5.}, {-3.4000000000000092,6.8000000000000185}, {12.4193548387096779,-24.8387096774193559}}, 4,4, OWNER)}
                  ),
  // complex 2x10 matrix test
  LUTestDataHolder(
                  pOGTerminal{new OGComplexMatrix(ccondok5x4,2,10,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[4] {0.5555555555555555,1.,1.,0.},2,2,OWNER)},
                  pOGTerminal{new OGComplexMatrix(new complex16[20] {{9.,-18.}, {0., 0.}, {8.,-16.}, {5.5555555555555554,-11.1111111111111107}, {9.,-18.}, {-4., 8.}, {19.,-38.}, {6.4444444444444446,-12.8888888888888893}, {2.,-4.}, {13.8888888888888893,-27.7777777777777786}, {19.,-38.}, {-0.5555555555555554, 1.1111111111111107}, {21.,-42.}, {17.3333333333333321,-34.6666666666666643}, {8.,-16.}, {-1.4444444444444446, 2.8888888888888893}, {21.,-42.}, {3.3333333333333321,-6.6666666666666643}, {4.,-8.}, {25.7777777777777786,-51.5555555555555571}}, 2,10, OWNER)}
                  ),
  // complex horizontal vector test
  LUTestDataHolder(
                  pOGTerminal{new OGComplexMatrix(c1_10,1,10,VIEWER)},
                  pOGTerminal{new OGRealScalar(1)},
                  pOGTerminal{new OGComplexMatrix(c1_10,1,10, VIEWER)}
                  ),
  // complex vertical vector test
  LUTestDataHolder(
                  pOGTerminal{new OGComplexMatrix(c1_10,10,1,VIEWER)},
                  pOGTerminal{new OGRealMatrix(new real16[10]{0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0},10,1, OWNER)},
                  pOGTerminal{new OGComplexScalar({10,100})}
                  )
};

INSTANTIATE_TEST_CASE_P(LUTesting, LUTest, ::testing::ValuesIn(testData));


pOGTerminal testSingluarData[] =
{
  // real scalar test
  pOGTerminal{new OGRealScalar(0)},
  pOGTerminal{new OGRealMatrix(rsingular3x3,3,3)},
  pOGTerminal{new OGComplexScalar({0,0})},
  pOGTerminal{new OGComplexMatrix(csingular3x3,3,3)}
};

INSTANTIATE_TEST_CASE_P(LUTesting, LUSingularTest, ::testing::ValuesIn(testSingluarData));

