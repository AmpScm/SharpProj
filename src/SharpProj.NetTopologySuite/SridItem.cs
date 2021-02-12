using System;
using System.Collections.Generic;
using System.Diagnostics;
using NetTopologySuite;
using NetTopologySuite.Geometries;
using SharpProj;

namespace SparpProj.NetTopologySuite
{
    [DebuggerDisplay("SRID={SRID}, CRS={CRS}")]
    public sealed class SridItem
    {
        readonly Lazy<GeometryFactory> _factory;

        public int SRID { get; }

        /// <summary>
        /// The registered CoordinateReferenceSystem
        /// </summary>
        public CoordinateReferenceSystem CRS { get; }

        readonly List<object> _idMap = new List<object>();

        public sealed class ItemDict
        {
            readonly Dictionary<object, object> _items = new Dictionary<object, object>();

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
        public ItemDict Items
        {
            get { return _items ?? (_items = new ItemDict()); }
        }

        internal SridItem(int srid, CoordinateReferenceSystem crs)
        {
            if (crs is null)
                throw new ArgumentNullException(nameof(crs));
            SRID = srid;
            CRS = crs;

            _factory = new Lazy<GeometryFactory>(() => NtsGeometryServices.Instance.CreateGeometryFactory(srid));
        }

        public static implicit operator CoordinateReferenceSystem(SridItem s)
        {
            return s?.CRS;
        }

        public T? Id<T>() where T : struct, Enum
        {
            int n = SridRegister.GetIdType<T>();

            object v = (n < _idMap.Count) ? _idMap[n] : null;

            if (v is T t)
                return t;
            else
                return null;
        }

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
        public GeometryFactory Factory => _factory.Value;
    }
}
