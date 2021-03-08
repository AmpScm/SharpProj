using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SharpProj.Proj;
using SharpProj.Testing;
using PJ = SharpProj.CoordinateTransform;

namespace SharpProj.Tests
{
    [TestClass]
    public class BasicTests
    {
        public TestContext TestContext { get; set; }
        [TestMethod]
        public void BadInit()
        {
            using (var pc = new ProjContext())
            {
                try
                {
                    using (var crs = CoordinateReferenceSystem.Create("!1@2#3$4%5^6&7*8(9)0", pc))
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
        public void EpsgVersionTest()
        {
            using (var pc = new ProjContext())
            {
                Assert.IsTrue(pc.Version >= new Version(8, 0, 0));
                Assert.IsTrue(pc.EpsgVersion >= new Version(10, 15));

                Assert.IsTrue(pc.EsriVersion >= new Version(10, 8, 1));
                Assert.IsTrue(pc.IgnfVersion >= new Version(3, 1, 0));
            }
        }

        [TestMethod]
        public void CreateAndDestroyContext()
        {
            using (var pc = new ProjContext())
            {
                using (var crs = pc.Create("+proj=merc +ellps=clrk66 +lat_ts=33"))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Name);

                    if (crs is CoordinateTransform cob)
                    {
                        Assert.IsNull(crs.Identifiers);

                        var src = cob.SourceCRS;
                        var dst = cob.TargetCRS;

                        Assert.IsNull(src);
                        Assert.IsNull(dst);

                        //Assert.AreEqual(1, crs.Identifiers.Count);

                    }
                    else
                        Assert.Fail();

                    Assert.AreEqual(ProjType.OtherCoordinateTransform, crs.Type);
                    string expected =
@"{
  ""$schema"": ""https://proj.org/schemas/v0.2/projjson.schema.json"",
  ""type"": ""Conversion"",
  ""name"": ""PROJ-based coordinate operation"",
  ""method"": {
    ""name"": ""PROJ-based operation method: +proj=merc +ellps=clrk66 +lat_ts=33""
  }
}".Replace("\r", "");
                    Assert.AreEqual(expected, crs.AsProjJson());
                    Assert.AreEqual("proj=merc ellps=clrk66 lat_ts=33", crs.Definition);
                    Assert.AreEqual("+proj=merc +ellps=clrk66 +lat_ts=33", crs.AsProjString());
                }

                using (var crs = pc.Create(new string[] { "proj=merc", "ellps=clrk66", "lat_ts=33" }))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Name);
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

                using (var crs = CoordinateReferenceSystem.Create("EPSG:25832", pc))
                {
                    Assert.AreEqual("ETRS89 / UTM zone 32N", crs.Name);

                    Assert.IsNotNull(crs.Identifiers);
                    Assert.AreEqual(1, crs.Identifiers.Count);
                    Assert.AreEqual("EPSG", crs.Identifiers[0].Authority);
                    Assert.AreEqual("25832", crs.Identifiers[0].Name);
                    Assert.AreEqual("+proj=utm +zone=32 +ellps=GRS80 +units=m +no_defs +type=crs", crs.AsProjString());

                    using (var t = ProjObject.Create(crs.AsProjString()))
                    {
                        Assert.IsTrue(t is CoordinateReferenceSystem);

                        Assert.IsNull(t.Identifiers);

                        Assert.AreEqual(crs.AsProjString(), t.AsProjString());
                    }

                    using (var t = ProjObject.Create(crs.AsWellKnownText()))
                    {
                        Assert.IsTrue(t is CoordinateReferenceSystem);

                        Assert.IsNotNull(t.Identifiers);
                        Assert.AreEqual("EPSG", t.Identifiers[0].Authority);
                        Assert.AreEqual("25832", t.Identifiers[0].Name);
                        Assert.AreEqual("+proj=utm +zone=32 +ellps=GRS80 +units=m +no_defs +type=crs", t.AsProjString());
                        Assert.AreEqual(crs.AsProjString(), t.AsProjString());

                        Assert.IsTrue(t.IsEquivalentTo(crs));
                    }
                    using (var t = ProjObject.Create(crs.AsProjJson()))
                    {
                        Assert.IsTrue(t is CoordinateReferenceSystem);

                        Assert.IsNotNull(t.Identifiers);
                        Assert.AreEqual("EPSG", t.Identifiers[0].Authority);
                        Assert.AreEqual("25832", t.Identifiers[0].Name);
                        Assert.AreEqual("+proj=utm +zone=32 +ellps=GRS80 +units=m +no_defs +type=crs", t.AsProjString());
                        Assert.AreEqual(crs.AsProjString(), t.AsProjString());
                        Assert.IsTrue(t.IsEquivalentTo(crs));
                    }


                }
            }
        }

        [TestMethod]
        public void CreateBasicTransform()
        {
            using (var pc = new ProjContext())
            {
                using (var crs1 = CoordinateReferenceSystem.Create("EPSG:25832", pc))
                using (var crs2 = CoordinateReferenceSystem.Create("EPSG:25833", pc))
                {
                    Assert.AreEqual(ProjType.ProjectedCrs, crs1.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs2.Type);

                    using (var t = CoordinateTransform.Create(crs1, crs2))
                    {
                        CoordinateTransformList steps = t as CoordinateTransformList;
                        Assert.IsNotNull(steps);

                        Assert.AreEqual(2, steps.Count);
                        Assert.AreEqual("Inverse of Transverse Mercator", steps[0].MethodName);
                        Assert.AreEqual("Transverse Mercator", steps[1].MethodName);

                        Assert.AreEqual("Inverse of UTM zone 32N + UTM zone 33N", t.Name);

                        using (var tr = t.CreateInverse())
                        {
                            Assert.AreEqual("Inverse of UTM zone 33N + UTM zone 32N", tr.Name);
                        }
                    }

                    using (var t = CoordinateTransform.Create(crs2, crs1))
                    {
                        Assert.AreEqual("Inverse of UTM zone 33N + UTM zone 32N", t.Name);
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
                using (var crs1 = CoordinateReferenceSystem.Create("EPSG:3857", pc))
                using (var crs2 = CoordinateReferenceSystem.Create("EPSG:23095", pc))
                using (var crs3 = CoordinateReferenceSystem.Create("EPSG:28992", pc))
                {
                    Assert.AreEqual("WGS 84 / Pseudo-Mercator", crs1.Name);
                    Assert.AreEqual("ED50 / TM 5 NE", crs2.Name);
                    Assert.AreEqual("Amersfoort / RD New", crs3.Name);

                    Assert.AreEqual(ProjType.ProjectedCrs, crs1.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs2.Type);
                    Assert.AreEqual(ProjType.ProjectedCrs, crs3.Type);

                    Assert.IsNotNull(crs3.GeodeticCRS);
                    Assert.AreEqual("Amersfoort", crs3.GeodeticCRS.Name);
                    Assert.AreEqual(ProjType.Geographic2DCrs, crs3.GeodeticCRS.Type);

                    Assert.IsNotNull(crs3.Datum);
                    Assert.AreEqual("Amersfoort", crs3.Datum.Name);
                    Assert.AreEqual(ProjType.GeodeticReferenceFrame, crs3.Datum.Type);
                    Assert.IsFalse(crs3.Datum is Proj.DatumList);
                    Assert.AreEqual(null, crs3.CoordinateSystem.Name);
                    Assert.AreEqual(ProjType.CoordinateSystem, crs3.CoordinateSystem.Type);
                    Assert.AreEqual(CoordinateSystemType.Cartesian, crs3.CoordinateSystem.CoordinateSystemType);
                    Assert.AreEqual(2, crs3.CoordinateSystem.Axis.Count);

                    Assert.AreEqual("Easting", crs3.CoordinateSystem.Axis[0].Name);
                    Assert.AreEqual("Northing", crs3.CoordinateSystem.Axis[1].Name);
                    Assert.AreEqual("X", crs3.CoordinateSystem.Axis[0].Abbreviation);
                    Assert.AreEqual("Y", crs3.CoordinateSystem.Axis[1].Abbreviation);
                    Assert.AreEqual("metre", crs3.CoordinateSystem.Axis[0].UnitName);
                    Assert.AreEqual("metre", crs3.CoordinateSystem.Axis[1].UnitName);
                    Assert.AreEqual("EPSG", crs3.CoordinateSystem.Axis[0].UnitAuthName);
                    Assert.AreEqual("EPSG", crs3.CoordinateSystem.Axis[1].UnitAuthName);
                    Assert.AreEqual(1.0, crs3.CoordinateSystem.Axis[0].UnitConversionFactor);
                    Assert.AreEqual(1.0, crs3.CoordinateSystem.Axis[1].UnitConversionFactor);
                    Assert.AreEqual("9001", crs3.CoordinateSystem.Axis[0].UnitCode);
                    Assert.AreEqual("9001", crs3.CoordinateSystem.Axis[1].UnitCode);
                    Assert.AreEqual("east", crs3.CoordinateSystem.Axis[0].Direction);
                    Assert.AreEqual("north", crs3.CoordinateSystem.Axis[1].Direction);

                    Assert.AreEqual("Bessel 1841", crs3.Ellipsoid.Name);
                    Assert.AreEqual(ProjType.Ellipsoid, crs3.Ellipsoid.Type);

                    Assert.AreEqual(6377397.0, Math.Round(crs3.Ellipsoid.SemiMajorMetre, 0));
                    Assert.AreEqual(6356079.0, Math.Round(crs3.Ellipsoid.SemiMinorMetre, 0));
                    Assert.AreEqual(true, crs3.Ellipsoid.IsSemiMinorComputed);
                    Assert.AreEqual(299.0, Math.Round(crs3.Ellipsoid.InverseFlattening, 0));

                    Assert.AreEqual("Greenwich", crs3.PrimeMeridian.Name);
                    Assert.AreEqual(0.0, crs3.PrimeMeridian.Longitude);
                    Assert.AreEqual(0.0175, Math.Round(crs3.PrimeMeridian.UnitConversionFactor, 4));
                    Assert.AreEqual("degree", crs3.PrimeMeridian.UnitName);

                    using (var t = CoordinateTransform.Create(crs1, crs2))
                    {
                        Assert.IsTrue(t is ChooseCoordinateTransform);

                        {
                            var c2 = crs1.GeodeticCRS;
                            Assert.AreEqual("WGS 84", c2.Name);
                            Assert.AreEqual(ProjType.Geographic2DCrs, c2.Type);
                            using (var t2 = CoordinateTransform.Create(crs1, c2))
                            {
                                Assert.AreEqual("Inverse of Popular Visualisation Pseudo-Mercator", t2.Name);
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
            var src = CoordinateReferenceSystem.Create("EPSG:4326", ctx);
            var dst = CoordinateReferenceSystem.Create(/*"+proj=utm +zone=32 +datum=WGS84" or */ "EPSG:32632", ctx);
            Assert.AreEqual("WGS 84", src.Name);
            Assert.AreEqual("WGS 84 / UTM zone 32N", dst.Name);

            var t = CoordinateTransform.Create(src, dst, ctx);

            var t2 = CoordinateTransform.Create(src.WithAxisNormalized(), dst.WithAxisNormalized(), ctx);


            var p = t2.ApplyReversed(new double[] { 12, 55 });

            Trace.WriteLine($"Easting: {p[0]}, Northing: {p[1]}");

            var r = t2.ApplyReversed(p);

            Trace.WriteLine($"Longitude: {r[0]}, Latitude: {r[1]}");


            var tt = CoordinateTransform.Create(src, src, null);
            Assert.AreEqual("Null geographic offset from WGS 84 to WGS 84", tt.Name);

            var ss = ctx.Create("+proj=utm +zone=32 +datum=WGS84 +ellps=clrk66");
        }

        [TestMethod]
        public void TestAmersfoort()
        {
            using (var c = new ProjContext())
            {
                using (var rd = CoordinateReferenceSystem.Create("EPSG:28992", c))
                using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326", c))
                using (var google = CoordinateReferenceSystem.Create("EPSG:3857", c))
                {
                    var area = rd.UsageArea;

                    Assert.IsNotNull(area);
                    Assert.AreEqual("Netherlands - onshore, including Waddenzee, Dutch Wadden Islands and 12-mile offshore coastal zone.", area.Name);
                    Assert.AreEqual(3.2, area.WestLongitude);
                    Assert.AreEqual(7.22, area.EastLongitude);


                    using (var t = CoordinateTransform.Create(rd, wgs84))
                    {
                        var r = t.Apply(155000, 463000);
                        Assert.AreEqual(52.155, Math.Round(r[0], 3));
                        Assert.AreEqual(5.387, Math.Round(r[1], 3));

                        Assert.AreEqual(1, t.Accuraracy);
                    }

                    using (var t = CoordinateTransform.Create(rd, google))
                    {
                        var r = t.Apply(155000, 463000);

                        Assert.AreEqual(599701.0, Math.Round(r[0], 0));
                        Assert.AreEqual(6828231.0, Math.Round(r[1], 0));

                        Assert.AreEqual(1, t.Accuraracy);
                    }
                }
            }
        }

        [TestMethod]
        public void TestSpain()
        {
            using (var c = new ProjContext())
            {
                c.AllowNetworkConnections = true;
                c.LogLevel = ProjLogLevel.Trace;
                c.Log += (_, m) => Debug.WriteLine(m);
                using (var wgs84 = CoordinateReferenceSystem.CreateFromDatabase("EPSG", "4326", c))
                using (var google = CoordinateReferenceSystem.Create("EPSG:3857", c))
                using (var q1 = CoordinateReferenceSystem.Create("EPSG:23030", c))
                using (var q2 = CoordinateReferenceSystem.Create("EPSG:2062", c))
                {
                    Assert.AreEqual("Engineering survey, topographic mapping.", q1.Scope);
                    Assert.AreEqual("Engineering survey, topographic mapping.", q2.Scope);

                    using (var t = CoordinateTransform.Create(google, wgs84))
                    {
                        var r = t.Apply(-333958.47, 4865942.28);
                        Assert.AreEqual(0, t.GridUsageCount);
                        Assert.AreEqual(40.0, Math.Round(r[0], 3));
                        Assert.AreEqual(-3, Math.Round(r[1], 3));
                    }

                    using (var t = CoordinateTransform.Create(google, q1))
                    {
                        var r = t.Apply(-333958.47, 4865942.28);
                        Assert.AreEqual(0, t.GridUsageCount);

                        Assert.AreEqual(500110.0, Math.Round(r[0], 0));
                        Assert.AreEqual(4427965.0, Math.Round(r[1], 0));
                    }

                    using (var t = CoordinateTransform.Create(google, q2))
                    {
                        var r = t.Apply(-333958.47, 4865942.28);
                        Assert.AreEqual(0, t.GridUsageCount);

                        Assert.AreEqual(658629.5, Math.Round(r[0], 1));
                        Assert.AreEqual(600226.1, Math.Round(r[1], 1));
                    }
                }
            }
        }

        [TestMethod]
        public void FewEpsg()
        {
            bool hasDeprecated = false;
            using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326"))
            {
                for (int i = 2000; i < 2200; i++)
                {
                    CoordinateReferenceSystem crs;
                    try
                    {
                        crs = CoordinateReferenceSystem.Create($"EPSG:{i}");
                    }
                    catch (ProjException)
                    {
                        Trace.WriteLine($"Not supported: {i}");
                        //Assert.IsTrue(new int[] { 0, 1, 2,3 }.Contains(i), $"EPSG {i} not supported");
                        continue;
                    }

                    using (crs)
                    {
                        if (crs.IsDeprecated)
                        {
                            hasDeprecated = true;
                            continue;
                        }

                        CoordinateTransform t;
                        try
                        {
                            t = CoordinateTransform.Create(crs, wgs84, new CoordinateTransformOptions { NoBallparkConversions = false });
                        }
                        catch (ProjException)
                        {
                            Trace.WriteLine($"Not convertible: {i}");
                            //Assert.IsTrue(new int[] { 0, 1, 2,3 }.Contains(i), $"EPSG {i} not supported");
                            continue;
                        }


                        using (t)
                        {
                            var a = crs.UsageArea;

                            PPoint center;
                            try
                            {
                                center = t.Apply(new PPoint((a.MinX + a.MaxX) / 2.0, (a.MinX + a.MaxY) / 2.0));
                            }
                            catch (ProjException)
                            {
                                center = new PPoint();
                            }


                            if (center.HasValues && t.HasInverse && !(t is ChooseCoordinateTransform))
                            {
                                PPoint ret = t.ApplyReversed(center);
                            }
                        }
                    }
                }
            }
            Assert.IsTrue(hasDeprecated, "Found deprecated");
        }

        [TestMethod]
        public void WithGrid()
        {
            using (var pc = new ProjContext())
            {
                // Don't use old cache
                pc.SetGridCache(true, Path.Combine(TestContext.TestResultsDirectory, "proj.cache"), 300, 3600 * 24);
                pc.LogLevel = ProjLogLevel.Trace;

                using (var crsAmersfoort = CoordinateReferenceSystem.Create(@"EPSG:4289", pc)) // Amersfoort
                using (var crsETRS89 = CoordinateReferenceSystem.Create(@"EPSG:4258", pc))
                {
                    // Do it the dumb way
                    using (var t = CoordinateTransform.Create(crsAmersfoort, crsETRS89))
                    {
                        Assert.IsFalse(t is ChooseCoordinateTransform);
                        var r = t.Apply(51, 4, 0);

                        Assert.AreEqual(50.999, Math.Round(r[0], 3));
                        Assert.AreEqual(4.0, Math.Round(r[1], 3));
                    }

                    // Now, let's enable gridshifts
                    Assert.IsFalse(pc.AllowNetworkConnections);
                    pc.AllowNetworkConnections = true;
                    pc.EndpointUrl = ProjContext.DefaultEndpointUrl;// "https://cdn.proj.org";
                    bool usedHttp = false;
                    pc.Log += (_, x) => { if (x.Contains("https://")) usedHttp = true; };

                    using (var t = CoordinateTransform.Create(crsAmersfoort, crsETRS89))
                    {
                        ChooseCoordinateTransform cl = t as ChooseCoordinateTransform;
                        Assert.IsNotNull(cl);
                        Assert.AreEqual(2, cl.Count);

                        Assert.IsTrue(cl[0].GridUsageCount > 0);
                        Assert.IsTrue(cl[1].GridUsageCount == 0);

                        Assert.AreEqual(new PPoint(50.999, 4.0), t.Apply(new PPoint(51, 4)).RoundXY(3));
                        var r = t.Apply(51, 4, 0);
                        Assert.IsTrue(usedHttp, "Now http");


                        var r0 = cl[0].Apply(51, 4, 0);
                        usedHttp = false;
                        var r1 = cl[1].Apply(51, 4, 0);
                        Assert.IsFalse(usedHttp, "No http");
                        Assert.IsNotNull(r0);
                        Assert.IsNotNull(r1);

                        Assert.AreEqual(50.999, Math.Round(r0[0], 3));
                        Assert.AreEqual(4.0, Math.Round(r0[1], 3));

                        Assert.AreEqual(50.999, Math.Round(r1[0], 3));
                        Assert.AreEqual(4.0, Math.Round(r1[1], 3));

                        Assert.IsNotNull(cl[0].MethodName);
                    }
                }
            }
        }

        [TestMethod]
        public void DegRadTests()
        {
            var PI = Math.PI;

            // If this fails, something is severely broken... 
            // Or we are using some different math implementation with minor difference

            // The code was moved to .Net to allow inlining by the CLR
            Assert.AreEqual(0.0 * PI / 2.0, PJ.ToRad(0));
            Assert.AreEqual(1.0 * PI / 2.0, PJ.ToRad(90));
            Assert.AreEqual(2.0 * PI / 2.0, PJ.ToRad(180));
            Assert.AreEqual(3.0 * PI / 2.0, PJ.ToRad(270));


            Assert.AreEqual(0.0, PJ.ToDeg(0.0 * PI / 2.0));
            Assert.AreEqual(90.0, PJ.ToDeg(1.0 * PI / 2.0));
            Assert.AreEqual(180.0, PJ.ToDeg(2.0 * PI / 2.0));
            Assert.AreEqual(270.0, PJ.ToDeg(3.0 * PI / 2.0));
        }

        [TestMethod]
        public void WalkAmersfoort()
        {
            using (var nlOfficial = CoordinateReferenceSystem.Create("EPSG:28992"))
            using (var wgs84Mercator = CoordinateReferenceSystem.Create("EPSG:900913"))
            using (var t = CoordinateTransform.Create(nlOfficial, wgs84Mercator))
            {
                for (double x = nlOfficial.UsageArea.MinX; x < nlOfficial.UsageArea.MaxX; x += 5000 /* Meter */)
                    for (double y = nlOfficial.UsageArea.MinY; y < nlOfficial.UsageArea.MaxY; y += 5000 /* Meter */)
                    {
                        t.Apply(new PPoint(x, y));
                    }
            }
        }

        [TestMethod]
        public void TestNLTransform()
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.AllowNetworkConnections = true;
                using (var epsg7000 = CoordinateTransform.CreateFromDatabase("EPSG", "7000", pc))
                using (var epsg1112 = CoordinateTransform.CreateFromDatabase("EPSG", "1112", pc))
                {
                    Console.WriteLine(epsg7000.SourceCRS?.ToString());
                    Console.WriteLine(epsg7000.TargetCRS?.ToString());
                    Console.WriteLine(epsg1112.SourceCRS?.ToString());
                    Console.WriteLine(epsg1112.TargetCRS?.ToString());
                }

                foreach(var p in pc.GetCoordinateReferenceSystems())
                {
                    Assert.IsNotNull(p.Authority);
                    Assert.IsNotNull(p.Code);
                    Console.WriteLine($"{p.Authority}:{p.Code} ({p.Type}) / {p.ProjectionName}");
                }
            }
        }
    }
}
