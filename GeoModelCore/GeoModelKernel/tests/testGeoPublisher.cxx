/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoPublisher.h"

#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoTube.h"

#include <gtest/gtest.h>

/*
  Unit tests for GeoModelKernel::GeoPublisher.

  GeoPublisher is used to store ("publish") selected geometry GeoModel nodes 
  that must later be retrieved after persistification of the
  geometry model; for example, to build the Readout Geometry at the 
  experiment software level. 
  
  Currently, the following node categories can be published:

    - GeoVFullPhysVol
    - GeoAlignableTransform

  Each published node is associated with a user-defined key. The key is stored
  as a GeoPublisher::DBRecord variant and may currently be one of:

    - int
    - long
    - float
    - double
    - std::string

  The key is intended to provide a user-defined identifier that
  can be written to the GeoModel SQLite database together with the published
  node and later used to retrieve the node and re-establish associations 
  between geometry objects and application data.

  In addition, GeoPublisher can store auxiliary user-defined data tables that
  are written together with the geometry by GeoModelIO::WriteGeoModel. 
  Those are data that need to be used later at the experiment software level.

  The tests below verify:

    - Publisher name storage and retrieval.
    - Publication of GeoVFullPhysVol nodes.
    - Publication of GeoAlignableTransform nodes.
    - Correct storage and retrieval of publication keys.
    - Support for all currently allowed key types:
        * int
        * long
        * float
        * double
        * std::string
    - Preservation of the key type stored in the DBRecord variant.
    - Distinction between numerically equal keys having different C++
      types (e.g. int versus long).
    - Duplicate-record suppression.
    - Support for multiple keys associated with the same node.
    - Handling of invalid (nullptr) inputs.
    - Storage and retrieval of auxiliary data tables.



  *** Note: ***
  These tests validate the GeoPublisher class / API only.
  Persistification of published nodes, publication keys, and auxiliary data
  into SQLite databases is tested separately in the GeoModelIO packages.
*/



using namespace GeoModelKernelUnits;

template <class MapT>
bool hasKey(const MapT& m, const std::string& key)
{
#if __cplusplus >= 202002L
  return m.contains(key);
#else
  return m.find(key) != m.end();
#endif
}

namespace {

GeoIntrusivePtr<GeoFullPhysVol> makeFPV()
{
  auto material =
      make_intrusive<GeoMaterial>(
          "TestMaterial",
          1.0 * gram / centimeter3);

  auto shape =
      make_intrusive<GeoTube>(
          1.0,
          2.0,
          3.0);

  auto logVol =
      make_intrusive<GeoLogVol>(
          "TestLogVol",
          shape,
          material.get());

  return make_intrusive<GeoFullPhysVol>(
      logVol.get());
}

} // namespace


TEST(GeoPublisher, NameCanBeStoredAndRetrieved)
{
  GeoPublisher pub;

  pub.setName("MyPublisher");

  EXPECT_EQ(pub.getName(), "MyPublisher");
}


TEST(GeoPublisher, PublishFullPhysVol)
{
  GeoPublisher pub;

  auto fpv = makeFPV();

  GeoVFullPhysVol* fpvBase = fpv.get();

  pub.publishNode(fpvBase, 42);

  auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 1u);

  auto it = records.begin();

  EXPECT_EQ(it->first, fpvBase);
  EXPECT_EQ(std::get<int>(it->second), 42);
}


TEST(GeoPublisher, DuplicateFullPhysVolRecordIsIgnored)
{
  GeoPublisher pub;

  auto fpv = makeFPV();

  GeoVFullPhysVol* fpvBase = fpv.get();

  pub.publishNode(fpvBase, 7);
  pub.publishNode(fpvBase, 7);

  EXPECT_EQ(pub.getPublishedFPV().size(), 1u);
}


TEST(GeoPublisher, SameFullPhysVolMayBePublishedWithDifferentKeys)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  pub.publishNode(fpvBase, 1);
  pub.publishNode(fpvBase, 2);

  auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 2u);

  bool foundKey1 = false;
  bool foundKey2 = false;

  for (const auto& [node, key] : records) {

    EXPECT_EQ(node, fpvBase);

    if (std::holds_alternative<int>(key)) {
      int value = std::get<int>(key);

      if (value == 1) foundKey1 = true;
      if (value == 2) foundKey2 = true;
    }
  }

  EXPECT_TRUE(foundKey1);
  EXPECT_TRUE(foundKey2);
}


TEST(GeoPublisher, PublishAlignableTransform)
{
  GeoPublisher pub;

  auto axf =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  pub.publishNode(
      axf.get(),
      std::string("AXF"));

  auto records = pub.getPublishedAXF();

  ASSERT_EQ(records.size(), 1u);

  auto it = records.begin();

  EXPECT_EQ(it->first, axf.get());
  EXPECT_EQ(
      std::get<std::string>(it->second),
      "AXF");
}


TEST(GeoPublisher, DuplicateAlignableTransformRecordIsIgnored)
{
  GeoPublisher pub;

  auto axf =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  pub.publishNode(axf.get(), 5);
  pub.publishNode(axf.get(), 5);

  EXPECT_EQ(pub.getPublishedAXF().size(), 1u);
}


TEST(GeoPublisher, NullptrPublishThrows)
{
  GeoPublisher pub;

  EXPECT_ANY_THROW(
      pub.publishNode(
          static_cast<GeoVFullPhysVol*>(nullptr),
          1));

  EXPECT_ANY_THROW(
      pub.publishNode(
          static_cast<GeoAlignableTransform*>(nullptr),
          1));
}


TEST(GeoPublisher, AuxiliaryTablesCanBeStored)
{
  GeoPublisher pub;

  std::vector<std::string> names{"id", "name"};
  std::vector<std::string> types{"INT", "TEXT"};

  std::vector<std::vector<GeoPublisher::DBRecord>> rows{
      {1, std::string("one")},
      {2, std::string("two")}
  };

  pub.storeDataTable(
      "MyTable",
      names,
      types,
      rows);

  auto [defs, data] =
      pub.getPublishedAuxData();

  ASSERT_EQ(defs.size(), 1u);
  ASSERT_EQ(data.size(), 1u);

  ASSERT_TRUE(hasKey(defs, "MyTable"));
  ASSERT_TRUE(hasKey(data, "MyTable"));

  EXPECT_EQ(defs["MyTable"].first, names);
  EXPECT_EQ(defs["MyTable"].second, types);

  EXPECT_EQ(data["MyTable"].size(), 2u);
}



TEST(GeoPublisher, PublishFullPhysVolWithLongKey)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  long key = 123456789L;

  pub.publishNode(fpvBase, key);

  auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 1u);

  auto it = records.begin();

  EXPECT_EQ(it->first, fpvBase);
  EXPECT_EQ(std::get<long>(it->second), key);
}


TEST(GeoPublisher, PublishFullPhysVolWithFloatKey)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  float key = 3.14159f;

  pub.publishNode(fpvBase, key);

  auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 1u);

  auto it = records.begin();

  EXPECT_EQ(it->first, fpvBase);
  EXPECT_FLOAT_EQ(
      std::get<float>(it->second),
      key);
}


TEST(GeoPublisher, PublishFullPhysVolWithDoubleKey)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  double key = 2.718281828;

  pub.publishNode(fpvBase, key);

  auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 1u);

  auto it = records.begin();

  EXPECT_EQ(it->first, fpvBase);
  EXPECT_DOUBLE_EQ(
      std::get<double>(it->second),
      key);
}


TEST(GeoPublisher, IntAndLongKeysAreDistinct)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  pub.publishNode(fpvBase, 1);
  pub.publishNode(fpvBase, 1L);

  EXPECT_EQ(
      pub.getPublishedFPV().size(),
      2u);
}


TEST(GeoPublisher, SameAlignableTransformMayBePublishedWithDifferentKeys)
{
  GeoPublisher pub;

  auto axf =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  pub.publishNode(axf.get(), 1);
  pub.publishNode(axf.get(), 2);

  auto records = pub.getPublishedAXF();

  ASSERT_EQ(records.size(), 2u);

  bool foundKey1 = false;
  bool foundKey2 = false;

  for (const auto& [node, key] : records) {

    EXPECT_EQ(node, axf.get());

    if (std::holds_alternative<int>(key)) {
      int value = std::get<int>(key);

      if (value == 1) foundKey1 = true;
      if (value == 2) foundKey2 = true;
    }
  }

  EXPECT_TRUE(foundKey1);
  EXPECT_TRUE(foundKey2);
}


TEST(GeoPublisher, DuplicateAlignableTransformPublicationIsIgnored)
{
  GeoPublisher pub;

  auto axf =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  pub.publishNode(axf.get(), 123);
  pub.publishNode(axf.get(), 123);

  auto records = pub.getPublishedAXF();

  ASSERT_EQ(records.size(), 1u);

  auto it = records.begin();

  EXPECT_EQ(it->first, axf.get());
  EXPECT_EQ(std::get<int>(it->second), 123);
}


