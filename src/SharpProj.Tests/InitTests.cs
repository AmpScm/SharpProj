using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace SharpProj.Tests
{
    [TestClass]
    public class InitTests
    {
        [TestMethod]
        public void TestITRFS()
        {
            // This uses the 'ITRF2000' text file that is commonly installed as '$/share/proj/ITRF2000',
            // but is for compatibility also in SharpProj's embeded resources.
            //
            // If it is used from the embedded resources, it will be installed as '$userdir/proj-<projversion>-ITRF2000'
            // and from then on used from there

            using(var pc = new ProjContext())
                using(var p = pc.Create("+init=ITRF2000:ITRF2005 +t_obs=2010.5"))
            {

                Assert.IsInstanceOfType(p, typeof(CoordinateTransform));
            }
        }
    }
}
