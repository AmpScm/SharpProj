using System;
using System.Collections.Generic;
using System.Linq;
using NetTopologySuite.Geometries;
using SparpProj.NetTopologySuite;

namespace SharpProj
{
    public static class NtsExtensions
    {
        public static Coordinate ToCoordinate(this PPoint p)
        {
            if (!p.HasValues)
                return null; //

            switch (p.Axis)
            {
                case 1:
                case 2:
                    return new Coordinate(p.X, p.Y);
                case 3:
                    return new CoordinateZ(p.X, p.Y, p.Z);
                case 4:
                default:
                    return new CoordinateZM(p.X, p.Y, p.Z, p.T);
            }
        }
        public static PPoint ToPPoint(this Coordinate coordinate)
        {
            if ((object)coordinate == null)
                return new PPoint(double.NaN, double.NaN);
            else if (double.IsNaN(coordinate.Z))
                return new PPoint(coordinate.X, coordinate.Y);
            if (coordinate is CoordinateZM)
                return new PPoint(coordinate.X, coordinate.Y, coordinate.Z, coordinate.M);
            else
                return new PPoint(coordinate.X, coordinate.Y, coordinate.Z);
        }


        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, CoordinateTransform operation, GeometryFactory factory)
            where TGeometry : Geometry
        {
            return SridRegister.Reproject(geometry, factory,
                sq => factory.CoordinateSequenceFactory.Create(sq.ToCoordinateArray().Select(x => operation.Apply(x)).ToArray()));
        }

        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, CoordinateReferenceSystem crs)
            where TGeometry : Geometry
        {
            if (geometry == null)
                return null;

            int srcSRID = geometry.SRID;
            if (srcSRID == 0)
                throw new ArgumentOutOfRangeException(nameof(geometry), "Geometry doesn't have valid srid");



            SridItem srcItem = SridRegister.GetByValue(srcSRID);
            SridItem dstItem = SridRegister.FindEnsured(crs);

            using (ProjContext pc = crs.Context.Clone()) // Make thread-safe. Use settings from crs
            using (CoordinateTransform co = CoordinateTransform.Create(srcItem, crs, pc))
            {
                return Reproject(geometry, co, dstItem.Factory);
            }
        }

        public static Coordinate Apply(this CoordinateTransform op, Coordinate c)
        {
            return op.Apply(c.ToPPoint()).ToCoordinate();
        }

        public static Coordinate ApplyReversed(this CoordinateTransform op, Coordinate c)
        {
            return op.ApplyReversed(c.ToPPoint()).ToCoordinate();
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
        /// <param name="c1"></param>
        /// <param name="c2"></param>
        /// <returns>The distance in meters or <see cref="double.NaN"/> if the value can't be calculated</returns>
        public static double GeoDistance(this CoordinateTransform operation, IEnumerable<Coordinate> coordinates)
        {
            return operation.GeoDistance(coordinates.Select(x=>x.ToPPoint()));
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
        /// <param name="c1"></param>
        /// <param name="c2"></param>
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
