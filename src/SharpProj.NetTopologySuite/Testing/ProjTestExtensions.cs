using System;
using NetTopologySuite.Geometries;

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
        /// Converts Coordinate to X and Y axis (dropping additional ordinates)
        /// </summary>
        /// <param name="coord"></param>
        /// <returns></returns>
        public static Coordinate ToXY(this Coordinate coord)
        {
            return new Coordinate(coord.X, coord.Y);
        }

        /// <summary>
        /// Converts Coordinate to X and Y axis (dropping additional ordinates), rounding to <paramref name="decimals"/> decimals.
        /// </summary>
        /// <param name="coord"></param>
        /// <param name="decimals"></param>
        /// <returns></returns>
        public static Coordinate ToXY(this Coordinate coord, int decimals)
        {
            return new Coordinate(Math.Round(coord.X, decimals),
                                  Math.Round(coord.Y, decimals));
        }

        /// <summary>
        /// Gets a double value where the year is the whole number and month are encoded in the fractions of the year (Via <see cref="DateTime.DayOfYear"/>)
        /// </summary>
        /// <param name="dt"></param>
        /// <returns></returns>
        public static double ToYearValue(this DateTime dt)
        {
            int year = dt.Year;
            int daysInYear = DateTime.IsLeapYear(year) ? 366 : 365;
            return year + 1.0 / daysInYear * (dt.DayOfYear - 1);
        }
    }
}
