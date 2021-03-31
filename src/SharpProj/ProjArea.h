#pragma once
namespace SharpProj {
	value class PPoint;


	namespace Proj {

		public interface class IProjArea
		{
			property double WestLongitude
			{
				double get();
			}
			property double SouthLatitude
			{
				double get();
			}
			property double EastLongitude
			{
				double get();
			}
			property double NorthLatitude
			{
				double get();
			}
		};

		public interface class IProjRange
		{
		public:
			property double MinX
			{
				double get();
			}
			property double MinY
			{
				double get();
			}
			property double MaxX
			{
				double get();
			}
			property double MaxY
			{
				double get();
			}
		};

		[DebuggerDisplay("W-E: {WestLongitude} - {EastLongitude}, N-S: {NorthLatitude} - {SouthLatitude}")]
		public ref class ProjArea : IProjArea, IFormattable, IEquatable<ProjArea^>
		{
		private:
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_west;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_south;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_east;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_north;

		protected:
			ProjArea()
			{

			}

		public:
			ProjArea(IProjArea^ area)
			{
				if (!area)
					throw gcnew ArgumentNullException("area");

				m_west = area->WestLongitude;
				m_south = area->SouthLatitude;
				m_east = area->EastLongitude;
				m_north = area->NorthLatitude;
			}
		public:
			ProjArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
			{
				m_west = westLongitude;
				m_south = southLatitude;
				m_east = eastLongitude;
				m_north = northLatitude;
			}

			property double WestLongitude
			{
				virtual double get() { return m_west; }
			protected:
				virtual void set(double value) { m_west = value; }
			}

			property double SouthLatitude
			{
				virtual double get() { return m_south; }
			protected:
				virtual void set(double value) { m_south = value; }
			}

			property double EastLongitude
			{
				virtual double get() { return m_east; }
			protected:
				virtual void set(double value) { m_east = value; }
			}
			property double NorthLatitude
			{
				virtual double get() { return m_north; }
			protected:
				virtual void set(double value) { m_north = value; }
			}

		public:
			virtual String^ ToString() override
			{
				return ToString("G", System::Globalization::CultureInfo::InvariantCulture);
			}

			// Inherited via IFormattable
			virtual String^ ToString(String^ format, IFormatProvider^ formatProvider)
			{
				String^ f = format;
				IFormatProvider^ fp = formatProvider;

				return String::Format(formatProvider, "W-E: {0} - {1}, S-N: {2} - {3}", Fmt(f, fp, WestLongitude), Fmt(f, fp, EastLongitude), Fmt(f, fp, SouthLatitude), Fmt(f, fp, NorthLatitude));
			}

		public:
			virtual bool Equals(ProjArea^ other)
			{
				if (!other)
					return false;

				return WestLongitude == other->WestLongitude && EastLongitude == other->EastLongitude && NorthLatitude == other->NorthLatitude && SouthLatitude == other->SouthLatitude;
			}

			virtual bool Equals(Object^ other) override
			{
				auto ll = dynamic_cast<ProjArea^>(other);

				if (ll)
					return Equals(ll);

				auto il = dynamic_cast<IProjArea^>(other);

				if (il)
					return Equals(gcnew ProjArea(il));

				return false;
			}

			virtual int GetHashCode() override
			{
				return WestLongitude.GetHashCode() ^ EastLongitude.GetHashCode() ^ NorthLatitude.GetHashCode() ^ SouthLatitude.GetHashCode();
			}

		private:
			String^ Fmt(String^ format, IFormatProvider^ formatProvider, double value)
			{
				if (format == "G")
					return String::Format(formatProvider, "{0}", value);
				else
					throw gcnew NotImplementedException();
			}
		};

		public ref class ProjRange : IProjRange, IFormattable, IEquatable<ProjRange^>
		{
		private:
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_minx;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_miny;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_maxx;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			double m_maxy;

		public:
			ProjRange(IProjRange^ range)
			{
				if (!range)
					throw gcnew ArgumentNullException("range");

				MinX = range->MinX;
				MinY = range->MinY;
				MaxX = range->MaxX;
				MaxY = range->MaxY;
			}

			ProjRange(double minx, double miny, double maxx, double maxy)
			{
				MinX = minx;
				MinY = miny;
				MaxX = maxx;
				MaxY = maxy;
			}

		public:
			property double MinX
			{
				virtual double get()
				{
					return m_minx;
				}
			protected:
				void set(double value)
				{
					m_minx = value;
				}
			}
			property double MinY
			{
				virtual double get()
				{
					return m_miny;
				}
			protected:
				void set(double value)
				{
					m_miny = value;
				}
			}
			property double MaxX
			{
				virtual double get()
				{
					return m_maxx;
				}
			protected:
				void set(double value)
				{
					m_maxx = value;
				}
			}
			property double MaxY
			{
				virtual double get()
				{
					return m_maxy;
				}
			protected:
				void set(double value)
				{
					m_maxy = value;
				}
			}

			virtual String^ ToString() override
			{
				return ToString("G", System::Globalization::CultureInfo::InvariantCulture);
			}

			virtual String^ ToString(System::String^ format, System::IFormatProvider^ formatProvider)
			{
				if (format == "G")
				{
					auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);
					String^ gs = ci ? ci->NumberFormat->NumberGroupSeparator : ",";

					return String::Format(formatProvider, "X: {0} - {1}{2} Y: {3} - {4}", MinX, MaxX, gs, MinY, MaxY);
				}
				else
					throw gcnew FormatException();
			}



			// Inherited via IEquatable
			virtual bool Equals(ProjRange^ other)
			{
				if (!other)
					return false;

				return MinX == other->MinX && MaxX == other->MaxX && MinY == other->MinY && MaxY == other->MaxY;
			}

			virtual bool Equals(Object^ other) override
			{
				auto pr = dynamic_cast<ProjRange^>(other);

				if (pr)
					return Equals(pr);

				auto il = dynamic_cast<IProjRange^>(other);

				if (il)
					return Equals(gcnew ProjRange(il));

				return false;
			}

			virtual int GetHashCode() override
			{
				return MinX.GetHashCode() ^ MinY.GetHashCode() ^ MaxX.GetHashCode() ^ MaxY.GetHashCode();
			}
		};
	}
}
