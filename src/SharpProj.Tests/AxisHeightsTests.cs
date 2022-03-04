using System;
using System.Diagnostics;
using System.Globalization;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace SharpProj.Tests
{
    [TestClass]
    public class AxisHeightsTests
    {
        static readonly PPoint DomUtrechtWGS84 = new PPoint(52.09063, 5.123078); // WGS84
        static readonly PPoint StServaasMaastrichtWGS84 = new PPoint(50.84938, 5.687712);

        public TestContext TestContext { get; set; }

        [TestMethod]
        [TestCategory("NeedsNetwork")]
        public void HeightConversionNLBE()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true })
            using (var nlNAP = CoordinateReferenceSystem.CreateFromEpsg(7415, pc))
            using (var beOstend = CoordinateReferenceSystem.CreateFromEpsg(8370, pc))
            {
                Assert.AreEqual(3, nlNAP.AxisCount);
                Assert.AreEqual(3, beOstend.AxisCount);

                PPoint domNL, servaasNL;
                using (var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
                using (var wgs84D3 = CoordinateReferenceSystem.CreateFromEpsg(4329, pc))
                {
                    Assert.AreEqual(2, wgs84.AxisCount);
                    Assert.AreEqual(3, wgs84D3.AxisCount);

                    var n = wgs84D3.WithNormalizedAxis();
                    Assert.IsFalse(wgs84D3.IsEquivalentTo(n));
                    Assert.IsTrue(wgs84D3.IsEquivalentToRelaxed(n));

                    PPoint domWGS84 = DomUtrechtWGS84;
                    PPoint stServaasWGS84 = StServaasMaastrichtWGS84;

                    Assert.AreEqual("Lat=52.09063, Lon=5.123078", domWGS84.ToString(wgs84, CultureInfo.InvariantCulture));
                    Assert.AreEqual("Lat=50.84938, Lon=5.687712", stServaasWGS84.ToString(wgs84, CultureInfo.InvariantCulture));

                    using (var t = CoordinateTransform.Create(wgs84, nlNAP))
                    {
                        domNL = t.Apply(domWGS84);

                        Assert.AreEqual(new PPoint(136898.7, 455851.9, -43.4), domNL.ToXYZ(1));

                        servaasNL = t.Apply(stServaasWGS84);

                        Assert.AreEqual(new PPoint(176164.5, 317770.5, -45.7), servaasNL.ToXYZ(1));

                        Assert.AreEqual(143562.0, Math.Round(wgs84.GeoDistance(domWGS84, stServaasWGS84), 1));
                        Assert.AreEqual(143562.4, Math.Round(nlNAP.GeoDistance(domNL, servaasNL), 1));
                    }

                    Assert.AreEqual(nlNAP, nlNAP.WithNormalizedAxis()); // NAP has axis normalized
                    using (var wgs84Normalized = wgs84.WithNormalizedAxis())
                    {
                        Assert.AreNotEqual(wgs84, wgs84Normalized);
                        Assert.IsTrue(wgs84.IsEquivalentToRelaxed(wgs84Normalized));

                        var domWgsNormalized = DomUtrechtWGS84.SwapXY();
                        var servaasWgsNormalized = stServaasWGS84.SwapXY();

                        Assert.AreEqual(143562.0, Math.Round(wgs84Normalized.GeoDistance(domWgsNormalized, servaasWgsNormalized), 1));
                    }

                    Assert.AreEqual(beOstend, beOstend.WithNormalizedAxis()); // beOstend has axis normalized

                    using (var t = CoordinateTransform.Create(nlNAP, beOstend))
                    {
                        var servaasBE = t.Apply(servaasNL);
                        var domBE = t.Apply(domNL);

                        Assert.AreEqual(new PPoint(742877.4, 671835, -45.7), servaasBE.ToXYZ(1));

                        servaasNL.Z = 15;
                        servaasBE = t.Apply(servaasNL);

                        Assert.AreEqual(new PPoint(742877.4, 671835, 15), servaasBE.ToXYZ(1));

                        servaasNL.Z = 0; //revert to original value

                        Assert.AreEqual(143562.0, Math.Round(beOstend.GeoDistance(domBE, servaasBE), 1));
                    }


                    using (var t = CoordinateTransform.Create(nlNAP, wgs84D3))
                    {
                        var domGPS = t.Apply(domNL);
                        Assert.AreEqual(new PPoint(52.09063, 5.123078, 0), domGPS.ToXYZ(7));

                        var servaasGPS = t.Apply(servaasNL);
                        Assert.AreEqual(new PPoint(50.84938, 5.687712, 45.7353891), servaasGPS.ToXYZ(7));

                        servaasNL.Z = 15;
                        servaasGPS = t.Apply(servaasNL);

                        Assert.AreEqual(new PPoint(50.84938, 5.68771, 60.73539), servaasGPS.ToXYZ(5));
                    }
                }
            }
        }

        [TestMethod]
        public void DistanceInNL()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true })
            using (var nl = CoordinateReferenceSystem.CreateFromEpsg(28992, pc))
            using (var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
            using (var nlNAP = CoordinateReferenceSystem.CreateFromEpsg(7415, pc))
            {
                PPoint dom, servaas;


                using (var t = CoordinateTransform.Create(wgs84, nl))
                {
                    dom = t.Apply(DomUtrechtWGS84);
                    servaas = t.Apply(StServaasMaastrichtWGS84);
                }

                Assert.AreEqual(143.562, Math.Round(wgs84.GeoDistance(DomUtrechtWGS84, StServaasMaastrichtWGS84)) / 1000.0, "Distance WGS84");

                Assert.AreEqual(143.562, Math.Round(nl.GeoDistance(dom, servaas), 0) / 1000.0, "Distance RD");

                double dx = (dom.X - servaas.X);
                double dy = (dom.Y - servaas.Y);

                Assert.AreEqual(143.556, Math.Round(Math.Sqrt(dx * dx + dy * dy)) / 1000.0, "Distance pythagoras");

                Assert.IsNotNull(nlNAP.GeodeticCRS);

                if (nlNAP is CoordinateReferenceSystemList l)
                {
                    var cs0 = l[0];
                    var cs1 = l[1];

                    GC.KeepAlive(cs0);
                    GC.KeepAlive(cs1);
                }

                using (var WGS84_2 = CoordinateReferenceSystem.CreateFromEpsg(4329, pc))
                {
                    GC.KeepAlive(WGS84_2);
                }
            }
        }

        [TestMethod]
        public void RecreateViaWKT()
        {
            using var pc = new ProjContext() { EnableNetworkConnections = true };

            pc.LogLevel = ProjLogLevel.Trace;
            pc.Log += (x, y) => Debug.WriteLine(y);

            using var nlNAP = CoordinateReferenceSystem.CreateFromEpsg(7415, pc);
            using var wgs84_3D = CoordinateReferenceSystem.CreateFromEpsg(4329, pc);

            string tWkt = "";
            string tProj = "";
            CoordinateTransform from;

            using var tOriginal = CoordinateTransform.Create(nlNAP, wgs84_3D);

            if (tOriginal is ChooseCoordinateTransform cct)
            {
                tWkt = cct[0].AsWellKnownText();
                tProj = cct[0].AsProjString(new Proj.ProjStringOptions { MultiLine = true });
                from = cct[0];
            }
            else
            {
                tWkt = tOriginal.AsWellKnownText();
                tProj = tOriginal.AsProjString(new Proj.ProjStringOptions { MultiLine = true });
                from = tOriginal;
            }

            Assert.IsNotNull(tWkt);

            using var tWgs84nlNAP = CoordinateTransform.Create(wgs84_3D, nlNAP);

            var domNL = tWgs84nlNAP.Apply(DomUtrechtWGS84);
            var servaasNL = tWgs84nlNAP.Apply(StServaasMaastrichtWGS84);


            using (var t1 = CoordinateTransform.Create(tProj, pc))
            {
                var domGPS1 = from.Apply(domNL);
                var domGPS = t1.Apply(domNL);

                Assert.AreEqual(domGPS1, domGPS);

                Assert.AreEqual(tProj, t1.AsProjString(new Proj.ProjStringOptions { MultiLine = true }));
            }

            using (var t2 = CoordinateTransform.CreateFromWellKnownText(tWkt, new Proj.CreateFromWKTOptions { Strict = true }, out var warnings, pc))
            {
                //Assert.AreEqual("", t2.AsProjString(new Proj.ProjStringOptions { MultiLine = true }));
                if (tProj != t2.AsProjString(new Proj.ProjStringOptions { MultiLine = true }))
                {
#if DEBUG
                    Assert.AreEqual(tProj, t2.AsProjString(new Proj.ProjStringOptions { MultiLine = true }));
#endif
                    Assert.Inconclusive("Proj string should have matched after reloading via WKT");
                }
            }
        }
    }
}
