#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoTube.h"

#include <gtest/gtest.h>
#include <type_traits>

using namespace GeoModelKernelUnits;


class SampleMaterial{
  private:
    const std::string m_name{"Rhodium"};
    const double m_dense{13.0*gram/centimeter3};
    GeoIntrusivePtr<GeoMaterial>  m_ptr{};
  public:
    GeoMaterial * ptr() const {return m_ptr.get();}
    SampleMaterial():m_ptr(make_intrusive<GeoMaterial>(m_name, m_dense)){
      //
    }
};

class GeoVolumeCursorTest{
private:
  const GeoVolumeCursor & m_g;
public:
  GeoVolumeCursorTest(const GeoVolumeCursor& g):m_g(g){ /* nop */}
  PVConstLink parent() const ;
  PVConstLink volume() const;
  VSConstLink surface() const ;
  GeoTrf::Transform3D transform() const;
  GeoTrf::Transform3D defTransform() const;
  
  unsigned int majorIndex() const;
  unsigned int minorIndex() const;
  unsigned int minorLimit() const;
  const GeoSerialTransformer * serialTransformer() const;
  
  const GeoNameTag *nameTag() const;
  const GeoSerialDenominator *m_serialDenominator() const;
  const GeoIdentifierTag *m_idTag();
  std::vector<const GeoTransform *>  pendingTransformList() const;
  unsigned int serialDenomPosition() const;
  const GeoSerialIdentifier *m_serialIdentifier() const;
  unsigned int serialIdentPosition() const;
  unsigned int  volCount() const;
  bool hasAlignTrans() const;
  GeoVAlignmentStore *alignStore() const;
};

TEST(GeoVolumeCursor, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoVolumeCursor>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoVolumeCursor>);
}

TEST(GeoVolumeCursor, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoVolumeCursor>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoVolumeCursor>);
}

TEST(GeoVolumeCursor, HasExpectedPublicProperties) {
  SampleMaterial s;
  auto pTube = make_intrusive<GeoTube>(0.5, 0.7, 1.0);
  auto pMaterial = s.ptr();
  auto pLogVol =  make_intrusive<GeoLogVol>("myTube", pTube, pMaterial);
  //needs a valid pointer due to the call to next, otherwise will segfault
  auto p = make_intrusive<GeoPhysVol>(pLogVol);
  GeoVolumeCursor g(p, nullptr);//no alignment store by default
  GeoTrf::Transform3D identity{GeoTrf::Transform3D::Identity()};
  EXPECT_TRUE(g.atEnd());
  EXPECT_EQ(g.getVolume() ,nullptr);
  EXPECT_TRUE(g.getTransform().isApprox(identity));
  EXPECT_TRUE(g.getDefTransform().isApprox(identity));
  EXPECT_EQ(g.getName(),"ANON");
  EXPECT_FALSE(bool(g.getId()));
  EXPECT_FALSE(g.hasAlignableTransform());
}

