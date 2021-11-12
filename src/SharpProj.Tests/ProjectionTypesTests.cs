using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SharpProj.Proj;

namespace SharpProj.Tests
{
    [TestClass]
    public class ProjectionTypesTests
    {
        public static IEnumerable<object[]> ProjectionArgs
            => ProjOperationDefinition.All.Where(x => x.Type == ProjOperationType.Projection).Select(x => new object[] { x });


        public static string ProjectionName(MethodInfo method, object[] args)
        {
            var def = args[0] as ProjOperationDefinition;
            return $"{method.Name}-{def.Name}({def.Title})";
        }

        [TestMethod]
        [DynamicData(nameof(ProjectionArgs), DynamicDataDisplayName = nameof(ProjectionName))]
        public void TestProjection(ProjOperationDefinition def)
        {
            using (ProjContext pc = new ProjContext() { })
                using(var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
            {
                //Console.WriteLine(def.Details);

                List<string> args = new List<string>() { "proj=" + def.Name };

                if (def.RequiredArguments.Any())
                {
                    if (def.RequiredArguments.Contains("o_proj"))
                        return; // Projection extending/over other projections

                    foreach (var opt in def.RequiredArguments)
                    {
                        SetupArg(args, opt, def.Name);
                    }
                }
                Console.WriteLine($"Trying {string.Join(" ", args)}");

                using (CoordinateReferenceSystem crs = CoordinateReferenceSystem.Create(args.ToArray(), pc))
                {
                    Assert.IsNotNull(crs.GeodeticCRS, "CRS has Geodetic CRS");
                    using(CoordinateTransform ct = CoordinateTransform.Create(wgs84, crs))
                    {
                        ct.Apply(new PPoint(0, 0));
                    }
                }                
            }
        }

        private object SetupArg(List<string> args, string opt, string name)
        {
            Func<string, object> set = delegate (string value) { args.Add(opt + "=" + value); return null; };

            switch (opt)
            {
                case "lat_0" when (name == "labrd"): return set("-10");
                case "lat_0" when (name == "lcca"): return set("35");
                case "lon_0" when (name == "labrd"): return set("40");
                case "lon_0" when (name == "lcc"): return set("-90");

                case "lat_1" when (name == "aea"): return set("29.5");
                case "lat_1" when (name == "bonne"): return set("10");
                case "lat_1" when (name == "chamb"): return set("10");
                case "lat_1" when (name == "ccon"): return set("52");
                case "lat_1" when (name == "eqdc"): return set("55");
                case "lat_1" when (name == "euler"): return set("67");
                case "lat_1" when (name == "imw_p"): return set("30");
                case "lat_1" when (name == "lcc"): return set("33");

                case "lat_1" when (name == "murd1"): return set("30");
                case "lat_1" when (name == "murd2"): return set("30");
                case "lat_1" when (name == "murd3"): return set("30");
                case "lat_1": return set("15");

                case "lon_1" when (name == "chamb"): return set("30.0");
                case "lon_1": return set("0");


                case "lat_2" when (name == "aea"):
                    return set("42.5");
                case "lat_2" when (name == "eqdc"):
                    return set("60");
                case "lat_2" when (name == "euler"):
                    return set("75");
                case "lat_2" when (name == "imw_p"):
                    return set("-40");
                case "lat_2" when (name == "lcc"): return set("45");
                case "lat_2" when (name == "murd1"): return set("50");
                case "lat_2" when (name == "murd2"): return set("50");
                case "lat_2" when (name == "murd3"): return set("50");
                case "lat_2":
                    return set("50");

                case "lon_2" when (name == "chamb"):
                    return set("40");
                case "lon_2":
                    return set("0.0");


                case "lsat" when (name == "lsat"):
                    return set("2");
                case "path" when (name == "lsat"):
                    return set("2");


                case "h" when (name == "geos"): return set("35785831.0");
                case "h" when (name == "nsper"): return set("3000000");
                case "h" when (name == "tpers"): return set("3000000");

                case "m" when (name == "gn_sinu"): return set("2");
                case "n" when (name == "gn_sinu"): return set("3");

                case "m" when (name == "oea"): return set("1");
                case "n" when (name == "oea"): return set("2");

                case "n" when (name == "urm5"): return set("0.9");
                case "n" when (name == "urmfps"): return set("0.5");

                case "path" when (name == "misrsom"): return set("1");

                    // Dummy to avoid failing tests
                case "plat_0" when (name == "sch"): return set("10");
                case "plon_0" when (name == "sch"): return set("20");
                case "phdg_0" when (name == "sch"): return set("30");
                case "h_0" when (name == "sch"): return set("3000000");
                // /dummy

                case "lonc" when (name == "omerc"): return set("0");
                case "alpha" when (name == "omerc"): return set("90");


                case "zone" when (name == "utm"): return set("59");

                default:
                    break;
            }
            return null;
        }
    }
}
