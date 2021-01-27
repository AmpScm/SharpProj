using System;
using System.Collections.Generic;
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
        public void CreateAndDestroyContext()
        {
            using (var pc = new ProjContext())
            {
                using (var crs = CoordinateSystem.Create(pc, "+proj=merc +ellps=clrk66 +lat_ts=33"))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Description);
                }

                using (var crs = CoordinateSystem.Create(pc, "proj=merc", "ellps=clrk66", "lat_ts=33"))
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

                using (var crs = CoordinateSystem.Create(pc, "EPSG:25832"))
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
                using (var crs1 = CoordinateSystem.Create(pc, "EPSG:25832"))
                using (var crs2 = CoordinateSystem.Create(pc, "EPSG:25833"))
                {
                    using (var t = CoordinateTransform.Create(crs1, crs2))
                    {
                        Assert.AreEqual("Inverse of UTM zone 32N + UTM zone 33N", t.Description);

                        using(var tr = t.CreateInverse())
                        {
                            Assert.AreEqual("Inverse of UTM zone 33N + UTM zone 32N", tr.Description);
                        }
                    }

                    using (var t = CoordinateTransform.Create(crs2, crs1))
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
                using (var crs1 = CoordinateSystem.Create(pc, "EPSG:3857"))
                using (var crs2 = CoordinateSystem.Create(pc, "EPSG:23095"))
                {
                    Assert.AreEqual(CoordinateSystemType.Unknown, crs1.Type);
                    Assert.AreEqual(-1, crs1.Axis);
                    Assert.AreEqual(CoordinateSystemType.Unknown, crs2.Type);
                    Assert.AreEqual(-1, crs2.Axis);

                    using (var t = CoordinateTransform.Create(crs1, crs2))
                    {
                        Assert.AreEqual("unavailable until proj_trans is called", t.Description);

                        var start = new double[] { 5.0, 52.0 };

                        var r = t.Transform(start);
                        GC.KeepAlive(r);

                        var s = t.InverseTransform(r);
                        GC.KeepAlive(s);

                        var d2 = crs1.Distance2D(start, s);
                        var d3 = crs1.Distance3D(start, s);

                        Assert.AreEqual(double.PositiveInfinity, d2);
                        Assert.AreEqual(double.PositiveInfinity, d3);


                        using (var c2 = crs1.GetGeometricCoordinateSystem())
                        {
                            Assert.AreEqual("WGS 84", c2.Description);
                            Assert.AreEqual(CoordinateSystemType.Unknown, c2.Type);
                            Assert.AreEqual(-1, c2.Axis);
                            using (var t2 = CoordinateTransform.Create(crs1, c2))
                            {
                                Assert.AreEqual("Inverse of Popular Visualisation Pseudo-Mercator", t2.Description);

                                var d22 = crs1.Distance2D(t2.Transform(start), t2.Transform(s));
                                var d32 = crs1.Distance3D(t2.Transform(start), t2.Transform(s));

                                Assert.AreEqual(double.PositiveInfinity, d22);
                                Assert.AreEqual(double.PositiveInfinity, d32);
                            }
                        }
                    }
                }
            }
        }
    }
}
