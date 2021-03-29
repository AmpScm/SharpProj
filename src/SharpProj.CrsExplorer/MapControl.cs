using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using System.Windows.Forms;
using NetTopologySuite.Geometries;

namespace SharpProj.CrsExplorer
{
    public partial class MapControl : UserControl
    {
        public MapControl()
        {
            InitializeComponent();
            DoubleBuffered = true;
        }

        protected override void OnCreateControl()
        {
            base.OnCreateControl();
            GC.KeepAlive(CountryShapes);
        }

        public Color BgColor { get; set; } = Color.FromArgb(0xFF, 0x59, 0x7F, 0xFF);

        protected override void OnPaint(PaintEventArgs e)
        {
            Rectangle cr = ClientRectangle;
            using (var br = new SolidBrush(BgColor))
            {
                e.Graphics.FillRectangle(br, cr);
            }

            if (_crs == null || Draw == null)
                return;

            Size sz = cr.Size;

            double minX = _crs.UsageArea.MinX;
            double minY = _crs.UsageArea.MinY;

            int minBorderWidth = sz.Width / 20;
            int minBorderHeight = sz.Height / 20;

            double boundDx = (_crs.UsageArea.MaxX - _crs.UsageArea.MinX);
            double boundDy = (_crs.UsageArea.MaxY - _crs.UsageArea.MinY);

            double dx = ((double)sz.Width - 2 * minBorderWidth) / boundDx;
            double dy = ((double)sz.Height - 2 * minBorderHeight) / boundDy;

            int n = 0;

            if (_crs.Axis != null && _crs.AxisCount >= 2 && _crs.Axis[0].UnitCode == _crs.Axis[1].UnitCode)
            {
                dx = dy = Math.Min(dx, dy);
            }

            int borderX = (sz.Width - (int)(dx * boundDx)) / 2;
            int borderY = (sz.Height - (int)(dy * boundDy)) / 2;

            Func<Coordinate, PointF> createCoord = (c => new PointF(
                                        (float)((c.X - minX) * dx + borderX),
                                        (float)(cr.Bottom - ((c.Y - minY) * dy) - borderY)));

            foreach (Geometry g in Draw)
            {
                using (Brush b = new SolidBrush(Colors[n++ % Colors.Length]))
                {
                    foreach (Polygon p in Polygons(g))
                    {
                        var coords = p.ExteriorRing.Coordinates
                                .Select(createCoord)
                                .ToArray();
                        e.Graphics.FillPolygon(b, coords);

                        foreach (var r in p.InteriorRings)
                        {
                            coords = r.Coordinates
                                    .Select(createCoord)
                                    .ToArray();
                            e.Graphics.FillPolygon(SystemBrushes.Highlight, coords);
                        }
                    }
                }
            }

            using (var blackPen = new Pen(Color.Black, 2))
                foreach (LineString l in Lines)
                {
                    var coords = l.Coordinates.Select(createCoord).ToArray();

                    var path = new GraphicsPath();
                    path.AddLines(coords);

                    e.Graphics.DrawPath(blackPen, path);
                }

            using (var transp = new SolidBrush(Color.FromArgb(80, 0, 0, 0)))// 0xFF, 0xFF, 0xFF)))
            {
                e.Graphics.FillRectangle(transp, new Rectangle(cr.Left, cr.Top, cr.Width, borderY)); // top
                e.Graphics.FillRectangle(transp, new Rectangle(cr.Left, cr.Bottom - borderY, cr.Width, borderY)); // bottom
                e.Graphics.FillRectangle(transp, new Rectangle(cr.Left, cr.Top + borderY, borderX, cr.Height - 2 * borderY)); // left
                e.Graphics.FillRectangle(transp, new Rectangle(cr.Right - borderX, cr.Top + borderY, borderX, cr.Height - 2 * borderY)); // right
            }
        }

        private IEnumerable<Polygon> Polygons(Geometry g)
        {
            if (g is Polygon p)
                yield return p;
            else if (g is MultiPolygon mp)
            {
                foreach (Polygon pp in mp.Geometries)
                    yield return pp;
            }
        }

        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            Invalidate();
        }

        static readonly Lazy<ReadOnlyCollection<CountryShape>> _geoms = new Lazy<ReadOnlyCollection<CountryShape>>(CreateCountryShapes);
        public ReadOnlyCollection<CountryShape> CountryShapes => _geoms.Value;

        readonly List<Geometry> Draw = new();

        CoordinateReferenceSystem _crs;

        public Color[] Colors { get; private set; }

        readonly List<LineString> Lines = new();

        internal void UpdateDisplay(CoordinateReferenceSystem crs, List<CountryShape> want)
        {
            Invalidate();
            _crs = null;
            Draw.Clear();
            Lines.Clear();
            if (crs.AxisCount < 2)// || !crs.Axis.All(x => x.UnitName == "metre")) // ??
                return;
            else if (crs.UsageArea == null || double.IsNaN(crs.UsageArea.MinX))
                return;
            else if (crs.GeodeticCRS == null)
                return;

            try
            {
                using (var ct = CoordinateTransform.Create(crs.GeodeticCRS.WithAxisNormalized(), crs))
                {
                    _crs = crs;
                    Draw.AddRange(want.Select(x => x.Geometry).Select(x => x.Reproject(ct, GeometryFactory.Default)));

                    if (Draw.Count <= 1)
                        this.Colors = new Color[] { Color.FromArgb(255, 32, 180, 32) };
                    else
                        this.Colors = Utils.Colors.DistinctColorGenerator.GetDistinctColors(Math.Min(Draw.Count, 250), BgColor);

                    var ua = crs.UsageArea;

                    if (ua.EastLongitude != ua.WestLongitude) // Next blocks fails over date line :(
                    {
                        HashSet<double> created = new HashSet<double>();

                        foreach (double r in new[] { 30.0, 15.0, 5.0, 1.0, 0.5, 0.25, 0.125, 0.025 })
                        {
                            foreach ((double west, double east) in NormalizeLongitude(ua.WestLongitude, ua.EastLongitude))
                            {
                                double ll = (Math.Truncate(west / r) - 1.0) * r;

                                while (ll < west)
                                    ll += r;

                                for (; ll <= east; ll += r)
                                {
                                    if (!created.Contains(ll))
                                    {
                                        CreateLongitudeLine(ct, ua, ll);
                                        created.Add(ll);
                                    }
                                }
                            }

                            if (created.Count >= 3)
                                break;
                        }
                    }

                    if (ua.SouthLatitude != ua.NorthLatitude)
                    {
                        HashSet<double> created = new HashSet<double>();

                        foreach (double r in new[] { 30.0, 15.0, 5.0, 1.0, 0.5, 0.25, 0.125, 0.025 })
                        {
                            double ll = (Math.Truncate(ua.NorthLatitude / r) + 1.0) * r;

                            while (ll > ua.NorthLatitude)
                                ll -= r;

                            for (; ll >= ua.SouthLatitude; ll -= r)
                            {
                                if (!created.Contains(ll))
                                {
                                    CreateLatitudeLine(ct, ua, ll);
                                    created.Add(ll);
                                }
                            }

                            if (created.Count >= 3)
                                break;
                        }
                    }
                }
            }
            catch (ProjException)
            {
                Draw.Clear();
                Lines.Clear();
            }
        }

        private IEnumerable<(double, double)> NormalizeLongitude(double westLongitude, double eastLongitude)
        {
            if (westLongitude < eastLongitude)
                yield return (westLongitude, eastLongitude);
            else
            {
                yield return (westLongitude, 180);
                yield return (-180, eastLongitude);
            }
        }

        private void CreateLongitudeLine(CoordinateTransform ct, Proj.UsageArea ua, double lon)
        {
            var c = new List<Coordinate>();

            int xPoints = 50;
            var d = (ua.SouthLatitude - ua.NorthLatitude) / xPoints;

            var north = Math.Min(90, ua.NorthLatitude - d * 5);
            var south = Math.Max(-90, ua.SouthLatitude + d * 5);

            d = (south - north) / xPoints;

            for (int i = 0; i <= xPoints; i++)
            {
                c.Add(new Coordinate(lon, north + d * i));
            }
            try
            {
                Lines.Add(new LineString(c.ToArray()).Reproject(ct, GeometryFactory.Default));
            }
            catch (ProjException)
            { }
        }

        private void CreateLatitudeLine(CoordinateTransform ct, Proj.ILatitudeLongitudeArea ua, double lat)
        {
            var c = new List<Coordinate>();

            foreach ((double wl, double el) in NormalizeLongitude(ua.WestLongitude, ua.EastLongitude))
            {
                int xPoints = 50;
                var d = (el - wl) / xPoints;
                var west = Math.Max(-180, wl - d * 5);
                var east = Math.Min(180, el + d * 5);

                d = (east - west) / xPoints;

                for (int i = 0; i <= xPoints; i++)
                {
                    c.Add(new Coordinate(west + d * i, lat));
                }
                try
                {
                    Lines.Add(new LineString(c.ToArray()).Reproject(ct, GeometryFactory.Default));
                }
                catch (ProjException)
                { }
            }
        }

        static ReadOnlyCollection<CountryShape> CreateCountryShapes()
        {
            List<CountryShape> g = new List<CountryShape>();
            using (var gz = new GZipStream(typeof(CountryShape).Assembly.GetManifestResourceStream("SharpProj.CrsExplorer.mapdata.json.gz"), CompressionMode.Decompress))
            using (StreamReader sr = new StreamReader(gz))
            {
                string name = null; ;
                while (sr.ReadLine() is string line)
                {
                    string lt = line.Trim();
                    if (lt == "\"geometry\": {")
                    {
                        List<string> lines = new List<string>();
                        lines.Add(line.TrimStart());

                        string start = line.Substring(0, line.IndexOf('\"')) + "}";

                        while (sr.ReadLine() is string line2)
                        {
                            lines.Add(line2.Substring(start.Length - 1));

                            if (line2.StartsWith(start))
                                break;
                        }
                        g.Add(CountryShape.Create(name, lines.ToArray()));
                    }
                    else if (lt.StartsWith("\"name\":"))
                        name = lt.Substring(lt.LastIndexOf('\"', lt.Length - 3)).Trim('\"', ',');
                }
            }
            return g.AsReadOnly();
        }

        [DebuggerDisplay("{Name,nq}: {Geometry}")]
        public class CountryShape
        {
            public string Name { get; private set; }
            public Geometry Geometry { get; private set; }

            public static CountryShape Create(string name, string[] lines)
            {
                lines[0] = lines[0].Substring(lines[0].IndexOf(':') + 1).TrimStart();
                var g = JsonSerializer.Deserialize<GeometryData>(string.Join("\n", lines), new JsonSerializerOptions { PropertyNameCaseInsensitive = true });
                g.Name = name;
                return Create(g);
            }

            private static CountryShape Create(GeometryData g)
            {
                Geometry gg;
                switch (g.Type)
                {
                    case "Polygon":
                        gg = ReadPolygon(g.Coordinates);
                        break;
                    case "MultiPolygon":
                        gg = ReadMultiPolygon(g.Coordinates);
                        break;
                    default:
                        throw new NotImplementedException();
                }

                return new CountryShape() { Name = g.Name, Geometry = gg };
            }

            private static Polygon ReadPolygon(JsonElement jsonElement)
            {
                var rings = new List<LinearRing>();

                for (int i = 0; i < jsonElement.GetArrayLength(); i++)
                {
                    rings.Add(ReadRing(jsonElement[i]));
                }

                return new Polygon(rings.First(), rings.Skip(1).ToArray());
            }

            private static LinearRing ReadRing(JsonElement jsonElement)
            {
                var coords = new List<Coordinate>();

                for (int i = 0; i < jsonElement.GetArrayLength(); i++)
                {
                    coords.Add(ReadCoordinate(jsonElement[i]));
                }

                return new LinearRing(coords.ToArray());
            }

            private static Coordinate ReadCoordinate(JsonElement jsonElement)
            {
                List<double> ordinates = new List<double>();
                for (int i = 0; i < jsonElement.GetArrayLength(); i++)
                {
                    ordinates.Add(jsonElement[i].GetDouble());
                }

                switch (ordinates.Count)
                {
                    case 2:
                        return new Coordinate(ordinates[0], ordinates[1]);
                    default:
                        throw new NotSupportedException();

                }
            }

            private static MultiPolygon ReadMultiPolygon(JsonElement jsonElement)
            {
                List<Polygon> polys = new List<Polygon>();
                for (int i = 0; i < jsonElement.GetArrayLength(); i++)
                {
                    polys.Add(ReadPolygon(jsonElement[i]));
                }

                return new MultiPolygon(polys.ToArray());
            }

            class GeometryData
            {
                [JsonIgnore]
                public string Name { get; set; }
                public string Type { get; set; }
                public JsonElement Coordinates { get; set; }
            }
        }
    }
}
