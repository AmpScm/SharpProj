using System;
using NetTopologySuite.Geometries;
using NetTopologySuite.Geometries.Implementation;

namespace SharpProj.Utils.NTSAdditions
{
    /// <summary>
    /// A filter that ReProjects all coordinates in a sequence
    /// </summary>
    /// <remarks>
    /// It might be suitable to derive custom filters to take advantage of raw
    /// coordinate data.
    /// </remarks>
    public class ReProjectFilter : IEntireCoordinateSequenceFilter
    {
        private readonly CoordinateTransform _ct;
        private readonly PrecisionModel _pm;

        /// <summary>
        /// Creates an instance of this class
        /// </summary>
        /// <param name="ct">The coordinate transformation object</param>
        /// <param name="pm">A precision model for the coordinates</param>
        public ReProjectFilter(CoordinateTransform ct, PrecisionModel pm)
        {
            _ct = ct;
            _pm = pm;
        }

        /// <summary>
        /// Filter method to perform the coordinate transformation
        /// </summary>
        /// <param name="seq">The sequenc</param>
        public virtual void Filter(CoordinateSequence seq)
        {
            for (int i = 0; i < seq.Count; i++)
            {
                var c = _ct.Apply(seq.GetCoordinate(i));
                _pm.MakePrecise(c);
                for (int j = 0; j < seq.Dimension; j++)
                    seq.SetOrdinate(i, j, c[j]);
            }
        }

        /// <inheritdoc cref="IEntireCoordinateSequenceFilter.Done"/>
        public bool Done { get => false; }

        /// <inheritdoc cref="IEntireCoordinateSequenceFilter.GeometryChanged"/>
        public bool GeometryChanged { get => true; }
    }

    public class ReProjectFilterForPds : ReProjectFilter
    {
        public ReProjectFilterForPds(CoordinateTransform ct, PrecisionModel pm)
            : base(ct, pm)
        {
        }

        public override void Filter(CoordinateSequence seq)
        {
            base.Filter(seq);
            return;

            // IF proj_trans_generic was available we could bulk transform here!
            /*
            var pds = (PackedDoubleCoordinateSequence) seq;
            double[] coords = pds.GetRawCoordinates();
            int dimension = seq.Dimension;
            var xs = new Span<double>(coords, 0, coords.Length - dimension--);
            var ys = new Span<double>(coords, 1, coords.Length - dimension--);
            var zs = seq.HasZ
                ? new Span<double>(coords, 1, coords.Length - dimension--)
                : new Span<double>(Array.Empty<double>());
            _ct.Apply(xs, seq.Dimension, ys, seq.Dimension, zs, seq.Dimension, null, 0);
            */
        }
    }

}
