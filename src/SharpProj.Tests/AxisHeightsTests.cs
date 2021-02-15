using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NetTopologySuite.Geometries;
using SharpProj.Proj;
using SharpProj.NetTopologySuite.Testing;

namespace SharpProj.Tests
{
    [TestClass]
    public class AxisHeightsTests
    {
        static readonly Coordinate DomUtrechtWGS84 = new Coordinate(52.09063, 5.123078); // WGS84
        static readonly Coordinate StServaasMaastrichtWGS84 = new Coordinate(50.84938, 5.687712);

        [TestMethod]
        public void HeightConversionNLBE()
        {
            using (ProjContext pc = new ProjContext())
            using (var nlNAP = CoordinateReferenceSystem.Create("EPSG:7415", pc))
            using (var beOstend = CoordinateReferenceSystem.Create("EPSG:8370", pc))
            {
                pc.AllowNetworkConnections = true;
                Assert.AreEqual(3, nlNAP.AxisCount);
                Assert.AreEqual(3, beOstend.AxisCount);

                Coordinate domNL, servaasNL;
                using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326", pc))
                using (var wgs84D3 = CoordinateReferenceSystem.Create("EPSG:4329", pc))
                {
                    Assert.AreEqual(2, wgs84.AxisCount);
                    Assert.AreEqual(3, wgs84D3.AxisCount);

                    using (var n = wgs84D3.WithAxisNormalized())
                    {
                        Assert.IsFalse(wgs84D3.IsEquivalentTo(n));
                        Assert.IsTrue(wgs84D3.IsEquivalentToRelaxed(n));
                    }

                    Coordinate domWGS84 = new Coordinate(DomUtrechtWGS84);
                    Coordinate stServaasWGS84 = new Coordinate(StServaasMaastrichtWGS84);

                    using (var t = CoordinateTransform.Create(wgs84, nlNAP))
                    {
                        domNL = t.Apply(domWGS84);

                        Assert.AreEqual(new CoordinateZ(136898.7, 455851.9, -43.4), domNL.RoundAll(1));

                        servaasNL = t.Apply(stServaasWGS84);

                        Assert.AreEqual(new CoordinateZ(176164.5, 317770.5, -45.7), servaasNL.RoundAll(1));
                    }


                    using (var t = CoordinateTransform.Create(nlNAP, beOstend))
                    {
                        var servaasBE = t.Apply(servaasNL);

                        Assert.AreEqual(new CoordinateZ(742877.4, 671835, -45.7), servaasBE.RoundAll(1));

                        servaasNL.Z = 15;
                        servaasBE = t.Apply(servaasNL);

                        Assert.IsTrue(new CoordinateZ(742877.4, 671835, 15).Equals3D(servaasBE.RoundAll(1)), $"Unexpected coordinate {servaasBE.RoundAll(1)}");

                        servaasNL.Z = 0; //revert to original value
                    }


                    using (var t = CoordinateTransform.Create(nlNAP, wgs84D3))
                    {
                        var domGPS = t.Apply(domNL);
                        Assert.AreEqual(new CoordinateZ(52.09063, 5.123078, 0), domGPS.RoundAll(7));

                        var servaasGPS = t.Apply(servaasNL);
                        Assert.AreEqual(new CoordinateZ(50.84938, 5.687712, 45.7353891), servaasGPS.RoundAll(7));

                        servaasNL.Z = 15;

                        servaasGPS = t.Apply(servaasNL);

                        Assert.IsTrue(new CoordinateZ(50.84938, 5.68771, 60.73539).Equals3D(servaasGPS.RoundAll(5)), $"Unexpected coordinate {servaasGPS.RoundAll(5)}");

                    }
                }
            }
        }
        [TestMethod]
        public void DistanceDemo()
        {
            // Demo case from https://www.mkompf.com/gps/distcalc.html
            PPoint RusselsheimStation = new PPoint(49.9917, 8.41321);
            PPoint RusselsheimOpelBridge = new PPoint(50.0049, 8.42182);
            PPoint BerlinBrandenburgGate = new PPoint(52.5164, 13.3777);
            PPoint LisbonTagusBridge = new PPoint(38.692668, -9.177944);

            using (var ctx = new ProjContext())
            using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326", ctx))
            {
                Assert.AreEqual(1592.7, Math.Round(wgs84.DistanceTransform.GeoDistance(RusselsheimStation, RusselsheimOpelBridge), 1));

                Assert.AreEqual(2318217, Math.Round(wgs84.DistanceTransform.GeoDistance(BerlinBrandenburgGate, LisbonTagusBridge)));
            }
        }

        [TestMethod]
        public void DistanceInNL()
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.AllowNetworkConnections = true;

                Coordinate dom, servaas;

                using (var nl = CoordinateReferenceSystem.Create("EPSG:28992", pc))
                {
                    using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326", pc))
                    {
                        using (var t = CoordinateTransform.Create(wgs84, nl))
                        {
                            dom = t.Apply(DomUtrechtWGS84);
                            servaas = t.Apply(StServaasMaastrichtWGS84);
                        }

                        Assert.AreEqual(143.562, Math.Round(wgs84.DistanceTransform.GeoDistance(DomUtrechtWGS84, StServaasMaastrichtWGS84)) / 1000.0, "Distance WGS84");
                    }

                    Assert.AreEqual(143.562, Math.Round(nl.DistanceTransform.GeoDistance(dom, servaas), 0) / 1000.0, "Distance RD");

                    double dx = (dom.X - servaas.X);
                    double dy = (dom.Y - servaas.Y);

                    Assert.AreEqual(143.556, Math.Round(Math.Sqrt(dx * dx + dy * dy)) / 1000.0, "Distance pythagoras");
                }

                using (var nlNAP = CoordinateReferenceSystem.Create("EPSG:7415", pc))
                {
                    Assert.IsNotNull(nlNAP.GeodeticCRS);

                    if (nlNAP is CoordinateReferenceSystemList l)
                    {
                        var cs0 = l[0];
                        var cs1 = l[1];

                        GC.KeepAlive(cs0);
                        GC.KeepAlive(cs1);
                    }

                    using (var WGS84 = CoordinateReferenceSystem.Create("EPSG:4329", pc))
                    {
                        GC.KeepAlive(WGS84);
                    }
                }
            }
        }
    }
}
