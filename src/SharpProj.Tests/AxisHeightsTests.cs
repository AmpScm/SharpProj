using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NetTopologySuite.Geometries;

namespace SharpProj.Tests
{
    [TestClass]
    public class AxisHeightsTests
    {
        static readonly Coordinate DomUtrechtWGS84 = new Coordinate(52.0906147, 5.1212057); // WGS84
        static readonly Coordinate StServaasMaastrichtWGS84 = new Coordinate(50.8491832, 5.6872587);

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

                    using (var n = wgs84D3.GetNormalized())
                    {
                        Assert.IsFalse(wgs84D3.IsEquivalentTo(n));
                        Assert.IsTrue(wgs84D3.IsEquivalentToRelaxed(n));
                    }

                    Coordinate domWGS84 = new Coordinate(DomUtrechtWGS84);
                    Coordinate stServaasWGS84 = new Coordinate(StServaasMaastrichtWGS84);

                    using (var t = CoordinateTransform.Create(wgs84, nlNAP))
                    {
                        domNL = t.Apply(domWGS84);

                        Assert.AreEqual(new CoordinateZ(136770.4, 455850.7, 0), domNL.Round(1));

                        servaasNL = t.Apply(stServaasWGS84);

                        Assert.AreEqual(new CoordinateZ(176132.7, 317748.5, -45.7), servaasNL.Round(1));
                    }


                    using (var t = CoordinateTransform.Create(nlNAP, beOstend))
                    {
                        var servaasBE = t.Apply(servaasNL);

                        Assert.AreEqual(new CoordinateZ(742845.8, 671812.5, -45.7), servaasBE.Round(1));

                        servaasNL.Z = 15;
                        servaasBE = t.Apply(servaasNL);

                        Assert.IsTrue(new CoordinateZ(742845.8, 671812.5, 15).Equals3D(servaasBE.Round(1)), $"Unexpected coordinate {servaasBE.Round(1)}");

                        servaasNL.Z = 0; //revert to original value
                    }


                    using (var t = CoordinateTransform.Create(nlNAP, wgs84D3))
                    {
                        var domGPS = t.Apply(domNL);
                        Assert.AreEqual(new CoordinateZ(52.0906147, 5.1212057, 0), domGPS.Round(7));

                        var servaasGPS = t.Apply(servaasNL);
                        Assert.AreEqual(new CoordinateZ(50.8491832, 5.6872587, 45.7359553), servaasGPS.Round(7));

                        servaasNL.Z = 15;

                        servaasGPS = t.Apply(servaasNL);

                        Assert.IsTrue(new CoordinateZ(50.84918, 5.68726, 60.73596).Equals3D(servaasGPS.Round(5)), $"Unexpected coordinate {servaasGPS.Round(5)}");

                    }
                }
            }
        }

        [TestMethod]
        public void DistanceInNL()
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.AllowNetworkConnections = true;

                Coordinate dom, servaas;

                using (var nl = CoordinateReferenceSystem.Create("EPSG:28992"))
                {
                    using (var wgs84 = CoordinateReferenceSystem.Create("EPSG:4326", pc))
                    {
                        using (var t = CoordinateTransform.Create(wgs84, nl))
                        {
                            dom = t.Apply(DomUtrechtWGS84);
                            servaas = t.Apply(StServaasMaastrichtWGS84);
                        }

                        using (var t = CoordinateTransform.Create(wgs84, wgs84))
                        {
                            Assert.AreEqual(151, Math.Round(t.EllipsoidDistance(t.Apply(DomUtrechtWGS84), t.Apply(StServaasMaastrichtWGS84)) / 1000.0, 0));
                        }
                    }


                    Assert.IsNotNull(nl.GeodeticCRS);

                    using (var t = CoordinateTransform.Create(nl, nl.GeodeticCRS))
                    using (var t2 = nl.GetTransform())
                    {
                        //Assert.IsTrue(t2.IsEquivalentTo(t));

                        Coordinate domGeo = t.Apply(dom);
                        Coordinate servaasGeo = t.Apply(servaas);

                        Coordinate domGeo2 = t2.Apply(dom);

                        Assert.AreEqual(new Coordinate(52.091595, 5.1216022), domGeo.Round(7));


                        //Assert.AreEqual(300.0, nl.EllipsoidDistance(domGeo, servaasGeo));
                        Assert.AreEqual(151, Math.Round(t2.EllipsoidDistance(domGeo, servaasGeo) / 1000.0, 0));

                    }
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

                        using (var op = cs0.GetTransform())
                        {
                            GC.KeepAlive(op);
                        }

                    }

                    pc.AllowNetworkConnections = true;
                    using (var WGS84 = CoordinateReferenceSystem.Create("EPSG:4329", pc))
                    {
                        GC.KeepAlive(WGS84);
                    }
                }
            }
        }
    }
}
