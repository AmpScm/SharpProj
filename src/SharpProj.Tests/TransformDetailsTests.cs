using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SharpProj.Proj;
using ProjId = SharpProj.Proj.Identifier;

namespace SharpProj.Tests
{
    [TestClass]
    public class TransformDetailsTests
    {
        public TestContext TestContext { get; set; }

        [TestMethod]
        [DataRow(true), DataRow(false)]
        public void Details_EPSG2964_to_EPSG_4326(bool useNetwork)
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.EnableNetworkConnections = useNetwork;

                using (CoordinateReferenceSystem crsFrom = CoordinateReferenceSystem.CreateFromEpsg(2964, pc))
                using (CoordinateReferenceSystem crsTo = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
                {
                    Assert.AreEqual("NAD27 / Alaska Albers", crsFrom.Name);
                    Assert.AreEqual(new ProjId("EPSG", 2964), crsFrom.Identifier);
                    Assert.AreEqual("WGS 84", crsTo.Name);
                    Assert.AreEqual(new ProjId("EPSG", 4326), crsTo.Identifier);

                    using (var ct = CoordinateTransform.Create(crsFrom, crsTo))
                    {
                        Assert.AreEqual("<choose-coordinate-transform>", ct.Name);

                        Assert.AreEqual("NAD27 / Alaska Albers", ct.SourceCRS.Name);
                        Assert.AreEqual("WGS 84", ct.TargetCRS.Name);
                        Assert.AreEqual(new ProjId("EPSG", 2964), ct.SourceCRS.Identifier);
                        Assert.AreEqual(new ProjId("EPSG", 4326), ct.TargetCRS.Identifier);
                        Assert.IsTrue(crsFrom.IsEquivalentTo(ct.SourceCRS));
                        Assert.IsTrue(crsTo.IsEquivalentTo(ct.TargetCRS));

                        // Returns best match
                        Assert.AreEqual(useNetwork ? "United States (USA) - Alaska including EEZ." : "United States (USA) - Alaska mainland.", ct.UsageArea.Name);
                        Assert.IsNull(ct.Scope);

                        var cct = ct as ChooseCoordinateTransform;
                        Assert.IsNotNull("Have Choose coordinate transform");

                        Assert.AreEqual(useNetwork ? 10 : 8, cct.Count);
                        Assert.AreEqual(cct.First().UsageArea.Name, ct.UsageArea.Name);

                        foreach (var transform in cct)
                        {
                            Assert.IsTrue(crsFrom.IsEquivalentTo(transform.SourceCRS));
                            Assert.IsTrue(crsTo.IsEquivalentTo(transform.TargetCRS));

                            TestContext.WriteLine($"{transform.Name} - {transform.Accuracy} m - {transform.UsageArea?.Name}");

                            string ids = transform.Identifier?.ToString();
                            string scopes = transform.Scope;
                            if (ids == null && transform is CoordinateTransformList ctl)
                            {
                                ids = string.Join(", ", ctl.Where(x => x.Identifiers != null).SelectMany(x => x.Identifiers));
                                scopes = string.Join(", ", ctl.Select(x => x.Scope).Where(x => x is not null).Distinct());
                            }
                            TestContext.WriteLine($"Identifier(s): {ids}");
                            TestContext.WriteLine($"Scope(s): {scopes}");
                            TestContext.WriteLine(transform.AsProjString(new Proj.ProjStringOptions { MultiLine = true }));
                            //TestContext.WriteLine(transform.AsWellKnownText());


                            TestContext.WriteLine("");
                        }
                    }
                }
            }
        }

        [TestMethod]
        public void CheckTransform_EPSG15020()
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.EnableNetworkConnections = true;

                using (var ct = CoordinateTransform.CreateFromEpsg(15020, pc))
                {
                    Assert.IsNotNull(ct);
                    Assert.AreEqual("Alaska Albers", ct.Name);
                    Assert.AreEqual(new Proj.Identifier("EPSG", 15020), ct.Identifier);

                    Assert.IsNull(ct.SourceCRS, "No sourceCRS");
                    Assert.IsNull(ct.TargetCRS, "No targetCRS");
                }
            }
        }

        [TestMethod]
        [DataRow(false)]
        [DataRow(true)]
        public void InvokeWays(bool enableNetwork)
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.EnableNetworkConnections = enableNetwork;

                using (CoordinateReferenceSystem crsFrom = CoordinateReferenceSystem.CreateFromEpsg(2964, pc))
                using (CoordinateReferenceSystem crsTo = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
                using (var ct = CoordinateTransform.Create(crsFrom, crsTo))
                {
                    Assert.AreEqual(enableNetwork ? "United States (USA) - Alaska including EEZ."
                                                  : "United States (USA) - Alaska mainland.", ct.UsageArea.Name);

                    Func<UsageArea, IEnumerable<PPoint>> getPoints =
                        (ua) => typeof(UsageArea).GetProperties().Where(x => x.PropertyType == typeof(PPoint)).Select(x => (PPoint)x.GetValue(ua)).Where(x => x.HasValues);

                    PPoint[] allPoints = getPoints(crsFrom.UsageArea).ToArray();

                    double[] xs = allPoints.Select(x => x.X).ToArray();
                    double[] ys = allPoints.Select(x => x.Y).ToArray();
                    double[,] ranked = new double[allPoints.Length, 2];

                    for (int i = 0; i < allPoints.Length; i++)
                    {
                        ranked[i, 0] = allPoints[i].X;
                        ranked[i, 1] = allPoints[i].Y;
                    }

                    PPoint[] transformedPoints = allPoints.Select(p => ct.Apply(p)).ToArray();
                    ct.Apply(xs, ys);
                    ct.Apply(ranked);

                    for (int i = 0; i < allPoints.Length; i++)
                    {
                        Assert.AreEqual(transformedPoints[i].X, xs[i]);
                        Assert.AreEqual(transformedPoints[i].Y, ys[i]);

                        Assert.AreEqual(ranked[i, 0], xs[i]);
                        Assert.AreEqual(ranked[i, 1], ys[i]);
                    }

                    PPoint[] backPoints = transformedPoints.Select(p => ct.ApplyReversed(p)).ToArray();
                    ct.ApplyReversed(xs, ys);
                    ct.ApplyReversed(ranked);

                    int precision = enableNetwork ? 1 : 10000; // Without network this doesn't perform a sane roundtrip

                    for (int i = 0; i < allPoints.Length; i++)
                    {
                        Assert.AreEqual(Math.Round(allPoints[i].X / precision), Math.Round(backPoints[i].X / precision));
                        Assert.AreEqual(Math.Round(allPoints[i].Y / precision), Math.Round(backPoints[i].Y / precision));

                        Assert.AreEqual(backPoints[i].X, xs[i]);
                        Assert.AreEqual(backPoints[i].Y, ys[i]);

                        Assert.AreEqual(ranked[i, 0], xs[i]);
                        Assert.AreEqual(ranked[i, 1], ys[i]);

                    }
                }
            }
        }

        [TestMethod]
        [DynamicData(nameof(AsCRS100))]
        public void TestBoundsCRS100(Identifier id)
        {
            using (var c = CoordinateReferenceSystem.CreateFromDatabase(id))
            {
                var v = c.UsageArea.MinX;
                Assert.IsTrue(!double.IsInfinity(v) && !double.IsNaN(v));
                Assert.AreNotEqual(c.UsageArea.MinX, c.UsageArea.MaxX);
                Assert.AreNotEqual(c.UsageArea.MinY, c.UsageArea.MaxY);
            }
        }

        public static IEnumerable<object[]> AsCRS100
        {
            get
            {
                using (ProjContext pc = new ProjContext())
                {
                    return pc.GetCoordinateReferenceSystems(
                        new CoordinateReferenceSystemFilter { CelestialBodyName = "Earth" }
                        ).Where(x => x.Type != ProjType.VerticalCrs).Select(x => new[] { x.Identifier }).Take(100).ToList();
                }
            }
        }


        [TestMethod]
        [DynamicData(nameof(AsCRSPole))]
        public void TestBoundsPole(Identifier id)
        {
            using (var c = CoordinateReferenceSystem.CreateFromDatabase(id))
            {
                var v = c.UsageArea.MinX;
                if (c.Name.Contains("Peirce") && ProjContext.ProjVersion == new Version(8, 2, 1))
                {
                    Assert.Inconclusive(); // Broken in 8.2.1
                }

                Assert.IsTrue(!double.IsInfinity(v) && !double.IsNaN(v));
                Assert.AreNotEqual(c.UsageArea.MinX, c.UsageArea.MaxX);
                Assert.AreNotEqual(c.UsageArea.MinY, c.UsageArea.MaxY);
                TestContext.WriteLine($"X: {c.UsageArea.MinX} - {c.UsageArea.MaxX}");
                TestContext.WriteLine($"Y: {c.UsageArea.MinY} - {c.UsageArea.MaxY}");
            }
        }

        public static IEnumerable<object[]> AsCRSPole
        {
            get
            {
                using (ProjContext pc = new ProjContext())
                {
                    return pc.GetCoordinateReferenceSystems(
                        new CoordinateReferenceSystemFilter { CelestialBodyName = "Earth" }
                        ).Where(x => (x.Name + x.AreaName).ToUpperInvariant().Contains("POLE") && x.Type != ProjType.VerticalCrs).Select(x => new[] { x.Identifier }).ToList();
                }
            }
        }

        [TestMethod]
        [DataRow("ESRI", 102034)]
        [DataRow("ESRI", 102036)]
        public void TestGnomoric(string authority, int code)
        {
            using (var c = CoordinateReferenceSystem.CreateFromDatabase(authority, code))
            {
                var v = c.UsageArea.MinX;

                Assert.IsTrue(!double.IsInfinity(v) && !double.IsNaN(v));
                Assert.AreNotEqual(c.UsageArea.MinX, c.UsageArea.MaxX);
                Assert.AreNotEqual(c.UsageArea.MinY, c.UsageArea.MaxY);
                TestContext.WriteLine($"X: {c.UsageArea.MinX} - {c.UsageArea.MaxX}");
                TestContext.WriteLine($"Y: {c.UsageArea.MinY} - {c.UsageArea.MaxY}");
            }
        }
    }
}
