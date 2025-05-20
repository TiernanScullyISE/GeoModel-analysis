#include "GeoModelKernel/GeoSurfaceCursor.h"
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


//class purely to expose private members for testing, declared as 'friend'
class GeoSurfaceCursorTest{
private:
  const GeoSurfaceCursor & m_g;
  public:
  GeoSurfaceCursorTest(const GeoSurfaceCursor & g):m_g(g){
  }
  PVConstLink parent() const {return m_g.m_parent;}
  PVConstLink volume() const {return m_g.m_volume;}
  VSConstLink surface() const {return m_g.m_surface;}
  GeoTrf::Transform3D  transform() const { return m_g.m_transform;}
  GeoTrf::Transform3D defTransform() const {return m_g.m_defTransform;}
  unsigned int majorIndex() const { return m_g.m_majorIndex;}
  unsigned int volCount() const { return m_g.m_volCount;}
  unsigned int surfCount() const { return m_g.m_surfCount;}
  std::vector<const GeoTransform *> pendingTransformList() const { return m_g.m_pendingTransformList;}
  bool hasAlignTrans() const{return m_g.m_hasAlignTrans;}
  GeoVAlignmentStore * alignStore() const {return m_g.m_alignStore;}
  
};

TEST(GeoSurfaceCursor, CanBeParameterConstructed){
  SampleMaterial s;
  auto pTube = make_intrusive<GeoTube>(0.5, 0.7, 1.0);
  auto pMaterial = s.ptr();
  auto pLogVol =  make_intrusive<GeoLogVol>("myTube", pTube, pMaterial);
  //needs a valid pointer due to the call to next, otherwise will segfault
  auto p = make_intrusive<GeoPhysVol>(pLogVol);
  EXPECT_NO_THROW(GeoSurfaceCursor g(p, nullptr));
}

TEST(GeoSurfaceCursor, HasExpectedPublicProperties){
  SampleMaterial s;
  auto pTube = make_intrusive<GeoTube>(0.5, 0.7, 1.0);
  auto pMaterial = s.ptr();
  auto pLogVol =  make_intrusive<GeoLogVol>("myTube", pTube, pMaterial);
  //needs a valid pointer due to the call to next, otherwise will segfault
  auto p = make_intrusive<GeoPhysVol>(pLogVol);
  GeoSurfaceCursor g(p, nullptr);//no alignment store by default
  GeoTrf::Transform3D identity{GeoTrf::Transform3D::Identity()};
  //public methods
  EXPECT_TRUE(g.getTransform().isApprox(identity));
  EXPECT_TRUE(g.getDefTransform().isApprox(identity));
  EXPECT_TRUE(g.atEnd());
}

TEST(GeoSurfaceCursor, HasExpectedPrivateProperties){
  SampleMaterial s;
  auto pTube = make_intrusive<GeoTube>(0.5, 0.7, 1.0);
  auto pMaterial = s.ptr();
  auto pLogVol =  make_intrusive<GeoLogVol>("myTube", pTube, pMaterial);
  //needs a valid pointer due to the call to next, otherwise will segfault
  auto p = make_intrusive<GeoPhysVol>(pLogVol);
  GeoSurfaceCursor g(p, nullptr);//no alignment store by default
  GeoSurfaceCursorTest test(g);
  EXPECT_EQ(test.parent(), p.get());
  EXPECT_EQ(test.volume(), nullptr);
  EXPECT_EQ(test.surface(), nullptr);
  EXPECT_EQ(test.majorIndex(), 0);
  EXPECT_EQ(test.volCount(), 0);
  EXPECT_EQ(test.surfCount(), 0);
  EXPECT_FALSE(test.hasAlignTrans());
}


TEST(GeoSurfaceCursor, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoSurfaceCursor>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoSurfaceCursor>);
}

TEST(GeoSurfaceCursor, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoSurfaceCursor>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoSurfaceCursor>);
}

TEST(GeoSurfaceCursor, DefaultConstructedPropertiesAreAsExpected){
  

}
