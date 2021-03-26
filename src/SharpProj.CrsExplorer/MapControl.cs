using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
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
                foreach (Polygon p in Polygons(g))
                {
                    using (Brush b = new SolidBrush(Colors[n % Colors.Length]))
                    {
                        var coords = p.ExteriorRing.Coordinates
                                .Select(createCoord)
                                .ToArray();
                        e.Graphics.FillPolygon(b, coords);
                    }

                    foreach (var r in p.InteriorRings)
                    {
                        var coords = r.Coordinates
                                .Select(createCoord)
                                .ToArray();
                        e.Graphics.FillPolygon(SystemBrushes.Highlight, coords);
                    }
                }
                n++;
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

        public List<Geometry> Draw { get; private set; }

        CoordinateReferenceSystem _crs;

        public Color[] Colors { get; private set; }

        internal void UpdateDisplay(CoordinateReferenceSystem crs, List<CountryShape> want)
        {
            Invalidate();
            _crs = null;
            Draw = null;
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
                    this.Draw = want.Select(x => x.Geometry).Select(x => x.Reproject(ct, GeometryFactory.Default)).ToList();
                    _crs = crs;

                    if (Draw.Count <= 1)
                        this.Colors = new Color[] { Color.FromArgb(255, 32, 180, 32) };
                    else
                        this.Colors = Utils.Colors.DistinctColorGenerator.GetDistinctColors(Math.Min(Draw.Count, 250), BgColor);
                }
            }
            catch (ProjException)
            {
                return;
            }
        }

        static ReadOnlyCollection<CountryShape> CreateCountryShapes()
        {
            List<CountryShape> g = new List<CountryShape>();
            using(var gz = new GZipStream(typeof(CountryShape).Assembly.GetManifestResourceStream("SharpProj.CrsExplorer.mapdata.json.gz"), CompressionMode.Decompress))
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
