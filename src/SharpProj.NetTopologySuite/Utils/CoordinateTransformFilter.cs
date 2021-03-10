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
        /// <summary>
        /// The transform behind this reproject
        /// </summary>
        protected CoordinateTransform Transform { get; }

        /// <summary>
        /// The precision model behind this reproject
        /// </summary>
        protected PrecisionModel PrecisionModel { get; }

        /// <summary>
        /// Creates an instance of this class
        /// </summary>
        /// <param name="transform">The coordinate transformation object</param>
        /// <param name="precisionModel">A precision model for the coordinates</param>
        public ReProjectFilter(CoordinateTransform transform, PrecisionModel precisionModel)
        {
            Transform = transform ?? throw new ArgumentNullException(nameof(transform));
            PrecisionModel = precisionModel ?? throw new ArgumentNullException(nameof(precisionModel));
        }

        /// <summary>
        /// Filter method to perform the coordinate transformation
        /// </summary>
        /// <param name="seq">The sequenc</param>
        public virtual void Filter(CoordinateSequence seq)
        {
            if (seq is PackedDoubleCoordinateSequence doubleSequence)
                FilterPacked(doubleSequence);
            else
                for (int i = 0; i < seq.Count; i++)
                {
                    var c = Transform.Apply(seq.GetCoordinate(i));
                    PrecisionModel.MakePrecise(c);
                    for (int j = 0; j < seq.Dimension; j++)
                        seq.SetOrdinate(i, j, c[j]);
                }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="seq"></param>
        protected void FilterPacked(PackedDoubleCoordinateSequence seq)
        {
            double[] coords = seq.GetRawCoordinates();
            int dimension = seq.Dimension;
            
            var xs = new Span<double>(coords, 0, coords.Length - 0);
            var ys = new Span<double>(coords, 1, coords.Length - 1);
            var zs = seq.HasZ
                ? new Span<double>(coords, 2, coords.Length - 2)
                : Span<double>.Empty;
            Transform.Apply(xs, seq.Dimension, ys, seq.Dimension, zs, seq.Dimension, null, 0);

            // We only make X and Y precise, just like PrecisionModel.MakePrecise()
            for (int i = 0; i < coords.Length; i += dimension)
            {
                coords[i] = PrecisionModel.MakePrecise(coords[i]);
                coords[i + 1] = PrecisionModel.MakePrecise(coords[i + 1]);
            }
        }

        /// <inheritdoc cref="IEntireCoordinateSequenceFilter.Done"/>
        public bool Done { get => false; }

        /// <inheritdoc cref="IEntireCoordinateSequenceFilter.GeometryChanged"/>
        public bool GeometryChanged { get => true; }
    }

}
