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

#include <stdexcept>


//
// Unit tests for the GeoPublisher class.
//
// GeoPublisher is used to associate user-defined keys with published detector
// nodes so that they can later be identified after writing and reading a
// geometry database.
//
// The publisher currently supports two node categories:
//
//   - GeoVFullPhysVol
//   - GeoAlignableTransform
//
// Publication keys may be of any of the supported DBRecord types:
//
//   - int
//   - long
//   - float
//   - double
//   - std::string
//
// The publisher also stores arbitrary auxiliary tables that are written to the
// output geometry database together with the published node information.
//
// These unit tests verify:
//
//   - publisher name storage and retrieval;
//   - publication of GeoVFullPhysVol and GeoAlignableTransform nodes;
//   - acceptance of multiple keys associated with the same node;
//   - rejection of duplicate publication keys for nodes of the same category;
//   - preservation of the original key-to-node association after a failed
//     duplicate publication attempt;
//   - acceptance of the same publication key for different node categories
//     (one GeoVFullPhysVol and one GeoAlignableTransform);
//   - rejection of nullptr publications;
//   - storage of auxiliary data tables;
//   - support for all supported publication key types;
//   - preservation of the original key type (e.g. int versus long).
//
//
//   *** Note: ***
//  These tests validate the GeoPublisher class / API only.
//  Persistification of published nodes, publication keys, and auxiliary data
//  into SQLite databases is tested separately in the GeoModelIO packages.
//

/* In particular, this is the list of the tests with their purpose:
| # | Test | Purpose |
|---:|------|---------|
| 1 | `NameCanBeStoredAndRetrieved` | Publisher name storage and retrieval |
| 2 | `PublishFullPhysVol` | Basic `GeoVFullPhysVol` publication |
| 3 | `DuplicateFullPhysVolKeyThrows` | Publishing the same FPV with the same key throws an exception |
| 4 | `DifferentFullPhysVolsMayNotReuseTheSameKey` | Different FPVs cannot be published with the same key |
| 5 | `SameFullPhysVolMayBePublishedWithDifferentKeys` | The same FPV may be published with multiple different keys |
| 6 | `PublishAlignableTransform` | Basic `GeoAlignableTransform` publication |
| 7 | `DuplicateAlignableTransformKeyThrows` | Publishing the same AXF with the same key throws an exception |
| 8 | `DifferentAlignableTransformsMayNotReuseTheSameKey` | Different AXFs cannot be published with the same key |
| 9 | `NullptrPublishThrows` | Publishing a `nullptr` throws an exception |
| 10 | `AuxiliaryTablesCanBeStored` | Auxiliary tables can be stored and retrieved |
| 11 | `PublishFullPhysVolWithLongKey` | Publication using a `long` key |
| 12 | `PublishFullPhysVolWithFloatKey` | Publication using a `float` key |
| 13 | `PublishFullPhysVolWithDoubleKey` | Publication using a `double` key |
| 14 | `IntAndLongKeysAreDistinct` | `int` and `long` keys with the same numeric value remain distinct |
| 15 | `SameAlignableTransformMayBePublishedWithDifferentKeys` | The same AXF may be published with multiple different keys |
| 16 | `SameKeyMayBeUsedForFPVAndAXF` | The same publication key may be reused across different node categories (one FPV and one AXF) |
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

  EXPECT_EQ(std::get<int>(it->first), 42);
  EXPECT_EQ(it->second, fpvBase);
}


TEST(GeoPublisher, DuplicateFullPhysVolKeyThrows)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  pub.publishNode(fpvBase, 7);

  EXPECT_THROW(
      pub.publishNode(fpvBase, 7),
      std::runtime_error);

  const auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 1u);

  const auto it = records.find(GeoPublisher::DBRecord{7});

  ASSERT_NE(it, records.end());
  EXPECT_EQ(it->second, fpvBase);
}


TEST(GeoPublisher, DifferentFullPhysVolsMayNotReuseTheSameKey)
{
  GeoPublisher pub;

  auto firstFPV = makeFPV();
  auto secondFPV = makeFPV();

  GeoVFullPhysVol* firstFPVBase = firstFPV.get();
  GeoVFullPhysVol* secondFPVBase = secondFPV.get();

  pub.publishNode(firstFPVBase, 17);

  EXPECT_THROW(
      pub.publishNode(secondFPVBase, 17),
      std::runtime_error);

  const auto records = pub.getPublishedFPV();

  ASSERT_EQ(records.size(), 1u);

  const auto it = records.find(GeoPublisher::DBRecord{17});

  ASSERT_NE(it, records.end());
  EXPECT_EQ(it->second, firstFPVBase);
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

  for (const auto& [key, node] : records) {

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

  EXPECT_EQ(
      std::get<std::string>(it->first),
      "AXF");
  EXPECT_EQ(it->second, axf.get());
}


TEST(GeoPublisher, DuplicateAlignableTransformKeyThrows)
{
  GeoPublisher pub;

  auto axf =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  pub.publishNode(axf.get(), 5);

  EXPECT_THROW(
      pub.publishNode(axf.get(), 5),
      std::runtime_error);

  const auto records = pub.getPublishedAXF();

  ASSERT_EQ(records.size(), 1u);

  const auto it = records.find(GeoPublisher::DBRecord{5});

  ASSERT_NE(it, records.end());
  EXPECT_EQ(it->second, axf.get());
}


TEST(GeoPublisher, DifferentAlignableTransformsMayNotReuseTheSameKey)
{
  GeoPublisher pub;

  auto firstAXF =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  auto secondAXF =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  pub.publishNode(firstAXF.get(), 23);

  EXPECT_THROW(
      pub.publishNode(secondAXF.get(), 23),
      std::runtime_error);

  const auto records = pub.getPublishedAXF();

  ASSERT_EQ(records.size(), 1u);

  const auto it = records.find(GeoPublisher::DBRecord{23});

  ASSERT_NE(it, records.end());
  EXPECT_EQ(it->second, firstAXF.get());
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

  EXPECT_EQ(std::get<long>(it->first), key);
  EXPECT_EQ(it->second, fpvBase);
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

  EXPECT_FLOAT_EQ(
      std::get<float>(it->first),
      key);
  EXPECT_EQ(it->second, fpvBase);
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

  EXPECT_DOUBLE_EQ(
      std::get<double>(it->first),
      key);
  EXPECT_EQ(it->second, fpvBase);
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

  for (const auto& [key, node] : records) {

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


TEST(GeoPublisher, SameKeyMayBeUsedForFPVAndAXF)
{
  GeoPublisher pub;

  auto fpv = makeFPV();
  GeoVFullPhysVol* fpvBase = fpv.get();

  auto axf =
      make_intrusive<GeoAlignableTransform>(
          GeoTrf::Transform3D::Identity());

  EXPECT_NO_THROW(pub.publishNode(fpvBase, 42));
  EXPECT_NO_THROW(pub.publishNode(axf.get(), 42));

  EXPECT_EQ(pub.getPublishedFPV().size(), 1u);
  EXPECT_EQ(pub.getPublishedAXF().size(), 1u);
}

