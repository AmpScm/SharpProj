using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace ProjSharp.Tests
{
    [TestClass]
    public class BasicTests
    {
        [TestMethod]
        public void CreateAndDestroyContext()
        {
            using(var pc = new ProjContext())
            {
                using(var crs = ProjCrs.Create(pc, "+proj=merc +ellps=clrk66 +lat_ts=33"))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Description);
                }

                using (var crs = ProjCrs.Create(pc, "proj=merc", "ellps=clrk66", "lat_ts=33"))
                {
                    Assert.AreEqual("PROJ-based coordinate operation", crs.Description);
                }
            }
        }


        [TestMethod]
        public void CreateAndDestroyContextEPSG()
        {
            using (var pc = new ProjContext())
            {
                GC.KeepAlive(pc.Clone());

                // Needs proj.db

                using (var crs = ProjCrs.Create(pc, "EPSG:25832"))
                {
                    Assert.AreEqual("ETRS89 / UTM zone 32N", crs.Description);
                }
            }
        }

    }
}
