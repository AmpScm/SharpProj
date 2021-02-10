using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using NetTopologySuite.Geometries;
using SparpProj.NetTopologySuite;

namespace SharpProj
{
    public static class NtsExtensions
    {
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

        public static bool IsNanOrInfinite(this double value)
        {
            return double.IsInfinity(value) || double.IsNaN(value);
        }

        public static Coordinate Apply(this CoordinateTransform op, Coordinate c)
        {
            return op.Apply(FromCoordinate(c)).ToCoordinate();
        }

        public static Coordinate ToCoordinate(this ProjCoordinate p)
        {
            switch (p.Axis)
            {
                case 1:
                case 2:
                    return new Coordinate(p.X, p.Y);
                case 3:
                    // TODO: Check TargetCRS.Axis[2] to see if we have a Z or an M
                    return new CoordinateZ(p.X, p.Y, p.Z);
                case 4:
                default:
                    return new CoordinateZM(p.X, p.Y, p.Z, p.T);
            }
        }

        public static Coordinate ApplyReversed(this CoordinateTransform op, Coordinate c)
        {
            return op.ApplyReversed(FromCoordinate(c)).ToCoordinate();
        }

        public static Coordinate Round(this Coordinate coord, int decimals)
        {
            if (coord is CoordinateZ)
            {
                if (coord is CoordinateZM)
                {
                    return new CoordinateZM(
                                    Math.Round(coord.X, decimals),
                                    Math.Round(coord.Y, decimals),
                                    Math.Round(coord.Z, decimals),
                                    Math.Round(coord.M, decimals));
                }
                else
                {
                    return new CoordinateZ(
                                    Math.Round(coord.X, decimals),
                                    Math.Round(coord.Y, decimals),
                                    Math.Round(coord.Z, decimals));
                }
            }
            else if (coord is CoordinateM)
            {
                return new CoordinateM(
                                    Math.Round(coord.X, decimals),
                                    Math.Round(coord.Y, decimals),
                                    Math.Round(coord.M, decimals));
            }
            else
            {
                return new Coordinate(Math.Round(coord.X, decimals),
                                      Math.Round(coord.Y, decimals));
            }
        }

        public static double? GetZ(this Coordinate coordinate)
        {
            return (coordinate as CoordinateZ)?.Z;
        }

        public static double? GetM(this Coordinate coordinate)
        {
            return (coordinate as CoordinateM)?.M ?? (coordinate as CoordinateZM)?.M;
        }

        private static ProjCoordinate FromCoordinate(Coordinate coordinate)
        {
            if (double.IsNaN(coordinate.Z))
                return new ProjCoordinate(coordinate.X, coordinate.Y);
            if (coordinate is CoordinateZM)
                return new ProjCoordinate(coordinate.X, coordinate.Y, coordinate.Z, coordinate.M);
            else
                return new ProjCoordinate(coordinate.X, coordinate.Y, coordinate.GetZ() ?? coordinate.GetM() ?? 0);
        }

        internal static Disposer WithReadLock(this ReaderWriterLockSlim rwls)
        {
            rwls.EnterReadLock();

            return new Disposer(() => rwls.ExitReadLock());
        }

        internal static Disposer WithWriteLock(this ReaderWriterLockSlim rwls)
        {
            rwls.EnterWriteLock();

            return new Disposer(() => rwls.ExitWriteLock());
        }

        public static bool Equals3D(this Coordinate coordinate, Coordinate other)
        {
            if (coordinate is CoordinateZ z1 && other is CoordinateZ z2)
                return z1.Equals3D(z2);
            else if (double.IsNaN(coordinate.Z) && double.IsNaN(other.Z))
                return coordinate.Equals2D(other);
            else
                return false;
        }

        public static bool Equals3D(this Coordinate coordinate, Coordinate other, double tolerance)
        {
            if (!coordinate.Equals2D(other, tolerance))
                return false;
            else if (coordinate is CoordinateZ z1 && other is CoordinateZ z2)
                return z1.EqualInZ(z2, tolerance);
            else if (double.IsNaN(coordinate.Z) && double.IsNaN(other.Z))
                return true;
            else
                return false;
        }

        public static double EllipsoidDistance(this CoordinateTransform operation, Coordinate c1, Coordinate c2)
        {
            if (operation.DegreeInput || operation.SourceCRS?.Axis?[0].UnitName == "degree")
            {
                return operation.EllipsoidDistance(FromCoordinate(c1).DegToRad(), FromCoordinate(c2).DegToRad());
            }
            else
            {
                return operation.EllipsoidDistance(FromCoordinate(c1), FromCoordinate(c2));
            }
        }

        public static double EllipsoidDistanceZ(this CoordinateTransform operation, Coordinate c1, Coordinate c2)
        {
            if (operation.DegreeInput)
            {
                return operation.EllipsoidDistanceZ(FromCoordinate(c1).DegToRad(), FromCoordinate(c2).DegToRad());
            }
            else
            {
                return operation.EllipsoidDistanceZ(FromCoordinate(c1), FromCoordinate(c2));
            }
        }

        internal sealed class Disposer : IDisposable
        {
            Action _dispose;

            public Disposer(Action onDispose)
            {
                _dispose = onDispose;
            }

            public void Dispose()
            {
                try
                {
                    _dispose?.Invoke();
                }
                finally
                {
                    _dispose = null;
                }
            }
        }
    }

}
