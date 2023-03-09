using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using NetTopologySuite;
using NetTopologySuite.Geometries;
using NetTopologySuite.Geometries.Implementation;

namespace SharpProj.NTS
{
    /// <summary>
    /// SRID to <see cref="CoordinateReferenceSystem"/> mapping item
    /// </summary>
    [DebuggerDisplay("SRID={SRID}, CRS={CRS}")]
    public sealed class SridItem
    {
        readonly Lazy<GeometryFactory> _factory;

        /// <summary>
        /// The unique SRID value used in NetTopologySuite
        /// </summary>
        public int SRID { get; }

        /// <summary>
        /// The registered CoordinateReferenceSystem
        /// </summary>
        public CoordinateReferenceSystem CRS { get; }

        readonly List<object> _idMap = new List<object>();

        /// <summary>
        /// Read only, not iterable dictionary to allow extending the registry
        /// </summary>
        public sealed class ItemDict
        {
            readonly Dictionary<object, object> _items = new Dictionary<object, object>();

            /// <summary>
            /// Gets or sets an item in the item dictionary for the <see cref="SridItem"/>
            /// </summary>
            /// <param name="key"></param>
            /// <returns></returns>
            public object this[object key]
            {
                get
                {
                    lock (_items)
                    {
                        if (_items.TryGetValue(key, out var value))
                            return value;
                        else
                            return null;
                    }
                }
                set
                {
                    lock (_items)
                    {
                        if (value == null)
                            _items.Remove(key);

                        _items[key] = value;
                    }
                }
            }
        }

        ItemDict _items;

        /// <summary>
        /// Gets the item dictionary
        /// </summary>
        public ItemDict Items
        {
            get { return _items ?? (_items = new ItemDict()); }
        }

        internal SridItem(CoordinateReferenceSystem crs, int srid, SridItemArgs args)
        {
            if (crs is null)
                throw new ArgumentNullException(nameof(crs));

            SRID = srid;
            CRS = crs;
            PrecisionModel pm = args.PrecisionModel;

            _factory = new Lazy<GeometryFactory>(() => NtsGeometryServices.CreateGeometryFactory(
                pm ?? NtsGeometryServices.DefaultPrecisionModel,
                SRID));
        }

        static Lazy<NtsGeometryServices> _ntsGeometryServices = new Lazy<NtsGeometryServices>(SetupServices);

        /// <summary>
        /// The NTS Geometry servic to create new Geometry factories
        /// </summary>
        public static NtsGeometryServices NtsGeometryServices
        {
            get { return _ntsGeometryServices.Value; }
            set
            {
                if (value != null && value != NtsGeometryServices)
                    _ntsGeometryServices = new Lazy<NtsGeometryServices>(() => value);
            }
        }

        private static NtsGeometryServices SetupServices()
        {
            NtsGeometryServices defaultInstance = NetTopologySuite.NtsGeometryServices.Instance;

            CoordinateSequenceFactory csf = PackedCoordinateSequenceFactory.DoubleFactory;
            PrecisionModel pm = defaultInstance.DefaultPrecisionModel;
            int srid = defaultInstance.DefaultSRID;
            GeometryOverlay go = GeometryOverlay.NG;
            CoordinateEqualityComparer c = defaultInstance.CoordinateEqualityComparer;

            return new NtsGeometryServices(csf, pm, srid, go, c);
        }

        /// <summary>
        /// Gets the <see cref="CRS"/> from the <see cref="SridItem"/>
        /// </summary>
        /// <param name="s"></param>
#pragma warning disable CA2225 // Operator overloads have named alternates
        public static implicit operator CoordinateReferenceSystem(SridItem s)
#pragma warning restore CA2225 // Operator overloads have named alternates
        {
            return s?.CRS;
        }

        /// <summary>
        /// Gets the registered value of the specified type for this <see cref="SridItem"/>. Users are generally expected to pass their own enum types.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T? Id<T>() where T : struct, Enum
        {
            int n = SridRegister.GetIdType<T>();

            object v = (n < _idMap.Count) ? _idMap[n] : null;

            if (v is T t)
                return t;
            else
                return null;
        }

        /// <summary>
        /// Tries to register the value of the specified type for this <see cref="SridItem"/>. The value must be unique in the register
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="value"></param>
        public void SetId<T>(T value)
            where T : struct, Enum
        {
            if (Id<T>().HasValue || !SridRegister.TryRegisterId(this, value))
                throw new InvalidOperationException();
        }

        // Called within writelock from SridRegister
        internal void SetId(int n, object value)
        {
            if (n <= _idMap.Count)
                _idMap.Add(null);

            _idMap[n] = value;
        }

        /// <summary>
        /// The factory used to construct new geometries for this <see cref="SridItem"/>
        /// </summary>
        public GeometryFactory Factory => _factory.Value;


        /// <summary>
        /// Setup arguments for <see cref="SridItem"/>
        /// </summary>
        public class SridItemArgs
        {
            /// <summary>
            /// 
            /// </summary>
            public PrecisionModel PrecisionModel { get; set; }
        }
    }
}
