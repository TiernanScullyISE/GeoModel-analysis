/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelKernel/RCBase.h"
#include <exception>
#include <gtest/gtest.h>
#include <limits>

class Stub:public RCBase{
  public:
  Stub() = default;
  virtual ~Stub() = default;
};

//Making the destructor of RCBase private would cause all the defaulted destructors
//of derived classes (e.g the 'Stub' above, and many others) no longer compile.
//However, keeping it protected results in this undesirable behaviour:
TEST(RCBase, StubCanBeConstructedOnStack) {
  EXPECT_NO_THROW([[maybe_unused]] Stub stub);
  //Stub stub1; (unused here if the following are commented out)
  //none of the following compile (correctly so)
  //Stub stub2(stub1);
  //Stub stub3 = stub1;
  //Stub stub4(std::move(stub1);
  //Stub stub5 = std::move(stub3);
}

TEST(RCBase, CanBeConstructedOnHeap) {
  RCBase * p{};
  EXPECT_NO_THROW(p = new RCBase);
  EXPECT_NE(p, nullptr);
  //deliberate leak here, RCBase d'tor is protected, so cannot delete
}

TEST(RCBase, StubCanBeConstructedOnHeap) {
  Stub * p{};
  EXPECT_NO_THROW(p = new Stub);
  EXPECT_NE(p, nullptr);
  //can delete the derived class directly
  EXPECT_NO_THROW(delete p);
}

TEST(RCBase, CanIncrementAndDecrementReference){
  //do on heap, as is supposed to be
  Stub * pStub = new Stub;
  EXPECT_EQ(pStub->refCount(), 0);
  EXPECT_NO_THROW(pStub->ref());
  EXPECT_EQ(pStub->refCount(), 1);
  EXPECT_NO_THROW(pStub->unref());
}
//undesirable behaviours
TEST(RCBase, CannotDecrementReferenceBelowZero){
  Stub * pStub = new Stub;
  EXPECT_NO_THROW(pStub->unref()); 
 
}

TEST(RCBase, DISABLED_CannotBeUsedAfterRefCountBecomesZero){
  Stub * pStub = new Stub;
  EXPECT_NO_THROW(pStub->ref());
  EXPECT_NO_THROW(pStub->unref());//object destroyed, but pointer still valid
  EXPECT_THROW(pStub->ref(), std::exception);//compiles, doesn't throw
  //now the refcount is some random value (returns 2043 on my machine)
  //deleting would cause segfault
  //delete pStub;
}
