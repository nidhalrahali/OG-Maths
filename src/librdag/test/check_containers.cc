/**
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for license.
 */

#include "containers.hh"
#include "gtest/gtest.h"

using namespace std;
using namespace librdag;

/*
 * Test rationale: check the functionality that we wrapped in the derived class
 * only. Otherwise we're just testing the C++ compiler/STL implementation.
 */
TEST(ContainerTest, OwningPtrVectorTest) {
  // Default constructor
  OwningPtrVector<const int*> *pv1 = new OwningPtrVector<const int*>();
  EXPECT_EQ(0, pv1->size());

  // Add elements
  size_t n = 5;
  int** i_ptr_list = (int**)malloc(sizeof(int*)*n);
  for (size_t i = 0; i < n; ++i)
  {
    int* intp = new int(i);
    i_ptr_list[i] = intp;
    pv1->push_back(intp);
  }

  // Check size matches number of inserted elements
  ASSERT_EQ(5, pv1->size());

  // Test operator[]
  for (size_t i = 0; i < n; ++i)
  {
    const int* ptr = (*pv1)[i];
    EXPECT_EQ(i_ptr_list[i], ptr);
    EXPECT_EQ(i, *ptr);
  }

  // Test iteration over list
  int i = 0;
  for (auto it = pv1->begin(); it != pv1->end(); ++it)
  {
    EXPECT_EQ(*it, i_ptr_list[i]);
    EXPECT_EQ(i, **it);
    ++i;
  }
  EXPECT_EQ(5, i);

  // Copy the OwningPtrVector
  OwningPtrVector<const int*> *pv2 = pv1->copy();

  // Test the copy is the same as the original
  OwningPtrVector<const int*>::citerator it1, it2;
  for (it1 = pv1->begin(), it2 = pv2->begin(); it1 != pv1->end(), it2 != pv2->end(); ++it1, ++it2)
  {
    EXPECT_EQ(**it1, **it2);
  }
  EXPECT_TRUE( (it1 == pv1->end()) && (it2 == pv2->end()) );

  // Attempt to put a null pointer in the container
  EXPECT_THROW(pv1->push_back(nullptr), rdag_error);

  // Delete - no leaks should occur. pv1 owns the pointers to individual ints.
  delete pv1;
  delete pv2;
  // However, the list of pointers is not owned by it.
  free(i_ptr_list);
}

/**
 * Almost verbatim copy of the above test. Can't have slightly different behaviour that
 * we need if we do a type-parameterised test.
 */
TEST(ContainerTest, NonOwningPtrVectorTest) {
  // Default constructor
  NonOwningPtrVector<const int*> *pv1 = new NonOwningPtrVector<const int*>();
  EXPECT_EQ(0, pv1->size());

  // Add elements
  size_t n = 5;
  int** i_ptr_list = (int**)malloc(sizeof(int*)*n);
  for (size_t i = 0; i < n; ++i)
  {
    int* intp = new int(i);
    i_ptr_list[i] = intp;
    pv1->push_back(intp);
  }

  // Check size matches number of inserted elements
  ASSERT_EQ(5, pv1->size());

  // Test operator[]
  for (size_t i = 0; i < n; ++i)
  {
    const int* ptr = (*pv1)[i];
    EXPECT_EQ(i_ptr_list[i], ptr);
    EXPECT_EQ(i, *ptr);
  }

  // Test iteration over list
  int i = 0;
  for (auto it = pv1->begin(); it != pv1->end(); ++it)
  {
    EXPECT_EQ(*it, i_ptr_list[i]);
    EXPECT_EQ(i, **it);
    ++i;
  }
  EXPECT_EQ(5, i);

  // Copy the NonOwningPtrVector
  NonOwningPtrVector<const int*> *pv2 = pv1->copy();

  // Test the copy is the same as the original
  NonOwningPtrVector<const int*>::citerator it1, it2;
  for (it1 = pv1->begin(), it2 = pv2->begin(); it1 != pv1->end(), it2 != pv2->end(); ++it1, ++it2)
  {
    EXPECT_EQ( *it1,  *it2);
    EXPECT_EQ(**it1, **it2);
  }
  EXPECT_TRUE( (it1 == pv1->end()) && (it2 == pv2->end()) );

  // Attempt to put a null pointer in the container
  EXPECT_THROW(pv1->push_back(nullptr), rdag_error);

  // Delete everything
  delete pv1;
  delete pv2;
  // The container does not own the ints so we need to delete them
  for (size_t i = 0; i < n; ++i)
  {
    delete i_ptr_list[i];
  }
  free(i_ptr_list);
}
