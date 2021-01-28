using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace ProjSharp.Tests
{
    [TestClass]
    public class BasicTests
    {
        [TestMethod]
        public void BadInit()
        {
            using (var pc = new ProjContext())
            {
                try
                {
                    using (var crs = CoordinateReferenceSystem.Create(pc, "!1@2#3$4%5^6&7*8(9)0"))
                    {
                    }
                    Assert.Fail();
                }
                catch (ProjException pe)
                {
                    Assert.AreEqual("proj_create: unrecognized format / unknown name", pe.GetBaseException().Message);
                }
            }
        }

        [TestMethod]
        public void CreateAndDestroyContext()
        {
            using (var pc = new ProjContext())
            {
                using (var crs = pc.Create("+proj=merc +ellps=clrk66 +lat_ts=33"))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Description);
                    Assert.AreEqual(false, crs.DegreeInput);
                    Assert.AreEqual(false, crs.DegreeOutput);
                    Assert.AreEqual(true, crs.AngularInput);
                    Assert.AreEqual(false, crs.AngularOutput);

                    Assert.AreEqual(ProjType.OtherCoordinateOperation, crs.Type);
                }

                using (var crs = CoordinateReferenceSystem.Create(pc, "proj=merc", "ellps=clrk66", "lat_ts=33"))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Description);
                }
            }
        }


        [TestMethod]
        public void CreateAndDestroyContextEPSG()
        {
            using (var pc = new ProjContext())
            {
                GC.KeepAlive(pc.Clone());

                // Needs proj.db

                using (var crs = CoordinateReferenceSystem.Create(pc, "EPSG:25832"))
                {
                    Assert.AreEqual("ETRS89 / UTM zone 32N", crs.Description);
                }
            }
        }

        [TestMethod]
        public void CreateBasicTransform()
        {
            using (var pc = new ProjContext())
            {
                using (var crs1 = CoordinateReferenceSystem.Create(pc, "EPSG:25832"))
                using (var crs2 = CoordinateReferenceSystem.Create(pc, "EPSG:25833"))
                {
                    Assert.AreEqual(ProjType.ProjectedCrs, crs1.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs2.Type);

                    using (var t = CoordinateOperation.Create(crs1, crs2))
                    {
                        Assert.AreEqual("Inverse of UTM zone 32N + UTM zone 33N", t.Description);

                        using (var tr = t.CreateInverse())
                        {
                            Assert.AreEqual("Inverse of UTM zone 33N + UTM zone 32N", tr.Description);
                        }
                    }

                    using (var t = CoordinateOperation.Create(crs2, crs1))
                    {
                        Assert.AreEqual("Inverse of UTM zone 33N + UTM zone 32N", t.Description);
                    }
                }
            }
        }

        [TestMethod]
        public void TransformNL()
        {
            using (var pc = new ProjContext())
            {
                pc.LogLevel = ProjLogLevel.Error;
                using (var crs1 = CoordinateReferenceSystem.Create(pc, "EPSG:3857"))
                using (var crs2 = CoordinateReferenceSystem.Create(pc, "EPSG:23095"))
                using (var crs3 = CoordinateReferenceSystem.Create(pc, "EPSG:28992"))
                {
                    Assert.AreEqual("WGS 84 / Pseudo-Mercator", crs1.Description);
                    Assert.AreEqual("ED50 / TM 5 NE", crs2.Description);
                    Assert.AreEqual("Amersfoort / RD New", crs3.Description);

                    Assert.AreEqual(ProjType.ProjectedCrs, crs1.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs2.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs3.Type);

                    using (var c = crs3.GetGeodeticCoordinateReferenceSystem(pc))
                    {
                        Assert.IsTrue((object)c is GeographicCoordinateReferenceSystem);
                        Assert.AreEqual("Amersfoort", c.Description);
                        Assert.AreEqual(ProjType.Geographic2DCrs, c.Type);
                    }
                    using (var d = crs3.GetDatum(pc))
                    {
                        Assert.IsTrue((object)d is ProjDatum);
                        Assert.AreEqual("Amersfoort", d.Description);
                        Assert.AreEqual(ProjType.GeodeticReferenceFrame, d.Type);
                    }
                    using (var d = crs3.GetDatumEnsamble(pc))
                    {
                        Assert.IsNull(d);
                    }
                    using (var d = crs3.GetDatumForced(pc))
                    {
                        Assert.IsTrue((object)d is ProjDatum);
                        Assert.AreEqual("Amersfoort", d.Description);
                        Assert.AreEqual(ProjType.GeodeticReferenceFrame, d.Type);
                    }
                    using (var d = crs3.GetHorizontalDatum(pc))
                    {
                        Assert.IsTrue((object)d is ProjDatum);
                        Assert.AreEqual("Amersfoort", d.Description);
                        Assert.AreEqual(ProjType.GeodeticReferenceFrame, d.Type);
                    }
                    using (var c = crs3.GetCoordinateSystem(pc))
                    {
                        Assert.IsTrue((object)c is CoordinateSystem);
                        Assert.AreEqual("", c.Description);
                        Assert.AreEqual(ProjType.Unknown, c.Type);
                        Assert.AreEqual(CoordinateSystemType.Cartesian, c.CsType);
                        Assert.AreEqual(2, c.AxisCount);
                        Assert.AreEqual(2, c.Axis.Count);

                        Assert.AreEqual("Easting", c.Axis[0].Name);
                        Assert.AreEqual("Northing", c.Axis[1].Name);
                        Assert.AreEqual("X", c.Axis[0].Abbreviation);
                        Assert.AreEqual("Y", c.Axis[1].Abbreviation);
                        Assert.AreEqual("metre", c.Axis[0].UnitName);
                        Assert.AreEqual("metre", c.Axis[1].UnitName);
                        Assert.AreEqual("EPSG", c.Axis[0].UnitAuthName);
                        Assert.AreEqual("EPSG", c.Axis[1].UnitAuthName);
                        Assert.AreEqual(1.0, c.Axis[0].UnitConversionFactor);
                        Assert.AreEqual(1.0, c.Axis[1].UnitConversionFactor);
                        Assert.AreEqual("9001", c.Axis[0].UnitCode);
                        Assert.AreEqual("9001", c.Axis[1].UnitCode);
                        Assert.AreEqual("east", c.Axis[0].Direction);
                        Assert.AreEqual("north", c.Axis[1].Direction);
                    }
                    using (var e = crs3.GetEllipsoid(pc))
                    {
                        Assert.IsTrue((object)e is Ellipsoid);
                        Assert.AreEqual("Bessel 1841", e.Description);
                        Assert.AreEqual(ProjType.Ellipsoid, e.Type);

                        Assert.AreEqual(6377397.0, Math.Round(e.SemiMajorMetre, 0));
                        Assert.AreEqual(6356079.0, Math.Round(e.SemiMinorMetre, 0));
                        Assert.AreEqual(true, e.IsSemiMinorComputed);
                        Assert.AreEqual(299.0, Math.Round(e.InverseFlattening, 0));
                    }
                    using (var pm = crs3.GetPrimeMeridian(pc))
                    {
                        Assert.AreEqual("Greenwich", pm.Description);
                        Assert.AreEqual(0.0, pm.Longitude);
                        Assert.AreEqual(0.0175, Math.Round(pm.UnitConversionFactor, 4));
                        Assert.AreEqual("degree", pm.UnitName);
                    }
                    using (var co = crs3.GetCoordinateOperation(pc))
                    {
                        Assert.AreEqual("RD New", co.Description);
                        //Assert.AreEqual(0.0, pm.Longitude);
                        //Assert.AreEqual(0.0175, Math.Round(pm.UnitConversionFactor, 4));
                        //Assert.AreEqual("degree", pm.UnitName);
                        double[] r = new double[] { 5, 45 };
                        Assert.AreEqual(111129.0, Math.Round(co.Distance2D(new double[] { co.ToRad(r[0]), co.ToRad(r[1]) }, new double[] { co.ToRad(r[0]), co.ToRad(r[1] + 1) }), 0));
                    }

                    using (var t = CoordinateOperation.Create(crs1, crs2))
                    {
                        Assert.AreEqual("unavailable until proj_trans is called", t.Description);

                        var start = new double[] { t.ToRad(5.0), t.ToRad(52.0) };

                        var r = t.Transform(start);
                        GC.KeepAlive(r);

                        Assert.AreEqual(ProjType.Unknown, t.Type);

                        var s = t.InverseTransform(r);
                        GC.KeepAlive(s);

                        var d2 = crs1.Distance2D(start, s);
                        var d3 = crs1.Distance3D(start, s);

                        Assert.AreEqual(double.PositiveInfinity, d2);
                        Assert.AreEqual(double.PositiveInfinity, d3);


                        using (var c2 = crs1.GetGeodeticCoordinateReferenceSystem(pc))
                        {
                            Assert.AreEqual("WGS 84", c2.Description);
                            Assert.AreEqual(ProjType.Geographic2DCrs, c2.Type);
                            using (var t2 = CoordinateOperation.Create(crs1, c2))
                            {
                                Assert.AreEqual("Inverse of Popular Visualisation Pseudo-Mercator", t2.Description);

                                var d22 = crs1.Distance2D(t2.Transform(start), t2.Transform(s));
                                var d32 = crs1.Distance3D(t2.Transform(start), t2.Transform(s));

                                Assert.AreEqual(double.PositiveInfinity, d22);
                                Assert.AreEqual(double.PositiveInfinity, d32);
                            }
                        }
                    }

                    Assert.AreEqual(ProjType.ProjectedCrs, crs1.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs2.Type);
                }
            }
        }

        [TestMethod]
        public void TestCopenhagen()
        {
            var ctx = new ProjContext();
            var src = CoordinateReferenceSystem.Create(ctx, "EPSG:4326");
            var dst = CoordinateReferenceSystem.Create(ctx, /*"+proj=utm +zone=32 +datum=WGS84" or */ "EPSG:32632");
            Assert.AreEqual("WGS 84", src.Description);
            Assert.AreEqual("WGS 84 / UTM zone 32N", dst.Description);

            var t = CoordinateOperation.Create(ctx, src, dst, null);

            var t2 = t.CreateNormalized();


            var p = t2.Transform(new double[] { 12, 55 });

            Trace.WriteLine($"Easting: {p[0]}, Northing: {p[1]}");

            var r = t2.InverseTransform(p);

            Trace.WriteLine($"Longitude: {r[0]}, Latitude: {r[1]}");

            Assert.AreEqual(double.NaN, t2.Distance2D(r, r));
            Assert.AreEqual(double.PositiveInfinity, src.Distance2D(r, r));
            Assert.AreEqual(double.PositiveInfinity, dst.Distance2D(r, r));

            var tt = CoordinateOperation.Create(ctx, src, src, null);
            Assert.AreEqual("Null geographic offset from WGS 84 to WGS 84", tt.Description);
            Assert.AreEqual(double.NaN, tt.Distance2D(r, r));
            Assert.AreEqual(double.NaN, tt.Distance2D(r, new double[] { r[0], r[1] + 1 }));
            Assert.AreEqual(double.NaN, t2.Distance2D(r, new double[] { r[0], r[1] + 1 }));

            var ss = ctx.Create("+proj=utm +zone=32 +datum=WGS84 +ellps=clrk66");
            Assert.AreEqual(111334.0, Math.Round(ss.Distance2D(new double[] { t.ToRad(r[0]), t.ToRad(r[1]) }, new double[] { t.ToRad(r[0]), t.ToRad(r[1] + 1) }), 0));
        }
    }
}
