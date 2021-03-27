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
    public class CoordinateTransformFilter : IEntireCoordinateSequenceFilter
    {
        /// <summary>
        /// The transform behind this reproject
        /// </summary>
        readonly CoordinateTransform m_transform;

        /// <summary>
        /// The precision model behind this reproject
        /// </summary>
        readonly PrecisionModel m_precisionModel;

        /// <summary>
        /// Creates an instance of this class
        /// </summary>
        /// <param name="transform">The coordinate transformation object</param>
        /// <param name="precisionModel">A precision model for the coordinates</param>
        public CoordinateTransformFilter(CoordinateTransform transform, PrecisionModel precisionModel)
        {
            m_transform = transform ?? throw new ArgumentNullException(nameof(transform));
            m_precisionModel = precisionModel ?? throw new ArgumentNullException(nameof(precisionModel));
        }

        /// <summary>
        /// Filter method to perform the coordinate transformation
        /// </summary>
        /// <param name="seq">The sequenc</param>
        public virtual void Filter(CoordinateSequence seq)
        {
            switch (seq)
            {
                case PackedDoubleCoordinateSequence doubleSequence:
                    FilterPacked(doubleSequence);
                    break;
                default:
                    for (int i = 0; i < seq.Count; i++)
                    {
                        var c = m_transform.Apply(seq.GetCoordinate(i));

                        if (c != null)
                        {
                            m_precisionModel.MakePrecise(c);
                            for (int j = 0; j < seq.Dimension; j++)
                                seq.SetOrdinate(i, j, c[j]);
                        }
                        else
                            throw new ProjException($"Reprojection of {seq.GetCoordinate(i)} failed");
                    }
                    break;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="seq"></param>
        private void FilterPacked(PackedDoubleCoordinateSequence seq)
        {
            double[] coords = seq.GetRawCoordinates();
            int dimension = seq.Dimension;

            var xs = new Span<double>(coords, 0, coords.Length - 0);
            var ys = new Span<double>(coords, 1, coords.Length - 1);
            var zs = seq.HasZ
                ? new Span<double>(coords, 2, coords.Length - 2)
                : Span<double>.Empty;
            m_transform.Apply(xs, seq.Dimension, ys, seq.Dimension, zs, seq.Dimension, null, 0);

            // We only make X and Y precise, just like PrecisionModel.MakePrecise()
            for (int i = 0; i < coords.Length; i += dimension)
            {
                coords[i] = m_precisionModel.MakePrecise(coords[i]);
                coords[i + 1] = m_precisionModel.MakePrecise(coords[i + 1]);
            }
        }

        /// <inheritdoc cref="IEntireCoordinateSequenceFilter.Done"/>
        bool IEntireCoordinateSequenceFilter.Done { get => false; }

        /// <inheritdoc cref="IEntireCoordinateSequenceFilter.GeometryChanged"/>
        bool IEntireCoordinateSequenceFilter.GeometryChanged { get => true; }
    }

}
