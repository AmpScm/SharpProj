using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NetTopologySuite.Geometries;

namespace SharpProj.Tests
{
    [TestClass]
    public class PPointTests
    {
        [TestMethod]
        public void BasicPoint()
        {
            Assert.AreEqual("X=1", new PPoint(1).ToString());
            Assert.AreEqual("X=1, Y=2", new PPoint(1, 2).ToString("G", CultureInfo.InvariantCulture));
            Assert.AreEqual("X=1, Y=2, Z=3", new PPoint(1, 2, 3).ToString("G", CultureInfo.InvariantCulture));
            Assert.AreEqual("X=1, Y=2, Z=3, T=4", new PPoint(1, 2, 3, 4).ToString("G", CultureInfo.InvariantCulture));

            Assert.AreEqual(new PPoint(0), new PPoint(0, 0));
            Assert.AreEqual(new PPoint(0, 1), new PPoint(0, 1));
            Assert.AreEqual(new PPoint(0, 1), new PPoint(0, 1, 0));
            Assert.AreEqual(new PPoint(0, 1), new PPoint(0, 1, 0, double.PositiveInfinity));
            Assert.AreNotEqual(new PPoint(0, 1), new PPoint(0, 1, 0, 0));

            Assert.AreEqual(new PPoint(0, 1, 2, 3), new PPoint(0, 1, 2, 3));

            Assert.AreEqual(1, new PPoint(0).Axis);
            Assert.AreEqual(2, new PPoint(0, 1).Axis);
            Assert.AreEqual(3, new PPoint(0, 1, 2).Axis);
            Assert.AreEqual(3, new PPoint(0, 1, 2, double.PositiveInfinity).Axis);
            Assert.AreEqual(4, new PPoint(0, 1, 2, 0).Axis);
            Assert.AreEqual(4, new PPoint(0, 1, 2, 3).Axis);

            Assert.AreEqual(1, new PPoint(new double[] { 0 }).Axis);
            Assert.AreEqual(2, new PPoint(new double[] { 0, 1 }).Axis);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2 }).Axis);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2, double.PositiveInfinity }).Axis);
            Assert.AreEqual(4, new PPoint(new double[] { 0, 1, 2, 0 }).Axis);
            Assert.AreEqual(4, new PPoint(new double[] { 0, 1, 2, 3 }).Axis);

            Assert.AreEqual("X=1, T=4", new PPoint(1) { T = 4 }.ToString("G", CultureInfo.InvariantCulture));
            Assert.AreEqual("X=1, Y=2, T=4", new PPoint(1, 2) { T = 4 }.ToString("G", CultureInfo.InvariantCulture));
            Assert.AreEqual("X=1, Y=2, Z=3, T=4", new PPoint(1, 2, 3) { T = 4 }.ToString("G", CultureInfo.InvariantCulture));

            Assert.AreEqual(double.PositiveInfinity, new PPoint(0).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(0, 1).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(0, 1, 2).T);
            Assert.AreEqual(3, new PPoint(0, 1, 2, 3).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(new double[] { 0 }).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(new double[] { 0, 1 }).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(new double[] { 0, 1, 2 }).T);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2, 3 }).T);


            Assert.AreNotEqual(new PPoint(), new PPoint() { T = 0 });
            Assert.AreEqual(new PPoint(), new PPoint() { T = double.PositiveInfinity });
            Assert.AreNotEqual(new PPoint(), new PPoint().WithT(0));
            Assert.AreEqual(new PPoint(), new PPoint().WithT(double.PositiveInfinity));

            Assert.AreEqual(new PPoint(0, 0, 0, 0), new PPoint() { T = 0 });
            Assert.AreNotEqual(new PPoint(0, 0, 0, 0), new PPoint() { T = double.PositiveInfinity });
            Assert.AreEqual(new PPoint(0, 0, 0, 0), new PPoint().WithT(0));
            Assert.AreNotEqual(new PPoint(0, 0, 0, 0), new PPoint().WithT(double.PositiveInfinity));

            Assert.AreNotEqual(new PPoint(0, 0, 0, double.PositiveInfinity), new PPoint() { T = 0 });
            Assert.AreEqual(new PPoint(0, 0, 0, double.PositiveInfinity), new PPoint() { T = double.PositiveInfinity });
            Assert.AreNotEqual(new PPoint(0, 0, 0, double.PositiveInfinity), new PPoint().WithT(0));
            Assert.AreEqual(new PPoint(0, 0, 0, double.PositiveInfinity), new PPoint().WithT(double.PositiveInfinity));
        }

        [TestMethod]
        public void PointToCoordinate()
        {
            Assert.AreEqual("", CoordinateEqual(new Coordinate(0, 1)));
            Assert.AreEqual("", CoordinateEqual(new CoordinateZ(0, 1, 2)));
            Assert.AreEqual("", CoordinateEqual(new CoordinateM(0, 1, 2)));
            Assert.AreEqual("", CoordinateEqual(new CoordinateZM(0, 1, 2, 3)));
        }

        private string CoordinateEqual(Coordinate coordinate)
        {
            Coordinate c = coordinate.ToPPoint().ToCoordinate();

            if (c.GetType() != coordinate.GetType())
                return $"Type different: {c} vs {coordinate}";
            if (!DoubleEqual(c.X, coordinate.X))
                return $"X different: {c} vs {coordinate}";
            if (!DoubleEqual(c.Y, coordinate.Y))
                return $"Y different: {c} vs {coordinate}";
            if (!DoubleEqual(c.Z, coordinate.Z))
                return $"Z different: {c} vs {coordinate}";
            if (!DoubleEqual(c.M, coordinate.M))
                return $"M different: {c} vs {coordinate}";

            return "";
        }

        private bool DoubleEqual(double m1, double m2)
        {
            if (m1 == m2)
                return true;
            else if (double.IsNaN(m1) && double.IsNaN(m2))
                return true;
            else if (double.IsPositiveInfinity(m1) && double.IsPositiveInfinity(m2))
                return true;
            else if (double.IsNegativeInfinity(m1) && double.IsNegativeInfinity(m2))
                return true;
            return false;
        }

        [TestMethod]
        public void TestHasValue()
        {
            Assert.IsTrue(new PPoint().HasValues);
            Assert.IsTrue(new PPoint(0).HasValues);
            Assert.IsTrue(new PPoint(0, 0).HasValues);
            Assert.IsTrue(new PPoint(0, 0, 0, 0).HasValues);
            // If X is NaN or Infinity, there is no actual value... Proj error condition
            Assert.IsFalse(new PPoint(double.PositiveInfinity, 0, 0, 0).HasValues);
            Assert.IsFalse(new PPoint(double.NaN, 0, 0, 0).HasValues);
            Assert.IsFalse(new PPoint(double.PositiveInfinity, 1, 0, 0).HasValues);
            Assert.IsFalse(new PPoint(double.NaN, 0, 1, 0).HasValues);
        }

        [TestMethod]
        public void DMSTest()
        {
            Assert.AreEqual("9d7'12\"N, 11d7'48\"E", new PPoint(9.12, 11.13).DegToRad().ToStringDMS(CultureInfo.InvariantCulture));
            Assert.AreEqual("12d8'24\"S, 19d19'12\"W", new PPoint(-12.14, -19.32).DegToRad().ToStringDMS(CultureInfo.InvariantCulture));
            Assert.AreEqual("9d7'12\"a, 11d7'48\"c", new PPoint(9.12, 11.13).DegToRad().ToString("DMS:abcd", CultureInfo.InvariantCulture));
            Assert.AreEqual("12d8'24\"b, 19d19'12\"d", new PPoint(-12.14, -19.32).DegToRad().ToString("DMS:abcd", CultureInfo.InvariantCulture));

            Assert.IsTrue(PPoint.TryParse(new PPoint(9.12, 11.13).DegToRad().ToStringDMS(CultureInfo.InvariantCulture), "DMS", CultureInfo.InvariantCulture, out var p));
            Assert.AreEqual(new PPoint(9.12, 11.13), p.RadToDeg().ToXY(2));

            Assert.IsTrue(PPoint.TryParse(new PPoint(-77.6543, -67.003).DegToRad().ToStringDMS(CultureInfo.InvariantCulture), "DMS", CultureInfo.InvariantCulture, out p));
            Assert.AreEqual(new PPoint(-77.65, -67.0), p.RadToDeg().ToXY(2));
        }

        [TestMethod]
        public void TryParse()
        {
            Assert.IsTrue(PPoint.TryParse(new PPoint(12.13, 14.15, 15.16, 16.17).ToString("G", CultureInfo.InvariantCulture), null, CultureInfo.InvariantCulture, out var p));
            Assert.AreEqual(new PPoint(12.13, 14.15, 15.16, 16.17), p);
            Assert.IsTrue(PPoint.TryParse(new PPoint(12.13, 14.15, 15.16, 16.17).ToString("DMS", CultureInfo.InvariantCulture), "DMS", CultureInfo.InvariantCulture, out p));
            Assert.AreEqual(new PPoint(12.13, 14.15, 15.16, 16.17), p.ToXYZ(2).WithT(p.T));
            //Assert.AreEqual("9d7'12\"N, 11d7'48\"E", new PPoint(9.12, 11.13).DegToRad().ToStringDMS(CultureInfo.InvariantCulture));
            //Assert.AreEqual("12d8'24\"S, 19d19'12\"W", new PPoint(-12.14, -19.32).DegToRad().ToStringDMS(CultureInfo.InvariantCulture));
            //Assert.AreEqual("9d7'12\"a, 11d7'48\"c", new PPoint(9.12, 11.13).DegToRad().ToString("DMS:abcd", CultureInfo.InvariantCulture));
            //Assert.AreEqual("12d8'24\"b, 19d19'12\"d", new PPoint(-12.14, -19.32).DegToRad().ToString("DMS:abcd", CultureInfo.InvariantCulture));
            //
            //Assert.IsTrue(PPoint.TryParse(new PPoint(9.12, 11.13).DegToRad().ToStringDMS(CultureInfo.InvariantCulture), "DMS", CultureInfo.InvariantCulture, out var p));
            //Assert.AreEqual(new PPoint(9.12, 11.13), p.RadToDeg().ToXY(2));
            //
            //Assert.IsTrue(PPoint.TryParse(new PPoint(-77.6543, -67.003).DegToRad().ToStringDMS(CultureInfo.InvariantCulture), "DMS", CultureInfo.InvariantCulture, out p));
            //Assert.AreEqual(new PPoint(-77.65, -67.0), p.RadToDeg().ToXY(2));
        }
    }
}
