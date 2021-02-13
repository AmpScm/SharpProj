using System;
using SharpProj;
using SparpProj.NetTopologySuite;

using DistanceOp = NetTopologySuite.Operation.Distance.DistanceOp;

namespace NetTopologySuite.Geometries
{
    public static class SharpProjNtsExtensions
    {
        /// <summary>
        /// Returns the minimum distance between this Geometry <paramref name="g0"/> and another Geometry <paramref name="g1"/>, by calculating the nearest
        /// points in NTS and then asking (Sharp)Proj to get the actual distance in meters.
        /// </summary>
        /// <param name="g0">Geometry 1</param>
        /// <param name="g1">Geometry 2</param>
        /// <returns>The distance in meters between g0 and g1, or null if unable to calculate</returns>
        /// <exception cref="ArgumentNullException">g0 or g1 is null</exception
        /// <exception cref="ArgumentOutOfRangeException">The SRID of g0 and g1 don't match, are 0 or can't be resolved using <see cref="SridRegister"/></exception>
        /// <exception cref="ArgumentException"></exception>
        public static double? MeterDistance(this Geometry g0, Geometry g1)
        {
            if (g0 == null)
                throw new ArgumentNullException(nameof(g0));
            else if (g1 == null)
                throw new ArgumentNullException(nameof(g1));

            int srid = g0.SRID;
            if (srid == 0 || g1.SRID != g0.SRID)
                throw new ArgumentOutOfRangeException("SRID is 0 or doesn't match");

            SridItem sridItem;
            try
            {
                sridItem = SridRegister.GetByValue(srid);
            }
            catch (IndexOutOfRangeException sridExcepton)
            {
                throw new ArgumentOutOfRangeException("SRID not resolveable", sridExcepton);
            }

            using (var dt = sridItem.CRS.DistanceTransform.Clone()) // Thread safe with clone
            {
                return g0.MeterDistance(g1, dt);
            }
        }

        private static double? MeterDistance(this Geometry g0, Geometry g1, CoordinateTransform dt)
        {
            DistanceOp distanceOp = new DistanceOp(g0, g1);
            Coordinate[] nearestPoints = distanceOp.NearestPoints();

            double d = dt.GeoDistance(nearestPoints[0].ToPPoint(), nearestPoints[1].ToPPoint());

            if (double.IsInfinity(d) || double.IsNaN(d))
                return null;
            else
                return d;
        }

        /// <summary>
        /// Returns a boolean indicating whether this Geometry <paramref name="g0"/> is within <paramref name="distanceInMeter"/> meters
        /// of Geometry <paramref name="g1"/>.
        /// </summary>
        /// <param name="g0">Geometry 1</param>
        /// <param name="g1">Geometry 2</param>
        /// <param name="distanceInMeter">The distance limit</param>
        /// <returns>true if the geometries are within distance, false if not and NULL if unable to calculate</returns>
        /// <exception cref="ArgumentNullException">g0 or g1 is null</exception
        /// <exception cref="ArgumentOutOfRangeException">The SRID of g0 and g1 don't match, are 0 or can't be resolved using <see cref="SridRegister"/></exception>
        /// <exception cref="ArgumentException"></exception>
        public static bool? IsWithinMeterDistance(this Geometry g0, Geometry g1, double distanceInMeter)
        {
            if (g0 == null)
                throw new ArgumentNullException(nameof(g0));
            else if (g1 == null)
                throw new ArgumentNullException(nameof(g1));

            int srid = g0.SRID;
            if (srid == 0 || g1.SRID != g0.SRID)
                throw new ArgumentOutOfRangeException("SRID is 0 or doesn't match");

            DistanceOp distanceOp = new DistanceOp(g0, g1);
            Coordinate[] nearestPoints = distanceOp.NearestPoints();

            SridItem sridItem;
            try
            {
                sridItem = SridRegister.GetByValue(srid);
            }
            catch (IndexOutOfRangeException sridExcepton)
            {
                throw new ArgumentOutOfRangeException("SRID not resolveable", sridExcepton);
            }

            // TODO: There should be possible optimizations now, with knowledg of the CRS
            //       Maybe we can assume that this is always false if the NTS cut-off thinks it's false
            //       Not 100% sure and we want correctness

            using (var dt = sridItem.CRS.DistanceTransform.Clone()) // Thread safe with clone
            {
                double d = dt.GeoDistance(nearestPoints[0].ToPPoint(), nearestPoints[1].ToPPoint());

                if (double.IsInfinity(d) || double.IsNaN(d))
                    return null;
                else
                    return (d <= distanceInMeter);
            }
        }

        public static bool Equals3D(this Coordinate coordinate, Coordinate other)
        {
            if (coordinate is null || other is null)
                return (coordinate is null == other is null);

            if (coordinate is CoordinateZ z1 && other is CoordinateZ z2)
                return z1.Equals3D(z2);
            else
                return coordinate.Equals2D(other)
                    && double.IsNaN(coordinate.Z) == double.IsNaN(other.Z);
        }

        public static bool Equals3D(this Coordinate coordinate, Coordinate other, double tolerance)
        {
            if (coordinate is null || other is null)
                return (coordinate is null == other is null);

            if (coordinate is CoordinateZ z1 && other is CoordinateZ z2)
                return z1.Equals3D(z2, tolerance);
            else
                return coordinate.Equals2D(other)
                    && double.IsNaN(coordinate.Z) == double.IsNaN(other.Z);
        }

        /// <summary>
        /// Returns the minimum distance between this Geometry <paramref name="g0"/> and another Geometry <paramref name="g1"/>, by calculating the nearest
        /// points in NTS and then asking (Sharp)Proj to get the actual distance in meters.
        /// </summary>
        /// <param name="g0">Geometry 1</param>
        /// <param name="g1">Geometry 2</param>
        /// <returns>The distance in meters between g0 and g1, or null if unable to calculate</returns>
        /// <exception cref="ArgumentNullException">g0 or g1 is null</exception
        /// <exception cref="ArgumentOutOfRangeException">The SRID of g0 and g1 don't match, are 0 or can't be resolved using <see cref="SridRegister"/></exception>
        /// <exception cref="ArgumentException"></exception>
        public static double? MeterLength(this LineString l)
        {
            if (l == null)
                throw new ArgumentNullException(nameof(l));

            int srid = l.SRID;
            if (srid == 0)
                throw new ArgumentOutOfRangeException("SRID is 0 or doesn't match");


            SridItem sridItem;
            try
            {
                sridItem = SridRegister.GetByValue(srid);
            }
            catch (IndexOutOfRangeException sridExcepton)
            {
                throw new ArgumentOutOfRangeException("SRID not resolveable", sridExcepton);
            }

            using (var dt = sridItem.CRS.DistanceTransform.Clone()) // Thread safe with clone
            {
                return l.MeterLength(dt);
            }
        }

        private static double? MeterLength(this LineString l, CoordinateTransform dt)
        {
            double distance = 0;
            var cs = l.CoordinateSequence;
            Coordinate last = cs.GetCoordinate(0);


            for (int i = 1; i < cs.Count; i++)
            {
                var c = cs.GetCoordinate(i);

                double d = dt.GeoDistance(last, c);
                if (double.IsNaN(d))
                    return null;

                distance += d;
                last = c;
            }

            return distance;
        }

        public static double? MeterArea(this Polygon p)
        {
            if (p == null)
                throw new ArgumentNullException(nameof(p));

            int srid = p.SRID;
            if (srid == 0)
                throw new ArgumentOutOfRangeException("SRID is 0 or doesn't match");


            SridItem sridItem;
            try
            {
                sridItem = SridRegister.GetByValue(srid);
            }
            catch (IndexOutOfRangeException sridExcepton)
            {
                throw new ArgumentOutOfRangeException("SRID not resolveable", sridExcepton);
            }


            using (var dt = sridItem.CRS.DistanceTransform.Clone())
            {
                return MeterArea(p, dt);
            }
        }

        private static double? MeterArea(this Polygon p, CoordinateTransform dt)
        {
            double? s = SignedRingArea(p.ExteriorRing, dt);

            if (!s.HasValue)
                return null;

            foreach(LineString ls in p.InteriorRings)
            {
                double? s2 = SignedRingArea(ls, dt);

                if (!s2.HasValue)
                    return null;

                s += s2;
            }

            return s;
        }

        private static double? SignedRingArea(LineString ring, CoordinateTransform dt)
        {
            return Algorithm.Area.OfRingSigned(ring.CoordinateSequence);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="gc"></param>
        /// <returns></returns>
        public static double? MeterLength(this GeometryCollection gc)
        {
            if (gc.Count == 0)
                return null;

            int srid = gc.SRID;
            if (srid == 0)
                throw new ArgumentOutOfRangeException("SRID is 0 or doesn't match");


            SridItem sridItem;
            try
            {
                sridItem = SridRegister.GetByValue(srid);
            }
            catch (IndexOutOfRangeException sridExcepton)
            {
                throw new ArgumentOutOfRangeException("SRID not resolveable", sridExcepton);
            }

            using (var dt = sridItem.CRS.DistanceTransform.Clone()) // Thread safe with clone
            {
                return gc.MeterLength(dt);
            }
        }

        private static double? MeterLength(this GeometryCollection gc, CoordinateTransform dt)
        {
            double sum = 0;
            
            foreach (Geometry g in gc)
            {
                double? s = (g as LineString)?.MeterLength(dt) ?? (g as GeometryCollection)?.MeterLength(dt);

                if (!s.HasValue)
                    return null;

                sum += s.Value;
            }
            return sum;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="gc"></param>
        /// <returns></returns>
        public static double? MeterArea(this GeometryCollection gc)
        {
            if (gc.Count == 0)
                return null;

            int srid = gc.SRID;
            if (srid == 0)
                throw new ArgumentOutOfRangeException("SRID is 0 or doesn't match");

            SridItem sridItem;
            try
            {
                sridItem = SridRegister.GetByValue(srid);
            }
            catch (IndexOutOfRangeException sridExcepton)
            {
                throw new ArgumentOutOfRangeException("SRID not resolveable", sridExcepton);
            }

            using (var dt = sridItem.CRS.DistanceTransform.Clone()) // Thread safe with clone
            {
                return gc.MeterArea(dt);
            }
        }

        private static double? MeterArea(this GeometryCollection gc, CoordinateTransform dt)
        {
            double sum = 0.0;

            // TODO: Optimize by fetching SRID only once
            foreach (Geometry g in gc)
            {
                double? s = (g as Polygon)?.MeterArea(dt) ?? (g as GeometryCollection)?.MeterArea(dt);

                if (!s.HasValue)
                    return null;

                sum += s.Value;
            }
            return sum;
        }
    }
}
