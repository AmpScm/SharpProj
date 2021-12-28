using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ProjId = SharpProj.Proj.Identifier;

namespace SharpProj.Tests
{
    [TestClass]
    public class TransformDetailsTests
    {
        public TestContext TestContext { get; set; }

        [TestMethod]
        [DataRow(true), DataRow(false)]
        public void Details_EPSG2964_to_EPSG_4326(bool useNetwork)
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.EnableNetworkConnections = useNetwork;

                using (CoordinateReferenceSystem crsFrom = CoordinateReferenceSystem.CreateFromEpsg(2964, pc))
                using (CoordinateReferenceSystem crsTo = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
                {
                    Assert.AreEqual("NAD27 / Alaska Albers", crsFrom.Name);
                    Assert.AreEqual(new ProjId("EPSG", 2964), crsFrom.Identifier);
                    Assert.AreEqual("WGS 84", crsTo.Name);
                    Assert.AreEqual(new ProjId("EPSG", 4326), crsTo.Identifier);

                    using (var ct = CoordinateTransform.Create(crsFrom, crsTo))
                    {
                        Assert.AreEqual("<choose-coordinate-transform>", ct.Name);

                        Assert.AreEqual("NAD27 / Alaska Albers", ct.SourceCRS.Name);
                        Assert.AreEqual("WGS 84", ct.TargetCRS.Name);
                        Assert.AreEqual(new ProjId("EPSG", 2964), ct.SourceCRS.Identifier);
                        Assert.AreEqual(new ProjId("EPSG", 4326), ct.TargetCRS.Identifier);
                        Assert.IsTrue(crsFrom.IsEquivalentTo(ct.SourceCRS));
                        Assert.IsTrue(crsTo.IsEquivalentTo(ct.TargetCRS));

                        var cct = ct as ChooseCoordinateTransform;
                        Assert.IsNotNull("Have Choose coordinate transform");

                        Assert.AreEqual(useNetwork ? 10 : 8, cct.Count);

                        foreach (var transform in cct)
                        {
                            Assert.IsTrue(crsFrom.IsEquivalentTo(transform.SourceCRS));
                            Assert.IsTrue(crsTo.IsEquivalentTo(transform.TargetCRS));

                            Console.WriteLine($"{transform.Name} - {transform.Accuraracy} m - {transform.UsageArea?.Name}");

                            string ids = transform.Identifier?.ToString();
                            string scopes = transform.Scope;
                            if (ids == null && transform is CoordinateTransformList ctl)
                            {
                                ids = string.Join(", ", ctl.Where(x => x.Identifiers != null).SelectMany(x => x.Identifiers));
                                scopes = string.Join(", ", ctl.Select(x => x.Scope).Where(x => x is not null).Distinct());
                            }
                            Console.WriteLine($"Identifier(s): {ids}");
                            Console.WriteLine($"Scope(s): {scopes}");
                            Console.WriteLine(transform.AsProjString(new Proj.ProjStringOptions { MultiLine = true }));
                            //Console.WriteLine(transform.AsWellKnownText());


                            Console.WriteLine();
                        }
                    }
                }
            }
        }

        [TestMethod]
        public void CheckTransform_EPSG15020()
        {
            using (ProjContext pc = new ProjContext())
            {
                pc.EnableNetworkConnections = true;

                using (var ct = CoordinateTransform.CreateFromEpsg(15020, pc))
                {
                    Assert.IsNotNull(ct);
                    Assert.AreEqual("Alaska Albers", ct.Name);
                    Assert.AreEqual(new Proj.Identifier("EPSG", 15020), ct.Identifier);

                    Assert.IsNull(ct.SourceCRS, "No sourceCRS");
                    Assert.IsNull(ct.TargetCRS, "No targetCRS");
                }
            }
        }
    }
}
