using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NetTopologySuite.Geometries;
using NetTopologySuite.Geometries.Implementation;
using SharpProj.NTS;
using SharpProj.Testing;

namespace SharpProj.Tests
{
    //[TestClass]
    public class SridTests
    {
        public TestContext TestContext { get; set; }
        enum Epsg //
        {
            Netherlands = 28992,
            BelgiumLambert = 3812,

            AnotherNL
        }

        [TestInitialize]
        public void Init()
        {
            SridRegister.Ensure(Epsg.Netherlands, () => CoordinateReferenceSystem.Create("EPSG:28992"), (int)Epsg.Netherlands);
            SridRegister.Ensure(Epsg.BelgiumLambert, () => CoordinateReferenceSystem.Create("EPSG:3812"), (int)Epsg.BelgiumLambert);
            SridRegister.Ensure(Epsg.AnotherNL, () => CoordinateReferenceSystem.Create("EPSG:28992"), (int)Epsg.AnotherNL); // Different EPSG, same definition. Ok. But can't use same CRS instance
        }

        [TestMethod]
        public void EpsgEnumTests()
        {
            var r = SridRegister.GetById(Epsg.Netherlands);
            var factory = r.Factory;

            //SharpProj.
            Assert.AreEqual((int)Epsg.Netherlands, r.SRID);
            Assert.AreEqual((int)Epsg.Netherlands, factory.SRID);

            Assert.AreEqual((int)Epsg.BelgiumLambert, SridRegister.GetById(Epsg.BelgiumLambert).SRID);

            Assert.AreEqual(2, r.CRS.CoordinateSystem.Axis.Count);
            Point p = factory.CreatePoint(new Coordinate(155000, 463000));

            Point pp = p.Reproject(SridRegister.GetById(Epsg.BelgiumLambert));

            Assert.IsNotNull(pp);

            Assert.IsNotNull(p.Reproject<Geometry>(SridRegister.GetById(Epsg.BelgiumLambert)));

            Assert.AreEqual((int)Epsg.BelgiumLambert, pp.SRID);
            Assert.AreEqual(new Point(719706, 816781), new Point(pp.Coordinate.RoundAll(0)));

            using(var pc = new ProjContext())
            using (CoordinateTransform t = CoordinateTransform.Create(SridRegister.GetByValue(p.SRID), SridRegister.GetById(Epsg.BelgiumLambert), new CoordinateTransformOptions { NoBallparkConversions = true }, pc))
            {
                if (t is CoordinateTransformList mc)
                {
                    Assert.AreEqual(3, mc.Count);
                    Assert.AreEqual(5, mc[0].Parameters.Count);
                    Assert.AreEqual(7, mc[1].Parameters.Count);
                    Assert.AreEqual(6, mc[2].Parameters.Count);
                }
                else
                    Assert.Fail();

                var rr = t.Apply(new PPoint(155000, 463000));

            }
        }

        [TestMethod, ExpectedException(typeof(ArgumentException))]
        public void NotSameTwice()
        {
            CoordinateReferenceSystem crs = SridRegister.GetById(Epsg.Netherlands);

            SridRegister.Ensure((Epsg)(-1), () => crs, -1);
        }

        private static Polygon CreateTriangle(GeometryFactory f, Coordinate centerPoint, double size)
        {
            double halfheight = Math.Cos(CoordinateTransform.ToRad(30)) * size / 2;

            return f.CreatePolygon(new Coordinate[]
            {
                new Coordinate(centerPoint.X, centerPoint.Y-halfheight),
                new Coordinate(centerPoint.X - size/2, centerPoint.Y+halfheight),
                new Coordinate(centerPoint.X + size/2, centerPoint.Y+halfheight),
                new Coordinate(centerPoint.X, centerPoint.Y-halfheight),
            });
        }

        [TestMethod]
        public void NtsMeterDistance()
        {
            PPoint amersfoortRD = new PPoint(155000, 463000);

            var srid = SridRegister.GetById(Epsg.Netherlands);

            var t1 = CreateTriangle(srid.Factory, amersfoortRD.Offset(-50000, 0).ToCoordinate(), 5000);
            var t2 = CreateTriangle(srid.Factory, amersfoortRD.Offset(20, 50000).ToCoordinate(), 5000);
            var t3 = CreateTriangle(srid.Factory, amersfoortRD.ToCoordinate(), 100000);


            Assert.AreEqual(300000, Math.Round(t3.ExteriorRing.Length, 5)); // Pythagoras. By definition

            Assert.AreEqual(65908.18, Math.Round(t1.Distance(t2), 2), "Distance pythagoras"); // Pythagoras
            Assert.AreEqual(65913.62, Math.Round(t1.MeterDistance(t2).Value, 2), "Geo distance"); // Via NL CRS



            Assert.AreEqual(300024.180, Math.Round(t3.ExteriorRing.MeterLength().Value, 3));




            Assert.IsTrue(t1.IsWithinMeterDistance(t2, 70000).Value);
            Assert.IsFalse(t2.IsWithinMeterDistance(t1, 64000).Value);


            Assert.IsTrue(t1.Centroid.Coordinate.Equals3D(t1.Centroid.Coordinate));

            Assert.AreEqual(4330127018.92, Math.Round(t3.Area, 2));

            // Test the raw (signed) operations to return the same thing. On polygon the value is taken absolute (unsigned)
            Assert.AreEqual(4330127018.92, Math.Round(NetTopologySuite.Algorithm.Area.OfRingSigned(t3.ExteriorRing.Coordinates), 2));
            Assert.AreEqual(4330957964.64, Math.Round(srid.CRS.DistanceTransform.GeoArea(t3.Coordinates), 2));

            Assert.AreEqual(4330957964.64, Math.Round(t3.MeterArea().Value, 2)); // Not using backing data yet
        }

        [TestMethod]
        public void ReprojectViaSequence()
        {
            PPoint amersfoortRD = new PPoint(155000, 463000);

            var srid = SridRegister.GetById(Epsg.Netherlands);
            var t = CreateTriangle(srid.Factory, amersfoortRD.Offset(-50000, 0).ToCoordinate(), 5000);

            CoordinateSequence cs = srid.Factory.CoordinateSequenceFactory.Create(t.Coordinates);
            CoordinateSequence pcs = new PackedCoordinateSequenceFactory().Create(t.Coordinates);

            var packedFactory = new GeometryFactory(srid.Factory.PrecisionModel, srid.Factory.SRID, new PackedCoordinateSequenceFactory());

            Polygon p_cs = srid.Factory.CreatePolygon(cs);
            Polygon p_pcs = packedFactory.CreatePolygon(pcs);

            Assert.IsNotNull(p_cs.Reproject(SridRegister.GetById(Epsg.BelgiumLambert)));
            Assert.IsNotNull(p_pcs.Reproject(SridRegister.GetById(Epsg.BelgiumLambert)));
        }
    }
}
