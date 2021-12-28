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
                        Assert.AreEqual("?", ct.Scope);

                        var cct = ct as ChooseCoordinateTransform;
                        Assert.IsNotNull("Have Choose coordinate transform");

                        Assert.AreEqual(useNetwork ? 10 : 8, cct.Count);
                        Assert.AreEqual(cct.First().UsageArea.Name, ct.UsageArea.Name);

                        foreach (var transform in cct)
                        {
                            Assert.IsTrue(crsFrom.IsEquivalentTo(transform.SourceCRS));
                            Assert.IsTrue(crsTo.IsEquivalentTo(transform.TargetCRS));

                            Console.WriteLine($"{transform.Name} - {transform.Accuraracy} m - {transform.UsageArea?.Name}");

                            string ids = transform.Identifier?.ToString();
                            string scopes = transform.Scope;
                            if (ids == null && transform is CoordinateTransformList ctl)
                            {
                                ids = string.Join(", ", ctl.Where(x => x.Identifiers != null).SelectMany(x => x.Identifiers));
                                scopes = string.Join(", ", ctl.Select(x => x.Scope).Where(x => x is not null).Distinct());
                            }
                            Console.WriteLine($"Identifier(s): {ids}");
                            Console.WriteLine($"Scope(s): {scopes}");
                            Console.WriteLine(transform.AsProjString(new Proj.ProjStringOptions { MultiLine = true }));
                            //Console.WriteLine(transform.AsWellKnownText());


                            Console.WriteLine();
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
        public void InvokeWays()
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.EnableNetworkConnections = false;

                using (CoordinateReferenceSystem crsFrom = CoordinateReferenceSystem.CreateFromEpsg(2964, pc))
                using (CoordinateReferenceSystem crsTo = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
                using (var ct = CoordinateTransform.Create(crsFrom, crsTo))
                {
                    Assert.AreEqual("United States (USA) - Alaska mainland.", ct.UsageArea.Name);

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
                }
            }
        }
    }
}
