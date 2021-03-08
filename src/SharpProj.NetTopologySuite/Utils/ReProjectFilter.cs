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
        protected CoordinateTransform CoordinateTransform { get; }

        /// <summary>
        /// The precision model behind this reproject
        /// </summary>
        protected PrecisionModel PrecisionModel { get; }

        /// <summary>
        /// Creates an instance of this class
        /// </summary>
        /// <param name="ct">The coordinate transformation object</param>
        /// <param name="pm">A precision model for the coordinates</param>
        public ReProjectFilter(CoordinateTransform ct, PrecisionModel pm)
        {
            CoordinateTransform = ct ?? throw new ArgumentNullException(nameof(ct));
            PrecisionModel = pm ?? throw new ArgumentNullException(nameof(pm));
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
                    var c = CoordinateTransform.Apply(seq.GetCoordinate(i));
                    PrecisionModel.MakePrecise(c);
                    for (int j = 0; j < seq.Dimension; j++)
                        seq.SetOrdinate(i, j, c[j]);
                }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="doubleSequence"></param>
        protected void FilterPacked(PackedDoubleCoordinateSequence doubleSequence)
        {
            double[] coords = doubleSequence.GetRawCoordinates();
            int dimension = doubleSequence.Dimension;

            // We would really like to use System.Span<> here, but that isn't supported in .Net 4.6 yet
            // which we would like to stick to for some time. For now use something similar
            CoordinateTransform.ApplyGeneric(
                coords, 0, dimension,
                    coords, 1, dimension,
                    doubleSequence.HasZ ? coords : Array.Empty<double>(), doubleSequence.HasZ ? 2 : 0, dimension,
                    Array.Empty<double>(), 0, dimension,
                    doubleSequence.Count);

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
