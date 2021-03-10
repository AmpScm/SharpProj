using System;
using System.Collections.Generic;
using System.Threading;
using NetTopologySuite.Geometries;
using SharpProj;
using SharpProj.Implementation;

namespace SharpProj.NTS
{
    /// <summary>
    /// The global SRID (int) to <see cref="CoordinateReferenceSystem"/> mapping
    /// </summary>
    public static partial class SridRegister
    {
        static readonly Dictionary<int, SridItem> _catalog = new Dictionary<int, SridItem>();
        static readonly Dictionary<CoordinateReferenceSystem, SridItem> _registered = new Dictionary<CoordinateReferenceSystem, SridItem>();

        static ReaderWriterLockSlim _rwl = new ReaderWriterLockSlim();
        static List<System.Collections.IDictionary> _dicts = new List<System.Collections.IDictionary>();
        static readonly Dictionary<Type, Delegate> _reprojects = new Dictionary<Type, Delegate>();


        static int _nextId = -21000;

        /// <summary>
        /// Gets the SRID item by id
        /// </summary>
        /// <param name="srid"></param>
        /// <returns></returns>
        /// <exception cref="IndexOutOfRangeException"></exception>
        /// <exception cref="ArgumentException"></exception>
        public static SridItem GetByValue(int srid)
        {
            using (_rwl.WithReadLock())
            {
                if (_catalog.TryGetValue(srid, out var v))
                    return v;

                throw new IndexOutOfRangeException($"Unregistered SRID {srid} used");
            }
        }

        /// <summary>
        /// Gets the SRID item by id
        /// </summary>
        /// <param name="srid"></param>
        /// <param name="item"></param>
        /// <returns></returns>
        /// <exception cref="IndexOutOfRangeException"></exception>
        /// <exception cref="ArgumentException"></exception>
        public static bool TryGetByValue(int srid, out SridItem item)
        {
            using (_rwl.WithReadLock())
            {
                if (_catalog.TryGetValue(srid, out item))
                    return true;

                item = null;
                return false;
            }
        }

        /// <summary>
        /// - Use TryGetByValue(). Will be removed
        /// </summary>
        /// <param name="srid"></param>
        /// <param name="item"></param>
        /// <returns></returns>
        [Obsolete("Please use .TryGetByValue. This method will be removed")]
        public static bool GetByValue(int srid, out SridItem item)
        {
            return TryGetByValue(srid, out item);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="key"></param>
        /// <param name="item"></param>
        /// <returns></returns>
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

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="key"></param>
        /// <returns></returns>
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
            return Register(crs, withSrid, new SridItem.SridItemArgs());
        }

        /// <summary>
        /// Tries to register the CoordinateReferenceSystem with the specific SRID. Fails if already registered one at this id
        /// </summary>
        /// <param name="crs"></param>
        /// <param name="withSrid"></param>
        /// <param name="args"></param>
        /// <returns></returns>
        public static SridItem Register(CoordinateReferenceSystem crs, int withSrid, SridItem.SridItemArgs args)
        {
            if (crs == null)
                throw new ArgumentNullException(nameof(crs));

            using (_rwl.WithWriteLock())
            {
                return WithinWriteLock_Register(crs, withSrid, args);
            }
        }

        private static SridItem WithinWriteLock_Register(CoordinateReferenceSystem crs, int withSrid, SridItem.SridItemArgs args)
        {
            SridItem added = new SridItem(crs, withSrid, args);

            _registered.Add(crs, added);
            _catalog.Add(withSrid, added);

            return added;
        }

        /// <summary>
        /// Registers the CoordinateReferenceSystem with a new SRID.
        /// </summary>
        /// <param name="crs"></param>
        /// <param name="args"></param>
        /// <returns></returns>
        public static SridItem Register(CoordinateReferenceSystem crs, SridItem.SridItemArgs args)
        {
            if (crs == null)
                throw new ArgumentNullException(nameof(crs));
            else if (args == null)
                throw new ArgumentNullException(nameof(args));

            using (_rwl.WithWriteLock())
            {
                if (_registered.ContainsKey(crs))
                    throw new ArgumentException("CRS instance already registered", nameof(crs));

                return WithinWriteLock_Register(crs, args);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="crs"></param>
        /// <returns></returns>
        public static SridItem Register(CoordinateReferenceSystem crs)
        {
            return Register(crs, new SridItem.SridItemArgs());
        }

        static SridItem WithinWriteLock_Register(CoordinateReferenceSystem crs, SridItem.SridItemArgs args)
        {
            while (_catalog.ContainsKey(_nextId))
                _nextId--;

            SridItem added = new SridItem(crs, _nextId, args);

            _registered.Add(crs, added);
            _catalog.Add(_nextId, added);

            return added;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="item"></param>
        /// <param name="value"></param>
        /// <returns></returns>
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

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="item"></param>
        /// <param name="value"></param>
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

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="value"></param>
        /// <param name="creator"></param>
        /// <returns></returns>
        public static SridItem Ensure<T>(T value, Func<CoordinateReferenceSystem> creator)
            where T : struct, Enum
        {
            return Ensure(value, creator, null, new SridItem.SridItemArgs());
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="value"></param>
        /// <param name="creator"></param>
        /// <param name="preferredSrid"></param>
        /// <returns></returns>
        public static SridItem Ensure<T>(T value, Func<CoordinateReferenceSystem> creator, int? preferredSrid)
        where T : struct, Enum
        {
            return Ensure(value, creator, preferredSrid, new SridItem.SridItemArgs());
        }
        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="value"></param>
        /// <param name="creator"></param>
        /// <param name="preferredSrid"></param>
        /// <param name="args"></param>
        /// <returns></returns>
        public static SridItem Ensure<T>(T value, Func<CoordinateReferenceSystem> creator, int? preferredSrid, SridItem.SridItemArgs args)
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
                        item = WithinWriteLock_Register(crs, preferredSrid.Value, args);
                    else
                        item = WithinWriteLock_Register(crs, args);

                    return item;
                }
                finally
                {
                    if (item != null)
                        WithingWriteLock_TryRegisterId(item, value);
                }
            }
        }

        //static readonly Func<Geometry, GeometryFactory, Func<CoordinateSequence, CoordinateSequence>, Geometry> GenericTransform =
        //                    (s, f, t) => new MyGeometryTransformer(t).Transform(s, f);

        ///// <summary>
        ///// Allows registering custom geometries in a reprojectable way
        ///// </summary>
        ///// <typeparam name="T"></typeparam>
        ///// <param name="reproject"></param>
        //public static void AddReproject<T>(Func<T, Func<CoordinateSequence, CoordinateSequence>, GeometryFactory, T> reproject)
        //{
        //    lock (_reprojects)
        //    {
        //        if (_reprojects.Count == 0)
        //            _reprojects.Add(typeof(Geometry), GenericTransform);
        //        else if (_reprojects.ContainsKey(typeof(T)))
        //            throw new ArgumentOutOfRangeException();

        //        _reprojects.Add(typeof(T), reproject);
        //    }
        //}

        //sealed class MyGeometryTransformer : Utils.NTSAdditions.GeometryTransformer
        //{
        //    readonly Func<CoordinateSequence, CoordinateSequence> _transform;

        //    public MyGeometryTransformer(Func<CoordinateSequence, CoordinateSequence> transform)
        //    {
        //        _transform = transform;
        //    }

        //    protected override CoordinateSequence TransformCoordinates(CoordinateSequence coords, Geometry parent)
        //    {
        //        return _transform(coords);
        //    }
        //}

        ///// <summary>
        ///// 
        ///// </summary>
        ///// <typeparam name="T"></typeparam>
        ///// <param name="source"></param>
        ///// <param name="toFactory"></param>
        ///// <param name="coordinateTransform"></param>
        ///// <returns></returns>
        //public static T ReProject<T>(T source, GeometryFactory toFactory, Func<CoordinateSequence, CoordinateSequence> coordinateTransform)
        //    where T : Geometry
        //{
        //    Type t = source.GetType();
        //    Delegate dlg = null;

        //    lock (_reprojects)
        //    {
        //        while (true)
        //        {
        //            if (_reprojects.TryGetValue(t, out dlg))
        //                break;

        //            if (t == typeof(Geometry))
        //                break;

        //            t = t.BaseType;
        //        }
        //    }

        //    if (dlg == null)
        //        return (T)GenericTransform(source, toFactory, coordinateTransform);

        //    // Two cases where we can handle things nicer than the dynamic invoke below
        //    if (typeof(T) == t)
        //    {
        //        var f = (Func<T, GeometryFactory, Func<CoordinateSequence, CoordinateSequence>, T>)dlg;

        //        return f(source, toFactory, coordinateTransform);
        //    }
        //    else if (typeof(Geometry) == t)
        //    {
        //        var f = (Func<Geometry, GeometryFactory, Func<CoordinateSequence, CoordinateSequence>, Geometry>)dlg ?? GenericTransform;

        //        return (T)f(source, toFactory, coordinateTransform);
        //    }

        //    return (T)dlg.DynamicInvoke(source, toFactory, coordinateTransform);
        //}
    }
}
