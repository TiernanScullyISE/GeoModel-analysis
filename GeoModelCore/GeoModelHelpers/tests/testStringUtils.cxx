/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/



#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelKernel/GeoElement.h"


#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>


#define GTEST_COUT std::cerr << "[ MESSAGE  ] "

using namespace GeoStrUtils;

TEST(StringUtils, resolveEnviromentVariablesTest){
  EXPECT_EQ(resolveEnviromentVariables("${PWD}") , getEnvVar("PWD"));
  EXPECT_EQ(resolveEnviromentVariables("${PWD}/Kuchen"), getEnvVar("PWD")+"/Kuchen");
}

TEST(StringUtils, chainUpTest) {
  std::vector<std::string> vs {"this", "is", "a", "test"};
  EXPECT_EQ(chainUp(vs, ""), "thisisatest");
  EXPECT_EQ(chainUp(vs, ";"), "this;is;a;test");
  EXPECT_EQ(chainUp(vs, " "), "this is a test");
  std::vector<std::string> emptyVector;
  EXPECT_EQ(chainUp(emptyVector, ""), "");
  EXPECT_EQ(chainUp(emptyVector), "");
}

TEST(StringUtils, CanGetAddressStringFromPointer){
  auto pElement = make_intrusive<GeoElement>("Rhodium","Rh", 45.,102.9);
  std::string addrStr;
  EXPECT_NO_THROW(addrStr = getAddressStringFromPointer(pElement.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //etc etc; there are no 'special cases' but there was originally a limited list of things 
  //allowed to be converted, so the following would not compile:
  //
  int * pI{};
  EXPECT_NO_THROW(addrStr = getAddressStringFromPointer(pI));
  //Using the new function signature 
  // getAddressStringFromPointer(const auto * p);
  //this is no longer true, and any bare pointer can be 'stringified'.
  //The original constraint _can_ be restored using concepts, however.
}

