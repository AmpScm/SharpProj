using System;
using NetTopologySuite.Geometries;
using NetTopologySuite.Geometries.Implementation;

namespace SharpProj.Utils.NTSAdditions
{
    /// <summary>
    /// Specialized/optimized reproject for <see cref="PackedDoubleCoordinateSequence"/>
    /// </summary>
    public class ReProjectFilterForPds : ReProjectFilter
    {
        /// <summary>
        /// Creates an instance of this class
        /// </summary>
        /// <param name="ct">The coordinate transformation object</param>
        /// <param name="pm">A precision model for the coordinates</param>
        public ReProjectFilterForPds(CoordinateTransform ct, PrecisionModel pm)
            : base(ct, pm)
        {
        }

        /// <inheritdoc cref="ReProjectFilter.Filter(CoordinateSequence)"/>
        public override void Filter(CoordinateSequence seq)
        {
            FilterPacked((PackedDoubleCoordinateSequence)seq);            
        }
    }

}
