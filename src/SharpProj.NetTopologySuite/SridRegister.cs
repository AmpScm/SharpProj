using System;
using System.Collections.Generic;
using System.Threading;
using NetTopologySuite.Geometries;
using SharpProj;
using SparpProj.NetTopologySuite.Implementation;

namespace SparpProj.NetTopologySuite
{
    public static partial class SridRegister
    {
        static readonly Dictionary<int, SridItem> _catalog = new Dictionary<int, SridItem>();
        static readonly Dictionary<CoordinateReferenceSystem, SridItem> _registered = new Dictionary<CoordinateReferenceSystem, SridItem>();

        static ReaderWriterLockSlim _rwl = new ReaderWriterLockSlim();
        static List<System.Collections.IDictionary> _dicts = new List<System.Collections.IDictionary>();
        static readonly Dictionary<Type, Delegate> _reprojects = new Dictionary<Type, Delegate>();


        static int _nextId = -21000;

        public static SridItem GetByValue(int srid)
        {
            using (_rwl.WithReadLock())
            {
                if (_catalog.TryGetValue(srid, out var v))
                    return v;

                throw new IndexOutOfRangeException();
            }
        }

        public static bool TryGetById<T>(T key, out SridItem item)
            where T : struct, Enum
        {
            item = null;
            using (_rwl.WithReadLock())
            {
                int n = GetIdType<T>();

                if (n >= _dicts.Count)
                    return false;

                IReadOnlyDictionary<T, SridItem> dict = (IReadOnlyDictionary<T, SridItem>)_dicts[n];

                if (dict?.TryGetValue(key, out item) ?? false)
                    return true;

                return false;
            }
        }

        internal static SridItem FindEnsured(CoordinateReferenceSystem crs)
        {
            using (_rwl.WithReadLock())
            {
                if (_registered.TryGetValue(crs, out var item))
                    return item;

                foreach (SridItem it in _registered.Values)
                {
                    if (it.CRS.IsEquivalentTo(crs))
                        return it;
                }
            }
            return Register(crs.Clone());
        }

        public static SridItem GetById<T>(T key)
            where T : struct, Enum
        {
            if (TryGetById(key, out var item))
                return item;
            else
                throw new KeyNotFoundException();
        }

        /// <summary>
        /// Tries to register the CoordinateReferenceSystem with the specific SRID. Fails if already registered one at this id
        /// </summary>
        /// <param name="crs"></param>
        /// <param name="withSrid"></param>
        /// <returns></returns>
        public static SridItem Register(CoordinateReferenceSystem crs, int withSrid)
        {
            if (crs == null)
                throw new ArgumentNullException(nameof(crs));

            using (_rwl.WithWriteLock())
            {
                return WithinWriteLock_Register(crs, withSrid);
            }
        }

        private static SridItem WithinWriteLock_Register(CoordinateReferenceSystem crs, int withSrid)
        {
            SridItem added = new SridItem(withSrid, crs);

            _registered.Add(crs, added);
            _catalog.Add(withSrid, added);

            return added;
        }

        /// <summary>
        /// Registers the CoordinateReferenceSystem with a new SRID.
        /// </summary>
        /// <param name="crs"></param>
        /// <param name="withSrid"></param>
        /// <returns></returns>
        public static SridItem Register(CoordinateReferenceSystem crs)
        {
            if (crs == null)
                throw new ArgumentNullException(nameof(crs));

            using (_rwl.WithWriteLock())
            {
                if (_registered.ContainsKey(crs))
                    throw new ArgumentException("CRS instance already registered", nameof(crs));

                return WithinWriteLock_Register(crs);
            }
        }

        static SridItem WithinWriteLock_Register(CoordinateReferenceSystem crs)
        {
            while (_catalog.ContainsKey(_nextId))
                _nextId--;

            SridItem added = new SridItem(_nextId, crs);

            _registered.Add(crs, added);
            _catalog.Add(_nextId, added);

            return added;
        }

        public static bool TryRegisterId<T>(SridItem item, T value)
            where T : Enum
        {
            if (item == null)
                throw new ArgumentNullException(nameof(item));
            if (!_catalog.TryGetValue(item.SRID, out var vv) || !ReferenceEquals(vv, item))
                throw new ArgumentOutOfRangeException(nameof(item));

            using (_rwl.WithWriteLock())
            {
                return WithingWriteLock_TryRegisterId(item, value);
            }
        }

        private static bool WithingWriteLock_TryRegisterId<T>(SridItem item, T value) where T : Enum
        {
            if (_catalog[item.SRID] != item)
                throw new InvalidOperationException();

            int n = GetIdType<T>();

            while (_dicts.Count <= n)
                _dicts.Add(null);

            if (_dicts[n] == null)
                _dicts[n] = new Dictionary<T, SridItem>();

            if (_dicts[n].Contains(value))
                return false;

            _dicts[n].Add(value, item);
            item.SetId(n, value);
            return true;
        }

        public static void RegisterId<T>(SridItem item, T value)
            where T : struct, Enum
        {
            if (!TryRegisterId(item, value))
            {
                throw new InvalidOperationException();
            }
        }

        static int _nextTypeId = 0;
        sealed class TypeId<T>
            where T : Enum
        {
            public static int Value { get; } = _nextTypeId++;
        }

        internal static int GetIdType<T>()
            where T : Enum
        {
            return TypeId<T>.Value;
        }


        public static SridItem Ensure<T>(T value, Func<CoordinateReferenceSystem> creator)
            where T : struct, Enum
        {
            return Ensure(value, creator, null);
        }

        public static SridItem Ensure<T>(T value, Func<CoordinateReferenceSystem> creator, int? preferredSrid)
        where T : struct, Enum
        {
            if (preferredSrid.HasValue && preferredSrid == 0)
                throw new ArgumentOutOfRangeException(nameof(preferredSrid));

            if (TryGetById(value, out var item))
                return item;

            CoordinateReferenceSystem crs = creator();

            using (_rwl.WithWriteLock())
            {
                if (_registered.ContainsKey(crs))
                    throw new ArgumentException("CRS instance already registered", nameof(crs));

                try
                {
                    item = null;
                    if (preferredSrid.HasValue && _catalog.TryGetValue(preferredSrid.Value, out item))
                    {
                        if (item.CRS.IsEquivalentTo(crs))
                            return item;
                        preferredSrid = null;
                    }

                    if (preferredSrid.HasValue)
                        item = WithinWriteLock_Register(crs, preferredSrid.Value);
                    else
                        item = WithinWriteLock_Register(crs);

                    return item;
                }
                finally
                {
                    if (item != null)
                        WithingWriteLock_TryRegisterId(item, value);
                }
            }
        }

        static readonly Func<Geometry, GeometryFactory, Func<CoordinateSequence, CoordinateSequence>, Geometry> GenericTransform =
                            (s, f, t) => new MyGeometryTransformer(t).Transform(s, f);

        /// <summary>
        /// Allows registering custom geometries in a reprojectable way
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="reproject"></param>
        public static void AddReproject<T>(Func<T, Func<CoordinateSequence, CoordinateSequence>, GeometryFactory, T> reproject)
        {
            lock (_reprojects)
            {
                if (_reprojects.Count == 0)
                    _reprojects.Add(typeof(Geometry), GenericTransform);
                else if (_reprojects.ContainsKey(typeof(T)))
                    throw new ArgumentOutOfRangeException();

                _reprojects.Add(typeof(T), reproject);
            }
        }

        sealed class MyGeometryTransformer : Utils.GeometryTransformer
        {
            readonly Func<CoordinateSequence, CoordinateSequence> _transform;

            public MyGeometryTransformer(Func<CoordinateSequence, CoordinateSequence> transform)
            {
                _transform = transform;
            }

            protected override CoordinateSequence TransformCoordinates(CoordinateSequence coords, Geometry parent)
            {
                return _transform(coords);
            }
        }

        public static T Reproject<T>(T source, GeometryFactory toFactory, Func<CoordinateSequence, CoordinateSequence> coordinateTransform)
            where T : Geometry
        {
            Type t = source.GetType();
            Delegate dlg = null;

            lock (_reprojects)
            {
                while (true)
                {
                    if (_reprojects.TryGetValue(t, out dlg))
                        break;

                    if (t == typeof(Geometry))
                        break;

                    t = t.BaseType;
                }
            }

            // Two cases where we can handle things nicer than the dynamic invoke below
            if (typeof(T) == t)
            {
                var f = (Func<T, GeometryFactory, Func<CoordinateSequence, CoordinateSequence>, T>)dlg;

                return f(source, toFactory, coordinateTransform);
            }
            else if (typeof(Geometry) == t)
            {
                var f = (Func<Geometry, GeometryFactory, Func<CoordinateSequence, CoordinateSequence>, Geometry>)dlg ?? GenericTransform;

                return (T)f(source, toFactory, coordinateTransform);
            }

            return (T)dlg.DynamicInvoke(source, toFactory, coordinateTransform);
        }
    }
}
