using System;
using NetTopologySuite.Geometries;
using SharpProj;

namespace SharpProj.Testing
{
    /// <summary>
    /// 
    /// </summary>
    public static class ProjTestExtensions
    {
        /// <summary>
        /// Rounds all values of the coordinate to <paramref name="decimals"/> decimals.
        /// </summary>
        /// <param name="coord"></param>
        /// <param name="decimals"></param>
        /// <returns></returns>
        public static Coordinate RoundAll(this Coordinate coord, int decimals)
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

        /// <summary>
        /// Rounds all values of the coordinate to <paramref name="decimals"/> decimals.
        /// </summary>
        /// <param name="coord"></param>
        /// <param name="decimals"></param>
        /// <returns></returns>
        public static Coordinate RoundXY(this Coordinate coord, int decimals)
        {
            if (coord is CoordinateZ)
            {
                if (coord is CoordinateZM)
                {
                    return new CoordinateZM(
                                    Math.Round(coord.X, decimals),
                                    Math.Round(coord.Y, decimals),
                                    coord.Z,
                                    coord.M);
                }
                else
                {
                    return new CoordinateZ(
                                    Math.Round(coord.X, decimals),
                                    Math.Round(coord.Y, decimals),
                                    coord.Z);
                }
            }
            else if (coord is CoordinateM)
            {
                return new CoordinateM(
                                    Math.Round(coord.X, decimals),
                                    Math.Round(coord.Y, decimals),
                                    coord.M);
            }
            else
            {
                return new Coordinate(Math.Round(coord.X, decimals),
                                      Math.Round(coord.Y, decimals));
            }
        }

        /// <summary>
        /// Rounds all values of the coordinate to <paramref name="decimals"/> decimals.
        /// </summary>
        /// <param name="p"></param>
        /// <param name="decimals"></param>
        /// <returns></returns>
        public static PPoint RoundAll(this PPoint p, int decimals)
        {
            PPoint pc = new PPoint(
                Math.Round(p.X, decimals),
                Math.Round(p.Y, decimals),
                Math.Round(p.Z, decimals),
                Math.Round(p.T, decimals));
            pc.Axis = p.Axis;
            return pc;
        }

        /// <summary>
        /// Rounds the X and Y values to the specified number of decimals
        /// </summary>
        /// <param name="p"></param>
        /// <param name="decimals"></param>
        /// <returns></returns>
        public static PPoint RoundXY(this PPoint p, int decimals)
        {
            PPoint pc = new PPoint(
                Math.Round(p.X, decimals),
                Math.Round(p.Y, decimals),
                p.Z,
                p.T);
            pc.Axis = p.Axis;
            return pc;
        }
    }
}
