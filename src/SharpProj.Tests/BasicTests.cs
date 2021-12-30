using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SharpProj.Proj;
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
                Assert.IsTrue(pc.Version >= new Version(8, 1, 0));
                Assert.IsTrue(pc.EpsgVersion >= new Version(10, 15));

                Assert.IsTrue(pc.EsriVersion >= new Version(10, 8, 1));
                Assert.IsTrue(pc.IgnfVersion >= new Version(3, 1, 0));

                Assert.IsTrue(pc.ProjDataVersion >= new Version(1, 7));
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

                using (var crs = CoordinateReferenceSystem.CreateFromEpsg(25832, pc))
                {
                    Assert.AreEqual("ETRS89 / UTM zone 32N", crs.Name);

                    Assert.IsNotNull(crs.Identifiers);
                    Assert.AreEqual(1, crs.Identifiers.Count);
                    Assert.AreEqual("EPSG", crs.Identifiers[0].Authority);
                    Assert.AreEqual("25832", crs.Identifiers[0].Code);
                    Assert.AreEqual("+proj=utm +zone=32 +ellps=GRS80 +units=m +no_defs +type=crs", crs.AsProjString());
                    Assert.AreEqual("+proj=utm +approx +zone=32 +ellps=GRS80 +units=m +no_defs +type=crs", crs.AsProjString(new ProjStringOptions { MultiLine = true, WriteApproxFlag = true }));

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
                        Assert.AreEqual("25832", t.Identifiers[0].Code);
                        Assert.AreEqual("+proj=utm +zone=32 +ellps=GRS80 +units=m +no_defs +type=crs", t.AsProjString());
                        Assert.AreEqual(crs.AsProjString(), t.AsProjString());

                        Assert.IsTrue(t.IsEquivalentTo(crs));
                    }
                    using (var t = ProjObject.Create(crs.AsProjJson()))
                    {
                        Assert.IsTrue(t is CoordinateReferenceSystem);

                        Assert.IsNotNull(t.Identifiers);
                        Assert.AreEqual("EPSG", t.Identifiers[0].Authority);
                        Assert.AreEqual("25832", t.Identifiers[0].Code);
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
                using (var crs1 = CoordinateReferenceSystem.CreateFromEpsg(25832, pc))
                using (var crs2 = CoordinateReferenceSystem.CreateFromEpsg(25833, pc))
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

                        Assert.AreEqual("Inverse of UTM zone 32N", steps[0].Name);
                        using (var tr = steps[0].CreateInverse())
                        {
                            Assert.AreEqual("UTM zone 32N", tr.Name);
                        }

                        using (var tr = t.CreateInverse())
                        {
                            Assert.AreEqual("Inverse of UTM zone 32N + UTM zone 33N", t.Name);
                            Assert.AreEqual("Inverse of UTM zone 33N + UTM zone 32N", tr.Name);


                            Assert.IsTrue(tr.CreateInverse().IsEquivalentTo(t));
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
                using (var crs1 = CoordinateReferenceSystem.CreateFromEpsg(3857, pc))
                using (var crs2 = CoordinateReferenceSystem.CreateFromEpsg(23095, pc))
                using (var crs3 = CoordinateReferenceSystem.CreateFromEpsg(28992, pc))
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
            using (var ctx = new ProjContext())
            {
                var src = CoordinateReferenceSystem.CreateFromEpsg(4326, ctx);
                var dst = CoordinateReferenceSystem.Create(/*"+proj=utm +zone=32 +datum=WGS84" or */ "EPSG:32632", ctx);
                Assert.AreEqual("WGS 84", src.Name);
                Assert.AreEqual("WGS 84 / UTM zone 32N", dst.Name);

                var t = CoordinateTransform.Create(src, dst, ctx);

                var t2 = CoordinateTransform.Create(src.WithAxisNormalized(), dst.WithAxisNormalized(), ctx);


                var p = t2.ApplyReversed(new PPoint(12, 55));

                Trace.WriteLine($"Easting: {p.X}, Northing: {p.Y}");

                var r = t2.ApplyReversed(p);

                Trace.WriteLine($"Longitude: {r.X}, Latitude: {r.Y}");


                var tt = CoordinateTransform.Create(src, src, null);
                Assert.AreEqual("Null geographic offset from WGS 84 to WGS 84", tt.Name);

                var ss = ctx.Create("+proj=utm +zone=32 +datum=WGS84 +ellps=clrk66");
            }
        }

        [TestMethod]
        public void TestAmersfoort()
        {
            using (var c = new ProjContext())
            {
                using (var rd = CoordinateReferenceSystem.CreateFromEpsg(28992, c))
                using (var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326, c))
                using (var google = CoordinateReferenceSystem.CreateFromEpsg(3857, c))
                {
                    var area = rd.UsageArea;

                    Assert.IsNotNull(area);
                    Assert.AreEqual("Netherlands - onshore, including Waddenzee, Dutch Wadden Islands and 12-mile offshore coastal zone.", area.Name);
                    Assert.AreEqual(3.2, area.WestLongitude);
                    Assert.AreEqual(7.22, area.EastLongitude);


                    using (var t = CoordinateTransform.Create(rd, wgs84))
                    {
                        var r = t.Apply(new PPoint(155000, 463000));
                        Assert.AreEqual(new PPoint(52.155, 5.387), r.ToXY(3));

                        Assert.AreEqual(1.0, t.Accuracy);

                        var d = t.RoundTrip(true, 3, new PPoint(155000, 463000));

                        Assert.IsTrue(d < 0.01);
                    }

                    using (var t = CoordinateTransform.Create(rd, google))
                    {
                        var r = t.Apply(new PPoint(155000, 463000));

                        Assert.AreEqual(new PPoint(599701, 6828231), r.ToXY(0));

                        Assert.AreEqual(1, t.Accuracy);
                    }
                }
            }
        }

        [TestMethod]
        public void TestSpain()
        {
            using (var c = new ProjContext() { EnableNetworkConnections = true, LogLevel = ProjLogLevel.Debug })
            {
                c.Log += (_, m) => Debug.WriteLine(m);

                using (var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326, c))
                using (var google = CoordinateReferenceSystem.CreateFromEpsg(3857, c))
                using (var q1 = CoordinateReferenceSystem.CreateFromEpsg(23030, c))
                using (var q2 = CoordinateReferenceSystem.CreateFromEpsg(2062, c))
                {
                    Assert.AreEqual("Engineering survey, topographic mapping.", q1.Scope);
                    Assert.AreEqual("Engineering survey, topographic mapping.", q2.Scope);

                    using (var t = CoordinateTransform.Create(google, wgs84))
                    {
                        var r = t.Apply(new PPoint(-333958.47, 4865942.28));
                        Assert.AreEqual(0, t.GridUsages.Count);
                        Assert.AreEqual(40.0, Math.Round(r[0], 3));
                        Assert.AreEqual(-3, Math.Round(r[1], 3));
                    }

                    using (var t = CoordinateTransform.Create(google, q1))
                    {
                        var r = t.Apply(new PPoint(-333958.47, 4865942.28));
                        Assert.AreEqual(0, t.GridUsages.Count);

                        Assert.AreEqual(500110.0, Math.Round(r[0], 0));
                        Assert.AreEqual(4427965.0, Math.Round(r[1], 0));
                    }

                    using (var t = CoordinateTransform.Create(google, q2))
                    {
                        var r = t.Apply(new PPoint(-333958.47, 4865942.28));
                        Assert.AreEqual(0, t.GridUsages.Count);

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
            using (var itrf2014 = CoordinateReferenceSystem.CreateFromEpsg(7789))
            {
                for (int i = 2000; i < 2200; i++)
                {
                    CoordinateReferenceSystem crs;
                    try
                    {
                        crs = CoordinateReferenceSystem.CreateFromEpsg(i);
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
                            t = CoordinateTransform.Create(crs, itrf2014, new CoordinateTransformOptions { NoBallparkConversions = false, NoDiscardIfMissing = true });
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

                            if (a.Center.HasValues)
                            {
                                PPoint center;
                                try
                                {
                                    center = t.Apply(a.Center);
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

                using (var crsAmersfoort = CoordinateReferenceSystem.CreateFromEpsg(4289, pc)) // Amersfoort
                using (var crsETRS89 = CoordinateReferenceSystem.CreateFromEpsg(4258, pc))
                {
                    // Do it the dumb way
                    using (var t = CoordinateTransform.Create(crsAmersfoort, crsETRS89))
                    {
                        Assert.IsFalse(t is ChooseCoordinateTransform);
                        var r = t.Apply(new PPoint(51, 4, 0));

                        Assert.AreEqual(new PPoint(50.999, 4.000), r.ToXY(3));
                    }

                    // Now, let's enable gridshifts
                    Assert.IsFalse(pc.EnableNetworkConnections);
                    pc.EnableNetworkConnections = true;
                    bool usedHttp = false;
                    pc.Log += (_, x) => { if (x.Contains("https://")) usedHttp = true; };

                    using (var t = CoordinateTransform.Create(crsAmersfoort, crsETRS89))
                    {
                        ChooseCoordinateTransform cl = t as ChooseCoordinateTransform;
                        Assert.IsNotNull(cl);
                        Assert.AreEqual(2, cl.Count);

                        Assert.IsTrue(cl[0].GridUsages.Count > 0);
                        Assert.IsNotNull(cl[0].GridUsages[0].Name);
                        Assert.IsTrue(cl[1].GridUsages.Count == 0);

                        Assert.AreEqual(new PPoint(50.999, 4.0), t.Apply(new PPoint(51, 4)).ToXY(3));
                        var r = t.Apply(new PPoint(51, 4, 0));
                        Assert.IsTrue(usedHttp, "Now http");


                        var r0 = cl[0].Apply(new PPoint(51, 4, 0));
                        usedHttp = false;
                        var r1 = cl[1].Apply(new PPoint(51, 4, 0));
                        Assert.IsFalse(usedHttp, "No http");
                        Assert.IsNotNull(r0);
                        Assert.IsNotNull(r1);

                        Assert.AreEqual(new PPoint(50.999, 4.0), r.ToXY(3));
                        Assert.AreEqual(new PPoint(50.999, 4.0), r1.ToXY(3));

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
            using (var nlOfficial = CoordinateReferenceSystem.CreateFromEpsg(28992))
            using (var wgs84Mercator = CoordinateReferenceSystem.CreateFromEpsg(900913))
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
                pc.EnableNetworkConnections = true;
                using (var epsg7000 = CoordinateTransform.CreateFromEpsg(7000, pc))
                using (var epsg1112 = CoordinateTransform.CreateFromEpsg(1112, pc))
                {
                    Console.WriteLine(epsg7000.SourceCRS?.ToString());
                    Console.WriteLine(epsg7000.TargetCRS?.ToString());
                    Console.WriteLine(epsg1112.SourceCRS?.ToString());
                    Console.WriteLine(epsg1112.TargetCRS?.ToString());
                }
            }
        }

        [TestMethod]
        public void WalkReferenceSystems()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = false })
            {
                foreach (var p in pc.GetCoordinateReferenceSystems())
                {
                    using (var c = p.Create())
                    {
                        if (c.Type == ProjType.TemporalCrs)
                        {
                            Console.WriteLine("!!Temporal!!");
                        }
                        Assert.IsNotNull(p.Authority);
                        Assert.IsNotNull(p.Code);
                        Console.WriteLine($"{p.Authority}:{p.Code} ({p.Type}) / {p.ProjectionName} {c.Name}");
                    }
                }
            }
        }

        [TestMethod]
        public void TestTime()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true, LogLevel = ProjLogLevel.Trace })
            using (var epsg6340 = CoordinateReferenceSystem.CreateFromEpsg(6340, pc)) // [ProjectedCrs] NAD83(2011) / UTM zone 11N
            using (var epsg7665 = CoordinateReferenceSystem.CreateFromEpsg(7665, pc)) // [Geographic3DCrs] WGS 84 (G1762)
            using (var t = CoordinateTransform.Create(epsg6340, epsg7665, new CoordinateTransformOptions { NoBallparkConversions = true, Accuracy = 1 }))
            {
                pc.Log += (_, l) => Console.WriteLine(l);


                var option1 = t.Options()[0];

                // If this is redefined in the EPSG standard, the following test values most likely need updates
                var helmert = option1.ProjOperations().First(x => x.Name == "helmert");
                Assert.AreEqual("helmert", helmert.Name);
                int helmertEpoch = int.Parse(helmert["t_epoch"]);
                Assert.AreEqual(1997, helmertEpoch);

                PPoint testVal = new PPoint(350499.911, 3884807.956, 150.072); // From proj/tests/test_c_api.cpp

                Console.WriteLine("-Default");
                Assert.AreEqual(new PPoint(35.09499807, -118.64016102), t.Apply(testVal).ToXY(8));
                Assert.AreEqual(new PPoint(35.09499807, -118.64016102), t.Apply(testVal.ToXYZ()).ToXY(8));
                Console.WriteLine("-AtEpoch verified");
                Assert.AreEqual(new PPoint(35.09499807, -118.64016102), t.Apply(testVal.WithT(helmertEpoch)).ToXY(8));

                Console.WriteLine("-0");
                Assert.AreEqual(new PPoint(35.09521122, -118.63986605), t.Apply(testVal.WithT(0)).ToXY(8));


                Console.WriteLine("-2000");
                Assert.AreEqual(new PPoint(35.09499775, -118.64016146), t.Apply(testVal.WithT(2000)).ToXY(8));
                Console.WriteLine("-2005");
                Assert.AreEqual(new PPoint(35.09499722, -118.64016220), t.Apply(testVal.WithT(2005)).ToXY(8));
                Console.WriteLine("-2010");
                Assert.AreEqual(new PPoint(35.09499668, -118.64016294), t.Apply(testVal.WithT(2010)).ToXY(8));
                Console.WriteLine("-2020");
                Assert.AreEqual(new PPoint(35.09499562, -118.64016442), t.Apply(testVal.WithT(2020)).ToXY(8));
                Console.WriteLine("-2021");
                Assert.AreEqual(new PPoint(35.09499551, -118.64016457), t.Apply(testVal.WithT(2021)).ToXY(8));
                Console.WriteLine("-2022");
                Assert.AreEqual(new PPoint(35.09499540, -118.64016471), t.Apply(testVal.WithT(2022)).ToXY(8));
                Console.WriteLine("-year0");
                var yearStart = t.Apply(testVal.WithT(DateTime.Now.Year));
                var yearEnd = t.Apply(testVal.WithT(DateTime.Now.Year + 1));
                var today = t.Apply(testVal.WithT(DateTime.Now));
                Assert.IsTrue(yearStart.X >= today.X && today.X >= yearEnd.X);
                Assert.IsTrue(yearStart.Y >= today.Y && today.Y >= yearEnd.Y);
                Assert.AreEqual(new PPoint(35.09521122, -118.63986605), t.Apply(testVal.WithT(0)).ToXY(8));
                Console.WriteLine("-year PositiveInf1");
                Assert.AreEqual(new PPoint(35.09499807, -118.64016102), t.Apply(testVal.WithT(double.PositiveInfinity)).ToXY(8));

                using (CoordinateTransform ct = CoordinateTransform.Create(t.Steps()[1].AsProjString()))
                {
                    GC.KeepAlive(ct);
                }
            }
        }

        [TestMethod]
        public void TestAustralia()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true })
            using (var australia = CoordinateReferenceSystem.CreateFromEpsg(7843, pc))
            using (var epsg7665 = CoordinateReferenceSystem.CreateFromEpsg(7665, pc))
            using (var t = CoordinateTransform.Create(australia, epsg7665, new CoordinateTransformOptions { IntermediateCrsUsage = IntermediateCrsUsage.Always, NoDiscardIfMissing = true }))
            {
                var ua = australia.UsageArea;
                var r = t.Apply(new PPoint(ua.CenterX, ua.CenterY));
                var r2 = t.Apply(new PPoint(ua.CenterX, ua.CenterY) { T = 2000 });
                var r3 = t.Apply(new PPoint(ua.CenterX, ua.CenterY) { T = 2020 });

                Assert.AreNotEqual(r2.ToXY(), r3.ToXY());
                GC.KeepAlive(t);
            }
        }

        [TestMethod]
        [TestCategory("NeedsNetwork")]
        public void TestNewZealand_defmodel()
        {
            // New Zealand (EPSG:2105) uses a defmodel to correct specific parts at specific points in time (new in proj 7.1.0)
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true, LogLevel = ProjLogLevel.Debug })
            {
                pc.Log += (_, m) => Console.WriteLine(m);
                using (var newZealandNorthIsland = CoordinateReferenceSystem.CreateFromEpsg(2105, pc))
                using (var itrf2014 = CoordinateReferenceSystem.CreateFromEpsg(9000, pc))
                {
                    var centerPoint = newZealandNorthIsland.UsageArea.Center;


                    using (var t = CoordinateTransform.Create(newZealandNorthIsland, itrf2014))
                    {
                        Assert.IsNotNull(t.ProjOperations().SingleOrDefault(x => x.Name == "defmodel"), "New Zealand has defmodel step");
                        Assert.IsNotNull(t.ProjOperations().SingleOrDefault(x => x.Name == "helmert"), "New Zealand has helmert step");

                        PPoint rLast = t.Apply(centerPoint.WithT(1999));

                        for (int year = 2000; year < 2020; year++)
                        {
                            PPoint rNow = t.Apply(centerPoint.WithT(year));

                            Assert.AreNotEqual(rLast.ToXY(), rNow.ToXY());
                            double distance = itrf2014.DistanceTransform.GeoDistance(rNow, rLast);

                            Assert.IsTrue(distance >= 0.04, "Distance > 4cm / year. check grid file acces (e.g. network) if this fails");

                            if (year == 2017)
                                Assert.IsTrue(distance >= 0.045, "Huge step in 2017");
                            else
                                Assert.IsTrue(distance < 0.045, "Small step when not 2017");
                            Console.WriteLine($"{year}: {Math.Round(distance, 5)} m");
                            rLast = rNow;
                        }
                    }
                }
            }
        }

        [TestMethod]
        public void TestFinland_tinshift()
        {
            // Finland uses a tinshift when converting EPSG:2393 to EPSG 3067
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true, LogLevel = ProjLogLevel.Debug })
            {
                pc.Log += (_, m) => Console.WriteLine(m);
                using (var finland = CoordinateReferenceSystem.CreateFromEpsg(2393, pc))
                using (var etrs89_finland = CoordinateReferenceSystem.CreateFromEpsg(3067, pc))
                {
                    var centerPoint = finland.UsageArea.Center;


                    using (var tBase = CoordinateTransform.Create(finland, etrs89_finland) as ChooseCoordinateTransform)
                    {
                        Assert.IsNotNull(tBase[0].ProjOperations().SingleOrDefault(x => x.Name == "tinshift"), "Finland option 1 has tinshift step");
                        Assert.IsNotNull(tBase[1].ProjOperations().SingleOrDefault(x => x.Name == "helmert"), "Finland option 2 has helmert step");

                        foreach (var t in tBase)
                        {
                            PPoint rLast = t.Apply(centerPoint);
                        }
                    }
                }
            }
        }

        [TestMethod]
        public void TestFrance()
        {
            using (var fr = CoordinateReferenceSystem.CreateFromEpsg(2154))
            using (var crs1 = CoordinateReferenceSystem.CreateFromEpsg(3857))
            using (var t = CoordinateTransform.Create(fr, crs1))
            {

                GC.KeepAlive(fr);
            }
        }


        [TestMethod]
        public void WalkAllOperations()
        {
            var v = ProjOperationDefinition.All;
            Assert.IsNotNull(v);
            Assert.AreNotEqual(0, v.Count, "Has ops");
            foreach (var m in v)
            {
                Assert.AreNotEqual("", m.Name);
                Assert.AreNotEqual("", m.Title);
                GC.KeepAlive(m.Details);
            }
        }

        [TestMethod]
        public void Epsg3851Bounds()
        {
            using (CoordinateReferenceSystem crs3851 = CoordinateReferenceSystem.CreateFromEpsg(3851))
            {
                var ua = crs3851.UsageArea;

                Assert.IsTrue(ua.MinX > 0);
            }
        }

        [TestMethod]
        public void MissingProjections()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true })
            {
                int missing = 0;
                foreach (var i in pc.GetCoordinateReferenceSystems(new CoordinateReferenceSystemFilter { Authority = "ESRI" }).Where(x => int.TryParse(x.Code, out var c) && c >= 53000 && c < 55000))
                {
                    using (var crs = i.Create())
                    {
                        if (RunMissing(crs, i.ToString()))
                            missing++;
                    }
                }
                Assert.AreEqual(13, missing, "Expected that this many ESRI test projections between 53000-54999 fail. Fix assumption");

                foreach (int epsg in new[] {2218, 2221, 2296, 2299, 2301, 2303, 2304, 2305, 2306, 2307, 2963, 2985, 2986, 3052,
                    3053, 3144, 3145, 3173, 5017, 5224, 5225, 5515, 5516, 22300, 22700, 32600, 32700})
                {
                    using (var crs = CoordinateReferenceSystem.CreateFromEpsg(epsg, pc))
                    {
                        if (!RunMissing(crs, crs.Name))
                        {
                            Assert.Inconclusive($"{crs.Identifiers[0]}: Expected to be failing. Fix test assumptions");
                        }
                    }
                }

                foreach (int esri in new[] { 53044, 102299, 102460 })
                {
                    using (var crs = CoordinateReferenceSystem.CreateFromDatabase("ESRI", esri, pc))
                    {
                        if (!RunMissing(crs, crs.Name))
                        {
                            Assert.Inconclusive($"{crs.Identifiers[0]}: Expected to be failing. Fix test assumptions");
                        }
                    }
                }
            }
        }

        bool RunMissing(CoordinateReferenceSystem crs, string i)
        {
            if (crs.DistanceTransform == null)
                Console.WriteLine($"{crs.Identifiers[0]}: Distance transform Null for {i}");
            else if (!crs.DistanceTransform.IsAvailable)
                Console.WriteLine($"{crs.Identifiers[0]}: Distance transform not available for {i}");
            else if (!crs.DistanceTransform.HasInverse)
                Console.WriteLine($"{crs.Identifiers[0]}: Distance transform not reversable for {i}");
            else
                return false;

            string projection = null;
            if (crs.DistanceTransform != null)
            {
                var p = crs.DistanceTransform.Options().SelectMany(x => x.ProjOperations()).FirstOrDefault(x => x.Type == ProjOperationType.Projection);

                if (projection != null)
                    projection = ProjOperationDefinition.All[p.Name].Title;

                if (projection == null)
                {
                    string method = crs.DistanceTransform.AsWellKnownText()?.Split('\n').Select(x => x.Trim()).FirstOrDefault(x => x.StartsWith("METHOD["));

                    if (method != null)
                    {
                        projection = method.Substring("METHOD[".Length).Trim(',', '\"', ']');
                    }
                }
            }
            if (projection != null)
                Console.WriteLine($" - Projection: {projection}");

            return true;
        }

        [TestMethod]
        public void GnomoricBounds()
        {
            foreach (int esri in new[] { 102034, 102036 })
            {
                using (var crs = CoordinateReferenceSystem.CreateFromDatabase("ESRI", esri).WithAxisNormalized())
                {
                    var ua = crs.UsageArea;
                    var dt = crs.DistanceTransform;

                    List<PPoint> points = new List<PPoint>();

                    for (double x = ua.WestLongitude; x <= ua.EastLongitude; x+=1)
                    {
                        for (double y = Math.Min(ua.SouthLatitude, ua.NorthLatitude); y <= Math.Max(ua.SouthLatitude, ua.NorthLatitude); y+=1)
                        {
                            points.Add(new PPoint(x, y));
                        }
                    }

                    List<PPoint> pTransformed = new List<PPoint>(points.Select(x => dt.ApplyReversed(x)));

                    double xMin = pTransformed.Where(x => x.HasValues).Min(x => x.X);
                    double xMax = pTransformed.Where(x => x.HasValues).Max(x => x.X);
                    double yMin = pTransformed.Where(x => x.HasValues).Min(x => x.Y);
                    double yMax = pTransformed.Where(x => x.HasValues).Max(x => x.Y);

                    int nXMin = pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.X == xMin));
                    int nXMax = pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.X == xMax));
                    int nYMin = pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.Y == yMin));
                    int nYMax = pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.Y == yMax));

                    Console.WriteLine($"xMin: {xMin} at {points[nXMin]} -> {pTransformed[nXMin]}");
                    Console.WriteLine($"xMax: {xMax} at {points[nXMax]} -> {pTransformed[nXMax]}");
                    Console.WriteLine($"yMin: {yMin} at {points[nYMin]} -> {pTransformed[nYMin]}");
                    Console.WriteLine($"yMax: {yMax} at {points[nYMax]} -> {pTransformed[nYMax]}");

                    ProjRange usageRange = new ProjRange(crs.UsageArea);
                    ProjRange range = new ProjRange(xMin, yMin, xMax, yMax);

                    //Assert.AreEqual(usageRange, range);
                    //
                    //Assert.AreEqual(ua.MinX, xMin, $"XMin out of bounds for {points[pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.X == xMin))]}");
                    //Assert.AreEqual(ua.MaxX, xMax, $"XMax out of bounds for {points[pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.X == xMax))]}");
                    //Assert.AreEqual(ua.MinY, yMin, $"YMin out of bounds for {points[pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.X == yMin))]}");
                    //Assert.AreEqual(ua.MaxY, yMax, $"YMax out of bounds for {points[pTransformed.IndexOf(pTransformed.FirstOrDefault(x => x.X == yMax))]}");
                }
                Console.WriteLine("---");
            }
        }
    }
}
