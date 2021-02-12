
using System;
using NetTopologySuite.Geometries;

namespace SparpProj.NetTopologySuite.Utils
{

    public class GeometryTransformer : global::NetTopologySuite.Geometries.Utilities.GeometryTransformer
    {

        public new Geometry InputGeometry { get; protected set; }

        protected new GeometryFactory Factory
        {
            get => base.Factory;
            set => base.Factory = value;
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="inputGeom"></param>
        /// <returns></returns>
        public new Geometry Transform(Geometry inputGeom)
        {
            return Transform(inputGeom, inputGeom.Factory);
        }

        public Geometry Transform(Geometry inputGeom, GeometryFactory factory)
        {
            InputGeometry = inputGeom;
            Factory = factory;
            return DoTransform(InputGeometry, null);
        }

        public virtual Geometry DoTransform(Geometry inputGeom, Geometry parentGeometry)
        {
            if (inputGeom is Point point)
                return TransformPoint(point, parentGeometry);
            else if (inputGeom is MultiPoint multipoint)
                return TransformMultiPoint(multipoint, parentGeometry);
            else if (inputGeom is LinearRing linearring)
                return TransformLinearRing(linearring, parentGeometry);
            else if (inputGeom is LineString linestring)
                return TransformLineString(linestring, parentGeometry);
            else if (inputGeom is MultiLineString multilinestring)
                return TransformMultiLineString(multilinestring, parentGeometry);
            else if (inputGeom is Polygon polygon)
                return TransformPolygon(polygon, parentGeometry);
            else if (inputGeom is MultiPolygon multipolygon)
                return TransformMultiPolygon(multipolygon, parentGeometry);
            else if (inputGeom is GeometryCollection geometryCollection)
                return TransformGeometryCollection(geometryCollection, parentGeometry);

            throw new ArgumentException("Unknown Geometry subtype: " + inputGeom.GetType().FullName);
        }
    }
}