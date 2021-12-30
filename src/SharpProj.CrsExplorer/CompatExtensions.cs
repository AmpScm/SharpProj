using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharpProj.CrsExplorer
{
    internal static class CompatExtensions
    {
#if NETFRAMEWORK
        public static bool Contains(this string on, string what, StringComparison c)
        {
            switch (c)
            {
                case StringComparison.InvariantCultureIgnoreCase:
                case StringComparison.OrdinalIgnoreCase:
                    on = on.ToUpperInvariant();
                    what = what.ToUpperInvariant();
                    break;
            }

            return on.Contains(what);
        }
#endif
    }
}
