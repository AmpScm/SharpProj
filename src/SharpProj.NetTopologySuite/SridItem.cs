using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

        readonly System.Collections.ArrayList _idMap = new System.Collections.ArrayList();
        Dictionary<object, object> _items;

        public IReadOnlyDictionary<object, object> Items
        {
            get { return _items ?? (_items = new Dictionary<object, object>()); }
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

        /// <summary>
        /// Allows adding user items to the 'Items' collection for registration purposes.
        /// </summary>
        /// <remarks>What is added can't be removed, so only use this for extending the global registry</remarks>
        /// <param name="key"></param>
        /// <param name="value"></param>
        public void AddItem(object key, object value)
        {
            lock (Items) // Triggers creating
            {
                _items.Add(key, value);
            }
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
