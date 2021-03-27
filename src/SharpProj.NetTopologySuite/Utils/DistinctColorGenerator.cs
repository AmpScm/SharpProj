using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace SharpProj.Utils.Colors
{
    [System.Diagnostics.DebuggerDisplay("X={X}, Y={Y}, Z={Z}, Color={ToColor()}")]
    struct ColorXyz
    {
        public double X { get; private set; }
        public double Y { get; private set; }
        public double Z { get; private set; }

        public ColorXyz(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static explicit operator ColorXyz(Color color)
        {
            return FromColor(color);
        }

        public static explicit operator Color(ColorXyz xyz)
        {
            return xyz.ToColor();
        }

        public static ColorXyz FromColor(Color rgb)
        {
            double r = rgb.R / 255.0;
            double g = rgb.G / 255.0;
            double b = rgb.B / 255.0;


            r = 100.0 * ((r > 0.04045) ? Math.Pow((r + 0.055) / 1.055, 2.4) : r / 12.92);
            g = 100.0 * ((g > 0.04045) ? Math.Pow((g + 0.055) / 1.055, 2.4) : g / 12.92);
            b = 100.0 * ((b > 0.04045) ? Math.Pow((b + 0.055) / 1.055, 2.4) : b / 12.92);

            return new ColorXyz
            {
                X = r * 0.4124 + g * 0.3576 + b * 0.1805,
                Y = r * 0.2126 + g * 0.7152 + b * 0.0722,
                Z = r * 0.0193 + g * 0.1192 + b * 0.9505
            };
        }

        public Color ToColor()
        {
            double x = this.X / 100.0;        //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
            double y = this.Y / 100.0;        //Y from 0 to 100.000
            double z = this.Z / 100.0;        //Z from 0 to 108.883

            var r = x * 3.2406 + y * -1.5372 + z * -0.4986;
            var g = x * -0.9689 + y * 1.8758 + z * 0.0415;
            var b = x * 0.0557 + y * -0.2040 + z * 1.0570;

            return Color.FromArgb(255,
                red: (int)Math.Round(255.0 * (r > 0.0031308 ? (1.055 * Math.Pow(r, 1.0 / 2.4) - 0.055) : 12.92 * r)),
                green: (int)Math.Round(255.0 * (g > 0.0031308 ? (1.055 * Math.Pow(g, 1.0 / 2.4) - 0.055) : 12.92 * g)),
                blue: (int)Math.Round(255.0 * (b > 0.0031308 ? (1.055 * Math.Pow(b, 1.0 / 2.4) - 0.055) : 12.92 * b)));
        }
    }

    [System.Diagnostics.DebuggerDisplay("L={L}, A={A}, B={B}, Color={ToColor()}")]
    struct ColorLab
    {
        public static readonly double DefaultLightness = 2.0;
        public static readonly double DefaultChroma = 1.0;

        public double L { get; private set; }
        public double A { get; private set; }
        public double B { get; private set; }

        public ColorLab(double l, double a, double b)
        {
            L = l;
            A = a;
            B = b;
        }

        public static explicit operator ColorLab(Color color)
        {
            return FromColor(color);
        }

        public static explicit operator Color(ColorLab xyz)
        {
            return xyz.ToColor();
        }

        public static ColorLab FromColor(Color rgb)
        {
            return FromColorXyz(ColorXyz.FromColor(rgb));
        }

        public Color ToColor()
        {
            return ToColorXyz().ToColor();
        }

        static readonly double[] REF = new double[] { 95.047, 100.0, 108.883 };

        public static ColorLab FromColorXyz(ColorXyz xyz)
        {
            double x = xyz.X / ColorLab.REF[0];
            double y = xyz.Y / ColorLab.REF[1];
            double z = xyz.Z / ColorLab.REF[2];

            x = (x > 0.008856) ? Math.Pow(x, 1.0 / 3.0) : 7.787 * x + 16.0 / 116.0;
            y = (y > 0.008856) ? Math.Pow(y, 1.0 / 3.0) : 7.787 * y + 16.0 / 116.0;
            z = (z > 0.008856) ? Math.Pow(z, 1.0 / 3.0) : 7.787 * z + 16.0 / 116.0;

            return new ColorLab
            {
                L = 116.0 * y - 16.0,
                A = 500.0 * (x - y),
                B = 200 * (y - z)
            };
        }

        public ColorXyz ToColorXyz()
        {
            double y = (this.L + 16.0) / 116.0;
            double x = this.A / 500.0 + y;
            double z = y - this.B / 200.0;

            y = Math.Pow(y, 3) > 0.008856 ? Math.Pow(y, 3) : (y - 16.0 / 116.0) / 7.787;
            x = Math.Pow(x, 3) > 0.008856 ? Math.Pow(x, 3) : (x - 16.0 / 116.0) / 7.787;
            z = Math.Pow(z, 3) > 0.008856 ? Math.Pow(z, 3) : (z - 16.0 / 116.0) / 7.787;

            return new ColorXyz(
                x: ColorLab.REF[0] * x,
                y: ColorLab.REF[1] * y,
                z: ColorLab.REF[2] * z
            );
        }

        public double ColorDifference(ColorLab other)
        {
            return ColorDifference(other, DefaultLightness, DefaultChroma);
        }

        // http://www.brucelindbloom.com/index.html?Eqn_DeltaE_CMC.html

        public double ColorDifference(ColorLab other, double lightness, double chroma)
        {
            if (lightness <= 0.0)
                throw new ArgumentOutOfRangeException(nameof(lightness));
            else if (chroma <= 0.0)
                throw new ArgumentOutOfRangeException(nameof(chroma));

            ColorLab lab1 = this;
            ColorLab lab2 = other;

            double deltaL = lab1.L - lab2.L;
            double deltaA = lab1.A - lab2.A;
            double deltaB = lab1.B - lab2.B;

            double C1 = Math.Sqrt(lab1.A * lab1.A + lab1.B * lab1.B);
            double C2 = Math.Sqrt(lab2.A * lab2.A + lab2.B * lab2.B);
            var deltaC = C1 - C2;
            var deltaH = Math.Sqrt(deltaA * deltaA + deltaB * deltaB - deltaC * deltaC); // Can return NaN!

            var H1 = (180.0 * Math.Atan2(lab1.B, lab1.A) / Math.PI + 360) % 360;

            double C1_4 = C1 * C1 * C1 * C1;
            double F = Math.Sqrt(C1_4 / (C1_4 + 1900.0));
            double T = (H1 > 345.0 || H1 < 164.0) ? (0.36 + Math.Abs(0.4 * Math.Cos(Math.PI * (H1 + 35.0) / 180.0))) : (0.56 + Math.Abs(0.2 * Math.Cos(Math.PI * (H1 + 168.0) / 180.0)));
            double SL = lab1.L < 16 ? 0.511 : (0.040975 * lab1.L) / (1 + 0.01765 * lab1.L);
            double SC = (0.0638 * C1) / (1 + 0.0131 * C1) + 0.638;
            double SH = SC * (F * T + 1 - F);
            return Math.Sqrt(Math.Pow(deltaL / (lightness * SL), 2) + Math.Pow(deltaC / (chroma * SC), 2) + Math.Pow(deltaH / SH, 2));
        }

        public static double ColorDifference(Color c1, Color c2)
        {
            return ColorDifference(c1, c2, DefaultLightness, DefaultChroma);
        }

        public static double ColorDifference(Color c1, Color c2, double lightness, double chroma)
        {
            double d = FromColor(c1).ColorDifference(FromColor(c2), lightness, chroma);

            if (double.IsNaN(d))
                return 0.0; // Happens for very light red combinations
            else
                return d;
        }
    }

    /// <summary>
    /// 
    /// </summary>
    public static class DistinctColorGenerator
    {
        static Color GetColorFromHSB(double hue, double saturation, double brightness)
        {
            if (hue < 0.0 || hue >= 360.0)
                throw new ArgumentOutOfRangeException(nameof(hue), hue, "Hue must be >= 0 and <= 360");
            else if (saturation < 0.0 || saturation > 1.0)
                throw new ArgumentOutOfRangeException(nameof(saturation), hue, "Saturation must be between 0 and 1");
            else if (brightness < 0.0 || brightness > 1.0)
                throw new ArgumentOutOfRangeException(nameof(brightness), hue, "Brightness must be between 0 and 1");

            double r = 0;
            double g = 0;
            double b = 0;
            if (saturation != 0)
            {
                // the color wheel consists of 6 sectors.
                double sectorPos = hue / 60.0f;
                int sectorNumber = (int)(Math.Floor(sectorPos));
                // get the fractional part of the sector
                double fractionalSector = sectorPos - sectorNumber;
                // calculate values for the three axes of the color.
                double p = brightness * (1.0f - saturation);
                double q = brightness * (1.0f - (saturation * fractionalSector));
                double t = brightness * (1.0f - (saturation * (1 - fractionalSector)));
                // assign the fractional colors to r, g, and b based on the sector the angle is in.
                switch (sectorNumber)
                {
                    case 0:
                        r = brightness;
                        g = t;
                        b = p;
                        break;
                    case 1:
                        r = q;
                        g = brightness;
                        b = p;
                        break;
                    case 2:
                        r = p;
                        g = brightness;
                        b = t;
                        break;
                    case 3:
                        r = p;
                        g = q;
                        b = brightness;
                        break;
                    case 4:
                        r = t;
                        g = p;
                        b = brightness;
                        break;
                    case 5:
                        r = brightness;
                        g = p;
                        b = q;
                        break;
                }
            }
            else
            {
                r = g = b = brightness;
            }
            //Check if any value is out of byte range
            if (r < 0)
                r = 0;
            if (g < 0)
                g = 0;
            if (b < 0)
                b = 0;

            return Color.FromArgb(255, (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
        }

        const int iHueCount = 360 / 8; // 0-360 with step 8
        const int iSatCount = 4; // 100%-25% with step -25
        const int iValCount = 16 + 1; // 100%-20% with step -5

        /// <summary>
        /// Gets the maximum amount of different colors <see cref="GetDifferentColors"/> and <see cref="GetDistinctColors(int)"/> can generate (most likely 3060)
        /// </summary>
        public static readonly int DifferentColorCount = iHueCount * iSatCount * iValCount;

        /// <summary>
        /// Gets an enumerable of <see cref="DifferentColorCount"/> different colors
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<Color> GetDifferentColors()
        {
            for (int iHue = 0; iHue < iHueCount; iHue++)
            {
                for (int iSat = 0; iSat < iSatCount; iSat++)
                {
                    for (int iValue = 0; iValue < iValCount; iValue++)
                    {
                        yield return GetColorFromHSB(8.0f * iHue, 1.0f - 0.25f * iSat, 1.0f - iValue * 0.05f);
                    }
                }
            }
        }

        /// <summary>
        /// Gets <paramref name="count"/> colors that are as distinct as possible.
        /// </summary>
        /// <param name="count"></param>
        /// <returns></returns>
        public static Color[] GetDistinctColors(int count)
        {
            return GetDistinctColors(count, null);
        }

        /// <summary>
        /// Gets <paramref name="count"/> colors that are as distinct as possible.
        /// </summary>
        /// <param name="count"></param>
        /// <param name="bgColor"></param>
        /// <returns></returns>
        public static Color[] GetDistinctColors(int count, Color? bgColor)
        {
            if (count < 1 || count > DifferentColorCount)
                throw new ArgumentOutOfRangeException();

            var colors = GetDifferentColors().ToArray();
            List<Color> results = new List<Color>(DifferentColorCount);
            double treshold = 16;
            bool notDown = false;

            if (count > 200)
                treshold /= 2;

            if (count == 1)
                return colors.Take(1).ToArray();

            do
            {
                foreach (var c in colors)
                {
                    if (bgColor.HasValue && ColorLab.ColorDifference(bgColor.Value, c, ColorLab.DefaultLightness, ColorLab.DefaultChroma*1.1) < treshold)
                        continue;
                    if (!results.Any(x => ColorLab.ColorDifference(x, c, ColorLab.DefaultLightness, ColorLab.DefaultChroma * 1.1) < treshold))
                        results.Add(c);
                }

                if (results.Count < count)
                {
                    treshold /= 1.2;
                    results.Clear();
                    notDown = true;
                }
                else if (results.Count > count * 2 && !notDown)
                {
                    treshold *= 1.2;
                    results.Clear();
                }
            }
            while (results.Count < count);

            int factor = Math.Min(5, results.Count);
            return Enumerable.Range(0, count).Select(x => results[GetInterleaved(x, factor, results.Count)]).ToArray();
        }

        internal static int GetInterleaved(int index, int factor, int count)
        {
            if (count < 0)
                throw new ArgumentOutOfRangeException(nameof(count));
            else if (index >= count || index < 0)
                throw new ArgumentOutOfRangeException(nameof(index));
            else if (factor < 0 || (factor > 1 && factor > count)) // Factor 1 is always ok
                throw new ArgumentOutOfRangeException(nameof(factor));

            int stride = (count + factor - 1) / factor;

            for (int i = 0; i < stride; ++i)
            {
                int max = (count - i + (stride - 1)) / stride; // Round up

                if (index >= max)
                    index -= max;
                else
                    return i + (index * stride);
            }

            throw new InvalidOperationException();
        }
    }
}
