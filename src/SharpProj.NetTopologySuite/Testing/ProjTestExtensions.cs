using System;
using NetTopologySuite.Geometries;
using SharpProj;

namespace SparpProj.NetTopologySuite.Testing
{
    public static class ProjTestExtensions
    {
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
