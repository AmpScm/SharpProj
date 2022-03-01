using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace SharpProj.Tests
{
    [TestClass]
    public class DistanceTests
    {
        public TestContext TestContext { get; set; }

        [TestMethod]
        public void DistanceDemo()
        {
            // Demo case from https://www.mkompf.com/gps/distcalc.html
            PPoint RusselsheimStation = new PPoint(49.9917, 8.41321);
            PPoint RusselsheimOpelBridge = new PPoint(50.0049, 8.42182);
            PPoint BerlinBrandenburgGate = new PPoint(52.5164, 13.3777);
            PPoint LisbonTagusBridge = new PPoint(38.692668, -9.177944);

            using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326"))
            {
                Assert.AreEqual("Lat=50.0049, Lon=8.42182", RusselsheimOpelBridge.ToString(wgs84, CultureInfo.InvariantCulture));
                var dt = wgs84.DistanceTransform;
                Assert.AreEqual("Lon=8.42182, Lat=50.0049", dt.Apply(RusselsheimOpelBridge).ToString(dt.TargetCRS, CultureInfo.InvariantCulture));

                Assert.AreEqual(1592.7, Math.Round(wgs84.GeoDistance(RusselsheimStation, RusselsheimOpelBridge), 1));

                Assert.AreEqual(2318217, Math.Round(wgs84.GeoDistance(BerlinBrandenburgGate, LisbonTagusBridge)));


                Assert.AreEqual(1592.7, Math.Round(wgs84.GeoDistanceZ(RusselsheimStation, RusselsheimOpelBridge), 1));

                Assert.AreEqual(2318217, Math.Round(wgs84.GeoDistanceZ(BerlinBrandenburgGate, LisbonTagusBridge)));


            }
        }

        [TestMethod]
        public void DistanceDemoNormalized()
        {
            // Demo case from https://www.mkompf.com/gps/distcalc.html
            PPoint RusselsheimStation = new PPoint(49.9917, 8.41321).SwapXY();
            PPoint RusselsheimOpelBridge = new PPoint(50.0049, 8.42182).SwapXY();
            PPoint BerlinBrandenburgGate = new PPoint(52.5164, 13.3777).SwapXY();
            PPoint LisbonTagusBridge = new PPoint(38.692668, -9.177944).SwapXY();

            using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326").WithNormalizedAxis())
            {
                Assert.AreEqual("Lon=8.42182, Lat=50.0049", RusselsheimOpelBridge.ToString(wgs84, CultureInfo.InvariantCulture));
                var dt = wgs84.DistanceTransform;
                Assert.AreEqual("Lon=8.42182, Lat=50.0049", dt.Apply(RusselsheimOpelBridge).ToString(dt.TargetCRS, CultureInfo.InvariantCulture));

                Assert.AreEqual(1592.7, Math.Round(wgs84.GeoDistance(RusselsheimStation, RusselsheimOpelBridge), 1));
                Assert.AreEqual(2318217, Math.Round(wgs84.GeoDistance(BerlinBrandenburgGate, LisbonTagusBridge)));

                Assert.AreEqual(1592.7, Math.Round(wgs84.GeoDistanceZ(RusselsheimStation, RusselsheimOpelBridge), 1));
                Assert.AreEqual(2318217, Math.Round(wgs84.GeoDistanceZ(BerlinBrandenburgGate, LisbonTagusBridge)));
            }
        }



        [TestMethod]
        public void TestNormalizeAustralia()
        {
            using (ProjContext pc = new ProjContext() { EnableNetworkConnections = true })
            using (var australia = CoordinateReferenceSystem.CreateFromEpsg(7843, pc))
            using (var australiaNormalized = australia.WithNormalizedAxis())
            using (var epsg7665 = CoordinateReferenceSystem.CreateFromEpsg(7665, pc))
            using (var epsg7665Normalized = epsg7665.WithNormalizedAxis())
            using (var t = CoordinateTransform.Create(australia, epsg7665, new CoordinateTransformOptions { IntermediateCrsUsage = IntermediateCrsUsage.Always, NoDiscardIfMissing = true }))
            using (var tNorm = CoordinateTransform.Create(australiaNormalized, epsg7665Normalized, new CoordinateTransformOptions { IntermediateCrsUsage = IntermediateCrsUsage.Always, NoDiscardIfMissing = true }))
            {
                Assert.IsTrue(!australia.IsEquivalentTo(australiaNormalized) && australia.IsEquivalentToRelaxed(australiaNormalized), "Normalized equal");
                Assert.IsTrue(!epsg7665.IsEquivalentTo(epsg7665Normalized) && epsg7665.IsEquivalentToRelaxed(epsg7665Normalized), "Normalized equal");

                var uua = australia.UsageArea;
                var ur = t.Apply(uua.Center);
                var ur2 = t.Apply(uua.Center.WithT(2000));
                var ur3 = t.Apply(uua.Center.WithT(2020));

                Assert.AreNotEqual(ur2.ToXY(), ur3.ToXY());
                GC.KeepAlive(t);

                var nua = australiaNormalized.UsageArea;
                var nr = tNorm.Apply(nua.Center);
                var nr2 = tNorm.Apply(nua.Center.WithT(2000));
                var nr3 = tNorm.Apply(nua.Center.WithT(2020));

                Assert.AreNotEqual(nr2.ToXY(), nr3.ToXY());

                Assert.AreNotEqual(ur.X, ur.Y);
                Assert.AreEqual(ur.X, nr.Y);
                Assert.AreEqual(ur.Y, nr.X);

                Assert.AreEqual("Lat,Lon,h", string.Join(",", australia.Axis.Select(x => x.Abbreviation)));
                Assert.AreEqual("Lat,Lon,h", string.Join(",", australia.DistanceTransform.SourceCRS.Axis.Select(x => x.Abbreviation)));
                Assert.AreEqual("Lon,Lat,h", string.Join(",", australia.DistanceTransform.TargetCRS.Axis.Select(x => x.Abbreviation)));
                Assert.AreEqual(new PPoint(133.375, -34.51, 0), australia.DistanceTransform.Apply(uua.Center));

                Assert.AreEqual("Lon,Lat,h", string.Join(",", australiaNormalized.Axis.Select(x => x.Abbreviation)));
                Assert.AreEqual("Lon,Lat,h", string.Join(",", australiaNormalized.DistanceTransform.SourceCRS.Axis.Select(x => x.Abbreviation)));
                Assert.AreEqual("Lon,Lat,h", string.Join(",", australiaNormalized.DistanceTransform.TargetCRS.Axis.Select(x => x.Abbreviation)));
                Assert.AreEqual(new PPoint(133.375, -34.51, 0), australiaNormalized.DistanceTransform.Apply(nua.Center));

                Assert.IsTrue(australia.Ellipsoid.IsEquivalentTo(australiaNormalized.Ellipsoid));
                Assert.IsTrue(australia.Datum.IsEquivalentTo(australiaNormalized.Datum));


                Assert.AreEqual("Lon=93.41, Lat=-8.47, h=0", australia.DistanceTransform.Apply(uua.NorthWestCorner).ToString(australia.DistanceTransform.TargetCRS, CultureInfo.InvariantCulture));
                Assert.AreEqual("Lon=93.41, Lat=-8.47, h=0", australiaNormalized.DistanceTransform.Apply(nua.NorthWestCorner).ToString(australiaNormalized.DistanceTransform.TargetCRS, CultureInfo.InvariantCulture));

                var uD = australia.DistanceTransform.GeoDistance(uua.NorthWestCorner, uua.Center);
                var nD = australiaNormalized.DistanceTransform.GeoDistance(nua.NorthWestCorner, nua.Center);

                Assert.IsFalse(double.IsNaN(uD));
                Assert.AreEqual(uD, nD);
                Assert.AreEqual(4992993, Math.Round(nD));
            }
        }
    }
}
