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
        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, CoordinateOperation operation, GeometryFactory factory)
            where TGeometry : Geometry
        {
            return SridRegister.Reproject(geometry, factory,
                sq => factory.CoordinateSequenceFactory.Create(sq.ToCoordinateArray().Select(x => operation.Transform(x)).ToArray()));
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
            using (CoordinateOperation co = CoordinateOperation.Create(srcItem, crs, pc))
            {
                return Reproject(geometry, co, dstItem.Factory);
            }
        }

        public static bool IsNanOrInfinite(this double value)
        {
            return double.IsInfinity(value) || double.IsNaN(value);
        }

        public static Coordinate Transform(this CoordinateOperation op, Coordinate c)
        {
            return ToCoordinate(op.Transform(FromCoordinate(c)));
        }

        public static Coordinate Round(this Coordinate coord, int decimals)
        {
            if (coord is CoordinateZ || coord is CoordinateM) // Includes CoordinateZM
            {
                return new CoordinateZM(Math.Round(coord.X, decimals),
                                        Math.Round(coord.Y, decimals),
                                        coord.Z.IsNanOrInfinite() ? coord.Z : Math.Round(coord.Z, decimals),
                                        coord.M.IsNanOrInfinite() ? coord.M : Math.Round(coord.M, decimals));
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
            return (coordinate as CoordinateM)?.M;
        }

        private static Coordinate ToCoordinate(ProjCoordinate p)
        {
            return new CoordinateZ(p.X, p.Y, p.Z);
        }

        private static ProjCoordinate FromCoordinate(Coordinate coordinate)
        {
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
