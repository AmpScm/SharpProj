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
    public class PPointTests
    {
        [TestMethod]
        public void BasicPoint()
        {
            Assert.AreEqual("X=1", new PPoint(1).ToString());
            Assert.AreEqual("X=1, Y=2", new PPoint(1, 2).ToString());
            Assert.AreEqual("X=1, Y=2, Z=3", new PPoint(1, 2, 3).ToString());
            Assert.AreEqual("X=1, Y=2, Z=3, T=4", new PPoint(1, 2, 3, 4).ToString());

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
            Assert.AreEqual(3, new PPoint(0, 1, 2, 0).Axis);
            Assert.AreEqual(4, new PPoint(0, 1, 2, 3).Axis);

            Assert.AreEqual(1, new PPoint(new double[] { 0 }).Axis);
            Assert.AreEqual(2, new PPoint(new double[] { 0, 1 }).Axis);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2 }).Axis);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2, double.PositiveInfinity }).Axis);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2, 0 }).Axis);
            Assert.AreEqual(4, new PPoint(new double[] { 0, 1, 2, 3 }).Axis);

            Assert.AreEqual("X=1, T=4", new PPoint(1) { T = 4 }.ToString());
            Assert.AreEqual("X=1, Y=2, T=4", new PPoint(1, 2) { T = 4 }.ToString());
            Assert.AreEqual("X=1, Y=2, Z=3, T=4", new PPoint(1, 2, 3) { T = 4 }.ToString());

            Assert.AreEqual(double.PositiveInfinity, new PPoint(0).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(0, 1).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(0, 1, 2).T);
            Assert.AreEqual(3, new PPoint(0, 1, 2, 3).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(new double[] { 0 }).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(new double[] { 0, 1 }).T);
            Assert.AreEqual(double.PositiveInfinity, new PPoint(new double[] { 0, 1, 2 }).T);
            Assert.AreEqual(3, new PPoint(new double[] { 0, 1, 2, 3 }).T);
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
    }
}
