using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SharpProj.Proj;

namespace SharpProj.Tests
{
    [TestClass]
    public class InfoTests
    {

        public static IEnumerable<object[]> BuiltinProjTypes => Enum.GetValues(typeof(ProjType)).Cast<ProjType>().Where(x => x < ProjType.ChooseTransform).Select(x => new object[] { x });

        [TestMethod]
        [DynamicData(nameof(BuiltinProjTypes))]
        public void CheckInfoEPSG(ProjType pt)
        {
            using var pc = new ProjContext();

            var expectNone = new List<ProjType> {
                ProjType.BoundCrs,
                ProjType.EngineeringCrs, ProjType.EngineeringDatum,
                ProjType.OtherCoordinateTransform,
                ProjType.ParametricDatum,
                ProjType.TemporalCrs, ProjType.TemporalDatum,
                ProjType.Unknown,
                ProjType.ChooseTransform, // SharpProj type (id = 1001)
                ProjType.CoordinateSystem, // Not mapped by proj
            };

            var noCreate = new List<ProjType> {
                ProjType.ConcatenatedOperation,
                ProjType.Conversion,
                ProjType.DynamicGeodeticReferenceFrame,
                ProjType.DynamicVerticalReferenceFrame,
                ProjType.DatumEnsemble,
                ProjType.Ellipsoid,
                ProjType.GeodeticReferenceFrame,
                ProjType.Transformation,
                ProjType.VerticalReferenceFrame,
            };

            var lst = pc.GetIdentifiers(pt, "EPSG");
            Assert.AreEqual(!expectNone.Contains(pt), lst.Any());

            if (lst.Any())
            {
                try
                {
                    using var p = pc.CreateFromDatabase(lst[0]);

                    Assert.IsNotNull(p);

                }
                catch (ProjException) when (noCreate.Contains(pt))
                { }
                catch (ProjException e)
                {
                    throw new InvalidOperationException($"While creating {lst[0]}", e);
                }
            }
        }

        [TestMethod]
        [DynamicData(nameof(BuiltinProjTypes))]
        public void CheckInfoAll(ProjType pt)
        {
            using var pc = new ProjContext();

            var expectNone = new List<ProjType> {
                ProjType.BoundCrs,
                ProjType.EngineeringCrs, ProjType.EngineeringDatum,
                ProjType.OtherCoordinateTransform,
                ProjType.ParametricDatum,
                ProjType.TemporalCrs, ProjType.TemporalDatum,
                ProjType.Unknown,
                ProjType.ChooseTransform, // SharpProj type (id = 1001)
                ProjType.CoordinateSystem, // Not mapped by proj
            };

            Assert.AreEqual(!expectNone.Contains(pt), pc.GetIdentifiers(pt).Any());
        }

        [TestMethod]
        public void CreateEllipsoid()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.Ellipsoid).First();

            using var ob = Ellipsoid.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }

        [TestMethod]
        public void CreateGeodeticReferenceFrame()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.GeodeticReferenceFrame).First();

            using var ob = Datum.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }

        [TestMethod]
        public void CreateDynamicGeodeticReferenceFrame()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.DynamicGeodeticReferenceFrame).First();

            using var ob = Datum.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }

        [TestMethod]
        public void CreateDynamicVerticalReferenceFrame()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.DynamicVerticalReferenceFrame).First();

            using var ob = Datum.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }

        [TestMethod]
        public void CreateConcatenatedOperation()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.ConcatenatedOperation).First();

            using var ob = CoordinateTransform.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }

        [TestMethod]
        public void CreateConversion()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.Conversion).First();

            using var ob = CoordinateTransform.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }

        [TestMethod]
        public void CreateTransform()
        {
            using var pc = new ProjContext();
            var item1 = pc.GetIdentifiers(ProjType.Transformation).First();

            using var ob = CoordinateTransform.CreateFromDatabase(item1);
            Assert.IsNotNull(ob);
        }
    }
}
