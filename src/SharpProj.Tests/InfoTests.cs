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

        public static IEnumerable<object[]> BuiltinProjTypes => Enum.GetValues(typeof(ProjType)).Cast<ProjType>().Where(x => x < ProjType.ChooseTransform).Select(x=>new object[] { x });

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

            Assert.AreEqual(!expectNone.Contains(pt), pc.GetIdentifiers(pt, "EPSG").Any());
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

    }
}
