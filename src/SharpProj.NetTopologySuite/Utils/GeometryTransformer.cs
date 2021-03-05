
using System;
using NetTopologySuite.Geometries;

namespace SharpProj.Utils.NTSAdditions
{
    /// <summary>
    /// 
    /// </summary>
    [Obsolete("Use ReProjectFilter w/ Geometry.Apply")]
    public class GeometryTransformer : global::NetTopologySuite.Geometries.Utilities.GeometryTransformer
    {
        /// <summary>
        /// 
        /// </summary>
        public new Geometry InputGeometry { get; protected set; }

        /// <summary>
        /// 
        /// </summary>
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

        /// <summary>
        /// 
        /// </summary>
        /// <param name="inputGeom"></param>
        /// <param name="factory"></param>
        /// <returns></returns>
        public Geometry Transform(Geometry inputGeom, GeometryFactory factory)
        {
            InputGeometry = inputGeom;
            Factory = factory;
            return DoTransform(InputGeometry, null);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="inputGeom"></param>
        /// <param name="parentGeometry"></param>
        /// <returns></returns>
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
