using System;
using System.Collections.Generic;
using System.Linq;
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
            using var pc = new ProjContext() { EnableNetworkConnections = false };


            var expectNone = new HashSet<ProjType> {
                ProjType.BoundCrs,
                ProjType.EngineeringCrs, ProjType.EngineeringDatum,
                ProjType.ParametricDatum,
                ProjType.TemporalCrs, ProjType.TemporalDatum,
                ProjType.Unknown,
                ProjType.ChooseTransform, // SharpProj type (id = 1001)
                ProjType.CoordinateSystem, // Not mapped by proj
                ProjType.CoordinateMetadata,
                ProjType.DerivedProjectedCrs
            };

            var failOnCreate = new HashSet<ProjType> {
                ProjType.ConcatenatedOperation,
                ProjType.Conversion,
                ProjType.DynamicGeodeticReferenceFrame,
                ProjType.DynamicVerticalReferenceFrame,
                ProjType.DatumEnsemble,
                ProjType.Ellipsoid,
                ProjType.GeodeticReferenceFrame,
                ProjType.Transformation,
                ProjType.VerticalReferenceFrame,
                ProjType.PrimeMeridian,
                ProjType.OtherCoordinateTransform,
            };

            var canNotCreate = new HashSet<ProjType> {
                // These are also included in `failOnCreate`
                ProjType.ConcatenatedOperation,
                ProjType.Conversion,
                ProjType.DynamicVerticalReferenceFrame,
                ProjType.Transformation,
                ProjType.OtherCoordinateTransform
            };

            var lst = pc.GetIdentifiers(pt, "EPSG");
            Assert.AreEqual(!expectNone.Contains(pt), lst.Any(), "Expect none");
            HashSet<ProjType> fails = new();

            if (lst.Any())
            {
                bool gotOne = false;
                int n = 0;
                foreach (var one in lst)
                {
                    try
                    {
                        using var p = pc.CreateFromDatabase(one);

                        Assert.IsNotNull(p);
                        gotOne = true;
                        break;

                    }
                    catch (ProjException) when (failOnCreate.Contains(pt))
                    {
                        fails.Add(pt);
                    }
                    catch (ProjException e)
                    {
                        throw new InvalidOperationException($"While creating {lst[0]}", e);
                    }

                    if (n++ > 250)
                        break;
                }


                Assert.AreEqual(!canNotCreate.Contains(pt), gotOne, "CanNotCreate set properly");
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
                ProjType.ParametricDatum,
                ProjType.TemporalCrs, ProjType.TemporalDatum,
                ProjType.Unknown,
                ProjType.ChooseTransform, // SharpProj type (id = 1001)
                ProjType.CoordinateSystem, // Not mapped by proj

                // New in 9.2
                ProjType.DerivedProjectedCrs,
                ProjType.CoordinateMetadata
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
