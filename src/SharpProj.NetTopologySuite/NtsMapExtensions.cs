using System;
using System.Collections.Generic;
using System.Linq;
using NetTopologySuite.Geometries;
using SharpProj.NTS;
using SharpProj.Utils.NTSAdditions;

namespace SharpProj
{
    /// <summary>
    /// 
    /// </summary>
    public static class NtsExtensions
    {
        /// <summary>
        /// Converts the <see cref="PPoint"/> to a <see cref="Coordinate"/>
        /// </summary>
        /// <param name="p"></param>
        /// <returns></returns>
        /// <remarks>Error PPoints are mapped to NULL</remarks>
        public static Coordinate ToCoordinate(this PPoint p)
        {
            if (!p.HasValues)
                return null; //

            if (!p.HasT)
                switch (p.Axis)
                {
                    case 1:
                    case 2:
                        return new Coordinate(p.X, p.Y);
                    case 3:
                    case 4:
                    default:
                        return new CoordinateZ(p.X, p.Y, p.Z);
                }
            else if (p.Axis <= 2)
                return new CoordinateM(p.X, p.Y, p.T);
            else
                return new CoordinateZM(p.X, p.Y, p.Z, p.T);
        }

        /// <summary>
        /// Converts a Coordinate to a PPoint
        /// </summary>
        /// <param name="coordinate"></param>
        /// <returns></returns>
        /// <remarks>An NULL Coordinate is converted to an Error PPoint (with NaN values).</remarks>
        public static PPoint ToPPoint(this Coordinate coordinate)
        {
            if ((object)coordinate == null)
                return new PPoint(double.NaN, double.NaN);

            if (double.IsNaN(coordinate.M))
            {
                if (double.IsNaN(coordinate.Z))
                    return new PPoint(coordinate.X, coordinate.Y);
                else
                    return new PPoint(coordinate.X, coordinate.Y, coordinate.Z);
            }
            else
            {
                if (double.IsNaN(coordinate.Z))
                    return new PPoint(coordinate.X, coordinate.Y) { T = coordinate.M };
                else
                    return new PPoint(coordinate.X, coordinate.Y, coordinate.Z) { T = coordinate.M };
            }
        }

        /// <summary>
        /// Converts sequence
        /// </summary>
        /// <param name="cs"></param>
        /// <returns></returns>
        public static IEnumerable<PPoint> ToPPoints(this CoordinateSequence cs)
        {
            if (cs.Dimension == 2)
            {
                for (int i = 0; i < cs.Count; i++)
                    yield return new PPoint(cs.GetX(i), cs.GetY(i));
            }
            else if (cs.Dimension == 3)
            {
                for (int i = 0; i < cs.Count; i++)
                    yield return new PPoint(cs.GetX(i), cs.GetY(i), cs.GetZ(i));
            }
            else
                for (int i = 0; i < cs.Count; i++)
                {
                    yield return cs.GetCoordinate(i).ToPPoint();
                }
        }

        /// <summary>
        /// Converts to PPoints
        /// </summary>
        /// <param name="cs"></param>
        /// <returns></returns>
        public static IEnumerable<PPoint> ToPPoints(this IEnumerable<Coordinate> cs)
        {
            foreach (Coordinate c in cs)
            {
                yield return c.ToPPoint();
            }
        }

        /// <summary>
        /// Converts sequence of points
        /// </summary>
        /// <param name="points"></param>
        /// <returns></returns>
        public static IEnumerable<Coordinate> ToCoordinates(this IEnumerable<PPoint> points)
        {
            foreach (var p in points)
            {
                yield return p.ToCoordinate();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="TGeometry"></typeparam>
        /// <param name="geometry"></param>
        /// <param name="toSrid"></param>
        /// <returns></returns>
        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, SridItem toSrid)
            where TGeometry : Geometry
        {
            if (geometry == null)
                return null;

            if (toSrid == null)
                throw new ArgumentNullException(nameof(toSrid));

            int srcSRID = geometry.SRID;
            if (srcSRID == 0)
                throw new ArgumentOutOfRangeException(nameof(geometry), "Geometry doesn't have valid srid");

            SridItem srcItem = SridRegister.GetByValue(srcSRID);

            using (ProjContext pc = toSrid.CRS.Context.Clone()) // Make thread-safe. Use settings from crs
            using (CoordinateTransform ct = CoordinateTransform.Create(srcItem, toSrid,
                new CoordinateTransformOptions { NoBallparkConversions = true },
                pc))
            {
                return Reproject(geometry, ct, toSrid.Factory);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="TGeometry"></typeparam>
        /// <param name="geometry"></param>
        /// <param name="toSRID"></param>
        /// <returns></returns>
        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, int toSRID)
            where TGeometry : Geometry
        {
            if (geometry == null)
                return null;

            SridItem toSridItem = SridRegister.GetByValue(toSRID);

            return Reproject(geometry, toSridItem);
        }

        /// <summary>
        /// Generic low-level reprojection. Used by the other methods
        /// </summary>
        /// <typeparam name="TGeometry"></typeparam>
        /// <param name="geometry"></param>
        /// <param name="operation"></param>
        /// <param name="factory"></param>
        /// <returns></returns>
        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, CoordinateTransform operation, GeometryFactory factory)
            where TGeometry : Geometry
        {
            // TODO: Add something to support custom geometry types (circles/arcs) as previously
            // implemented via the SridRegister
            var res = (TGeometry)factory.CreateGeometry(geometry);

            IEntireCoordinateSequenceFilter filter = new ReProjectFilter(operation, factory.PrecisionModel);

            res.Apply(filter);
            return res;
        }

        /// <summary>
        /// Wraps <see cref="CoordinateTransform.Apply(PPoint)"/> for NTS
        /// </summary>
        /// <param name="op"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public static Coordinate Apply(this CoordinateTransform op, Coordinate c)
        {
            return op.Apply(c.ToPPoint()).ToCoordinate();
        }

        /// <summary>
        /// Wraps <see cref="CoordinateTransform.ApplyReversed(PPoint)"/> for NTS
        /// </summary>
        /// <param name="op"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        public static Coordinate ApplyReversed(this CoordinateTransform op, Coordinate c)
        {
            return op.ApplyReversed(c.ToPPoint()).ToCoordinate();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="op"></param>
        /// <param name="xSpan"></param>
        /// <param name="xStep"></param>
        /// <param name="xCount"></param>
        /// <param name="ySpan"></param>
        /// <param name="yStep"></param>
        /// <param name="yCount"></param>
        /// <param name="zSpan"></param>
        /// <param name="zStep"></param>
        /// <param name="zCount"></param>
        /// <param name="tSpan"></param>
        /// <param name="tStep"></param>
        /// <param name="tCount"></param>
        public static unsafe void Apply(this CoordinateTransform op,
            Span<double> xSpan, int xStep, int xCount,
            Span<double> ySpan, int yStep, int yCount,
            Span<double> zSpan, int zStep, int zCount,
            Span<double> tSpan, int tStep, int tCount)
        {
            fixed (double* x0 = &xSpan.GetPinnableReference())
            fixed (double* y0 = &ySpan.GetPinnableReference())
            fixed (double* z0 = &zSpan.GetPinnableReference())
            fixed (double* t0 = &tSpan.GetPinnableReference())
            {
                op.Apply(
                    x0, xStep, xCount,
                    y0, yStep, yCount,
                    z0, zStep, zCount,
                    t0, tStep, tCount);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="op"></param>
        /// <param name="xSpan"></param>
        /// <param name="xStep"></param>
        /// <param name="xCount"></param>
        /// <param name="ySpan"></param>
        /// <param name="yStep"></param>
        /// <param name="yCount"></param>
        /// <param name="zSpan"></param>
        /// <param name="zStep"></param>
        /// <param name="zCount"></param>
        /// <param name="tSpan"></param>
        /// <param name="tStep"></param>
        /// <param name="tCount"></param>
        public static unsafe void ApplyReversed(this CoordinateTransform op,
            Span<double> xSpan, int xStep, int xCount,
            Span<double> ySpan, int yStep, int yCount,
            Span<double> zSpan, int zStep, int zCount,
            Span<double> tSpan, int tStep, int tCount)
        {
            fixed (double* x0 = &xSpan.GetPinnableReference())
            fixed (double* y0 = &ySpan.GetPinnableReference())
            fixed (double* z0 = &zSpan.GetPinnableReference())
            fixed (double* t0 = &tSpan.GetPinnableReference())
            {
                op.ApplyReversed(
                    x0, xStep, xCount,
                    y0, yStep, yCount,
                    z0, zStep, zCount,
                    t0, tStep, tCount);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="op"></param>
        /// <param name="xSpan"></param>
        /// <param name="xStep"></param>
        /// <param name="ySpan"></param>
        /// <param name="yStep"></param>
        /// <param name="zSpan"></param>
        /// <param name="zStep"></param>
        /// <param name="tSpan"></param>
        /// <param name="tStep"></param>
        public static void Apply(this CoordinateTransform op,
            Span<double> xSpan, int xStep,
            Span<double> ySpan, int yStep,
            Span<double> zSpan, int zStep,
            Span<double> tSpan, int tStep)
        {
            op.Apply(
                    xSpan, xStep, (xStep > 0) ? ((xSpan.Length + xStep - 1) / xStep) : 0,
                    ySpan, yStep, (yStep > 0) ? ((ySpan.Length + yStep - 1) / yStep) : 0,
                    zSpan, zStep, (zStep > 0) ? ((zSpan.Length + zStep - 1) / zStep) : 0,
                    tSpan, tStep, (tStep > 0) ? ((tSpan.Length + tStep - 1) / tStep) : 0);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="op"></param>
        /// <param name="xSpan"></param>
        /// <param name="xStep"></param>
        /// <param name="ySpan"></param>
        /// <param name="yStep"></param>
        /// <param name="zSpan"></param>
        /// <param name="zStep"></param>
        /// <param name="tSpan"></param>
        /// <param name="tStep"></param>
        public static void ApplyReversed(this CoordinateTransform op,
            Span<double> xSpan, int xStep,
            Span<double> ySpan, int yStep,
            Span<double> zSpan, int zStep,
            Span<double> tSpan, int tStep)
        {
            op.ApplyReversed(
                    xSpan, xStep, (xStep > 0) ? ((xSpan.Length + xStep - 1) / xStep) : 0,
                    ySpan, yStep, (yStep > 0) ? ((ySpan.Length + yStep - 1) / yStep) : 0,
                    zSpan, zStep, (zStep > 0) ? ((zSpan.Length + zStep - 1) / zStep) : 0,
                    tSpan, tStep, (tStep > 0) ? ((tSpan.Length + tStep - 1) / tStep) : 0);
        }

        /// <summary>
        /// Calculates the distance between the two coordinates (X,Y) in meters
        /// </summary>
        /// <param name="operation"></param>
        /// <param name="c1"></param>
        /// <param name="c2"></param>
        /// <returns>The distance in meters or <see cref="double.NaN"/> if the value can't be calculated</returns>
        public static double GeoDistance(this CoordinateTransform operation, Coordinate c1, Coordinate c2)
        {
            return operation.GeoDistance(c1.ToPPoint(), c2.ToPPoint());
        }

        /// <summary>
        /// Calculates the distance between the coordinates (X,Y) in meters
        /// </summary>
        /// <param name="operation"></param>
        /// <param name="coordinates"></param>
        /// <returns>The distance in meters or <see cref="double.NaN"/> if the value can't be calculated</returns>
        public static double GeoDistance(this CoordinateTransform operation, IEnumerable<Coordinate> coordinates)
        {
            return operation.GeoDistance(coordinates.ToPPoints());
        }

        /// <summary>
        /// Calculates the distance between the two coordinates (X,Y) in meters, and then applies Z (assumed to be meters) via Pythagoras
        /// </summary>
        /// <param name="operation"></param>
        /// <param name="c1"></param>
        /// <param name="c2"></param>
        /// <returns>The distance in meters or <see cref="double.NaN"/> if the value can't be calculated</returns>
        public static double GeoDistanceZ(this CoordinateTransform operation, Coordinate c1, Coordinate c2)
        {
            return operation.GeoDistanceZ(ToPPoint(c1), ToPPoint(c2));
        }

        /// <summary>
        /// Calculates the distance between the two coordinates (X,Y) in meters, and then applies Z (assumed to be meters) via Pythagoras
        /// </summary>
        /// <param name="operation"></param>
        /// <param name="coordinates"></param>
        /// <returns>The distance in meters or <see cref="double.NaN"/> if the value can't be calculated</returns>
        public static double GeoDistanceZ(this CoordinateTransform operation, IEnumerable<Coordinate> coordinates)
        {
            return operation.GeoDistanceZ(coordinates.Select(x => x.ToPPoint()));
        }

        /// <summary>
        /// Calculates the area in square meters occupied by the polygon described in coordinates
        /// </summary>
        /// <param name="operation"></param>
        /// <param name="coordinates"></param>
        /// <returns></returns>
        public static double GeoArea(this CoordinateTransform operation, IEnumerable<Coordinate> coordinates)
        {
            return operation.GeoArea(coordinates.Select(x => x.ToPPoint()));
        }
    }

}
