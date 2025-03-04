using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using NetTopologySuite.Geometries;
using SharpProj.Proj;

namespace SharpProj.CrsExplorer
{
    public partial class CRSForm : Form
    {
        public CRSForm()
        {
            InitializeComponent();
        }

        public ProjContext ProjContext { get; internal set; }

        private CRSItem[] _allItems;

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            var systems = ProjContext.GetCoordinateReferenceSystems(new CoordinateReferenceSystemFilter { CelestialBodyName = "Earth" });

            _allItems = systems.Where(x => x.Type != ProjType.VerticalCrs).Select(x => new CRSItem { Info = x }).OrderBy(x => int.TryParse(x.Info.Code, out var v) ? v : 0).ToArray();

            comboBox1.Items.AddRange(_allItems);

            if (map == null)
            {
                map = new MapControl();
                map.Dock = DockStyle.Fill;
                splitContainer1.Panel2.Controls.Add(map);
            }

            if (comboBox1.SelectedItem == null)
                comboBox1.SelectedItem = _allItems.FirstOrDefault(x => x.IntCode == 3857);
        }

        private class CRSItem
        {
            private int? _code;
            public CoordinateReferenceSystemInfo Info { get; set; }
            public int? IntCode => _code ?? (int.TryParse(Info.Code, out var code) ? (_code = code).Value : null);

            public override string ToString()
            {
                if (!string.IsNullOrEmpty(Info.Authority))
                    return $"{Info.Name} - {Info.Authority}:{Info.Code}";
                else
                    return Info.Name;
            }
        }

        private void comboBox1_SelectedValueChanged(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem is CRSItem item
                && item != Current)
            {
                Current = item;
                if (CRS != null)
                    CRS.Dispose();
                CRS = Current.Info.Create();

                var ua = CRS.UsageArea;

                IdBox.Text = string.Join(", ", CRS.Identifiers);
                areaNameBox.Text = ua.Name;
                boundsLLBox.Text = $"X: {ua.WestLongitude} - {ua.EastLongitude}, Y: {ua.NorthLatitude} - {ua.SouthLatitude} (Lon-E/Lat-N)";
                if (CRS.Type != ProjType.VerticalCrs)
                {
                    if ((CRS.Axis?[0].UnitName?.Equals("metre", StringComparison.OrdinalIgnoreCase) ?? false)
                    || (CRS.Axis?[0].UnitName?.Equals("meter", StringComparison.OrdinalIgnoreCase) ?? false))
                    {
                        boundsProjectedBox.Text = $"X: {Math.Round(ua.MinX)} - {Math.Round(ua.MaxX)} m, Y: {Math.Round(ua.MinY)} - {Math.Round(ua.MaxY)} m";
                    }
                    else
                        boundsProjectedBox.Text = $"X: {ua.MinX} - {ua.MaxX}, Y: {ua.MinY} - {ua.MaxY}";

                    if (CRS.Axis != null && CRS.AxisCount >= 2)
                        boundsProjectedBox.Text += $" ({CRS.Axis[0].Abbreviation}/{CRS.Axis[1].Abbreviation})";

                    bool swapped = (CRS.WithNormalizedAxis() == CRS);
                    if (swapped)
                        boundsProjectedBox.Text += $" (axis swapped for visualization)";
                }
                else
                    boundsProjectedBox.Text = "";

                Geometry mapped;
                if (ua.WestLongitude <= ua.EastLongitude)
                {
                    mapped = new Polygon(new LinearRing(
                        new Coordinate[] {
                            new Coordinate(ua.WestLongitude, ua.NorthLatitude),
                            new Coordinate(ua.EastLongitude, ua.NorthLatitude),
                            new Coordinate(ua.EastLongitude, ua.SouthLatitude),
                            new Coordinate(ua.WestLongitude, ua.SouthLatitude),
                            new Coordinate(ua.WestLongitude, ua.NorthLatitude)
                        }));
                    mapped.Normalize();
                }
                else
                {
                    mapped = new MultiPolygon(new Polygon[] {
                        new Polygon(new LinearRing(
                            new Coordinate[] {
                                new Coordinate(ua.WestLongitude, ua.NorthLatitude),
                                new Coordinate(180, ua.NorthLatitude),
                                new Coordinate(180, ua.SouthLatitude),
                                new Coordinate(ua.WestLongitude, ua.SouthLatitude),
                                new Coordinate(ua.WestLongitude, ua.NorthLatitude)
                            })),
                        new Polygon(new LinearRing(
                            new Coordinate[] {
                                new Coordinate(-180, ua.NorthLatitude),
                                new Coordinate(ua.EastLongitude, ua.NorthLatitude),
                                new Coordinate(ua.EastLongitude, ua.SouthLatitude),
                                new Coordinate(-180, ua.SouthLatitude),
                                new Coordinate(-180, ua.NorthLatitude)
                            }))
                    });
                    mapped.Normalize();
                }

                var extra = mapped.Buffer(3);


                List<string> contained = new List<string>();
                List<string> intersects = new List<string>();
                List<MapControl.CountryShape> want = new List<MapControl.CountryShape>();
                int n = 0;
                int n2 = 0;
                foreach (var s in map.CountryShapes)
                {
                    if (mapped.Contains(s.Geometry))
                    {
                        contained.Add(s.Name);
                        want.Insert(n++, s);
                    }
                    else if (mapped.Intersects(s.Geometry))
                    {
                        intersects.Add(s.Name);
                        want.Insert(n + n2++, s);
                    }
                    else if (extra.Intersects(s.Geometry))
                    {
                        want.Add(s);
                    }
                }

                string mp = "";
                if (CRS.DistanceTransform != null)
                {
                    var projection = CRS.DistanceTransform.Options().SelectMany(x => x.ProjOperations()).FirstOrDefault(x => x.Type == ProjOperationType.Projection);

                    if (projection != null)
                        mp = ProjOperationDefinition.All[projection.Name].Title;
                    else if (!CRS.DistanceTransform.IsAvailable || !CRS.DistanceTransform.HasInverse)
                    {
                        string method = CRS.DistanceTransform.AsWellKnownText()?.Split('\n').Select(x => x.Trim()).FirstOrDefault(x => x.StartsWith("METHOD["));

                        if (method != null)
                        {
                            method = method.Substring("METHOD[".Length).Trim(',', '\"', ']');

                            mp = $">>Projection '{method}' unavailable<<";
                        }
                    }
                }

                if (contained.Count > 0)
                    mp += "\r\nContains: " + string.Join(", ", contained);
                if (intersects.Count > 0)
                    mp += "\r\nContains part of: " + string.Join(", ", intersects);

                textBox1.Text = mp.Trim();
                map.UpdateDisplay(CRS.WithNormalizedAxis(), want);
            }
        }

        private CRSItem Current { get; set; }
        private CoordinateReferenceSystem CRS { get; set; }

        private void filterBox_TextChanged(object sender, EventArgs e)
        {
            string filterText = filterBox.Text.Trim();

            CRSItem item = comboBox1.SelectedItem as CRSItem;

            CRSItem[] allItems;
            if (string.IsNullOrEmpty(filterText))
                allItems = _allItems;
            else
                allItems = _allItems.Where(x => (x.Info.Name?.Contains(filterText, StringComparison.OrdinalIgnoreCase) ?? false)
                                                || (x.Info.AreaName?.Contains(filterText, StringComparison.OrdinalIgnoreCase) ?? false)
                                                || (int.TryParse(filterText, out var i) && x.IntCode == i)).ToArray();

            if (comboBox1.Items.Count != allItems.Length || allItems.Length != _allItems.Length)
            {
                comboBox1.Items.Clear();
                comboBox1.Items.AddRange(allItems);

                comboBox1.SelectedItem = allItems.Contains(item) ? item : allItems.FirstOrDefault();
            }
        }
    }
}
