using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NetTopologySuite.Geometries;

namespace ProjSharp
{
    public static class ProjSharpExtensions
    {
        public static TGeometry Reproject<TGeometry>(this TGeometry geometry, CoordinateOperation operation, GeometryFactory factory)
            where TGeometry : Geometry
        {
            GeometryFactory f = new GeometryFactory();

            Coordinate[] coords = geometry.Coordinates;


            //coords.

            
            return geometry;
        }
    }
}
