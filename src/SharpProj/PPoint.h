#pragma once
namespace SharpProj {
	ref class CoordinateTransform;

	/// <summary>
	/// .Net wrapper for proj coordinate, containing at most 4 ordinates conveniently called: X, Y, Z, T. (or V[0] upto V[3]).
	/// What these coordinates mean (and if they are swapped, etc.) is all defined by their usage.
	/// </summary>
	public value class PPoint : IEquatable<PPoint>, IFormattable
	{
	public:
		/// <summary>
		/// First ordinate of point
		/// </summary>
		property double X;
		/// <summary>
		/// Second ordinate of point
		/// </summary>
		property double Y;
		/// <summary>
		/// Third ordinate of point
		/// </summary>
		property double Z;
		/// <summary>
		/// Time component of point
		/// </summary>
		property double T
		{
			double get()
			{
				return m_tv.HasValue ? m_tv.Value : double::PositiveInfinity;
			}
			void set(double value)
			{
				if (value == double::PositiveInfinity)
					m_tv = Nullable<double>();
				else
					m_tv = value;
			}
		}
	private:
		Nullable<double> m_tv;
		Byte m_axis;

	internal:
		PPoint(int axis, const PJ_COORD& pc)
		{
			if (axis < 1 || axis > 4)
				throw gcnew ArgumentOutOfRangeException("axis");

			m_axis = (Byte)axis;
			X = pc.xyzt.x;
			Y = pc.xyzt.y;
			Z = pc.xyzt.z;
			T = pc.xyzt.t;
		}

		PPoint(const PJ_COORD& coord)
		{
			X = coord.xyzt.x;
			Y = coord.xyzt.y;
			Z = coord.xyzt.z;
			T = coord.xyzt.t;

			if (HasT)
				m_axis = 4;
			else if (coord.xyzt.z != 0)
				m_axis = 3;
			else
				m_axis = 2;
		}
	public:
		PPoint(double x)
		{
			X = x;
			Y = 0;
			Z = 0;
			T = double::PositiveInfinity;
			m_axis = 1;
		}

		PPoint(double x, double y)
		{
			X = x;
			Y = y;
			Z = 0;
			T = double::PositiveInfinity;
			m_axis = 2;
		}

		PPoint(double x, double y, double z)
		{
			X = x;
			Y = y;
			Z = z;
			T = double::PositiveInfinity;
			m_axis = 3;
		}

		PPoint(double x, double y, double z, double t)
		{
			X = x;
			Y = y;
			Z = z;
			T = t;
			m_axis = HasT ? 4 : 3;
		}

		PPoint(array<double>^ v)
		{
			if (!v || v->Length < 1 || v->Length > 4)
				throw gcnew ArgumentException("Ordinate array needs 1-4 elements");

			X = v[0];
			int n = v->Length;

			Y = (n > 1) ? v[1] : 0.0;
			Z = (n > 2) ? v[2] : 0.0;
			T = (n > 3) ? v[3] : double::PositiveInfinity;

			if (n >= 4 && HasT)
				m_axis = 4;
			else
				m_axis = Math::Min(v->Length, 3);
		}

		property bool HasT
		{
			bool get()
			{
				return m_tv.HasValue;
			}
		}

		property double default[int]
		{
			double get(int index)
			{
				switch (index)
				{
					case 0:
						return X;
					case 1:
						return Y;
					case 2:
						return Z;
					case 3:
						return T;
					default:
						throw gcnew IndexOutOfRangeException();
				}
			}
			void set(int index, double value)
			{
				switch (index)
				{
					case 0:
						X = value;
						break;
					case 1:
						Y = value;
						break;
					case 2:
						Z = value;
						break;
					case 3:
						T = value;
						break;
					default:
						throw gcnew IndexOutOfRangeException();
				}
			}
		}

			array<double>^ ToArray()
		{
			switch (Axis)
			{
			case 1:
			{
				return gcnew array<double> {X};
			}
			case 2:
			{
				return gcnew array<double> {X, Y};
			}
			case 3:
			{
				return gcnew array<double> {X, Y, Z};
			}
			case 4:
			{
				return gcnew array<double> {X, Y, Z, T};
			}
			default:
			{
				if (HasT)
					return gcnew array<double> {X, Y, Z, T};
				else
					return gcnew array<double> {X, Y, Z};
			}
			}
		}

		static PPoint FromArray(array<double>^ v)
		{
			return PPoint(v);
		}

	public:
		static explicit operator array<double> ^ (PPoint v)
		{
			return v.ToArray();
		}

		static explicit operator PPoint (array<double>^ v)
		{
			return FromArray(v);
		}

		static bool operator ==(PPoint p1, PPoint p2)
		{
			return p1.X == p2.X && p1.Y == p2.Y && p1.Z == p2.Z && p1.T == p2.T;
		}

		static bool operator !=(PPoint p1, PPoint p2)
		{
			return p1.X != p2.X || p1.Y != p2.Y || p1.Z != p2.Z || p1.T != p2.T;
		}

		virtual bool Equals(Object^ other) override sealed
		{
			PPoint^ p = dynamic_cast<PPoint^>(other);

			if (p)
				return X == p->X && Y == p->Y && Z == p->Z && T == p->T;
			else
				return false;
		}

		virtual bool Equals(PPoint other) sealed
		{
			return X == other.X && Y == other.Y && Z == other.Z && T == other.T;
		}

		virtual int GetHashCode() override sealed
		{
			return X.GetHashCode() ^ Y.GetHashCode() ^ Z.GetHashCode() ^ T.GetHashCode();
		}

		/// <summary>Alias for Z</summary>
		[System::ComponentModel::BrowsableAttribute(false)]
		property double M
		{
			double get() { return Z; }
			void set(double value) { Z = value; }
		}

		[System::ComponentModel::BrowsableAttribute(false)]
		property int Axis
		{
			int get()
			{
				return m_axis ? m_axis : (HasT ? 4 : 3);
			}
			void set(int value)
			{
				if (value >= 1 && value <= 4)
					m_axis = value;
				else if (m_axis == 0)
					m_axis = (HasT ? 4 : 3);
			}
		}

		PPoint DegToRad();

		PPoint RadToDeg();

		PPoint Offset(double dx, double dy)
		{
			PPoint r = PPoint(X + dx, Y + dy, Z, T);
			r.Axis = Axis;
			return r;
		}

		PPoint Offset(double dx, double dy, double dz)
		{
			PPoint r = PPoint(X + dx, Y + dy, Z + dz, T);
			r.Axis = Axis;
			return r;
		}

		property bool HasValues
		{
			bool get()
			{
				return !double::IsNaN(X) && !double::IsInfinity(X)
					&& !double::IsNaN(Y) && !double::IsInfinity(Y);
			}
		}

	public:
		/// <summary>
		/// Trims values of <see cref="PPoint" /> to just X and Y
		/// </summary>
		/// <returns></returns>
		PPoint ToXY()
		{
			return PPoint(X, Y);
		}

		/// <summary>
		/// Trims values in <see cref="PPoint" /> to just X and Y. Also rounds these values to <paramref name="decimals"/> decimals.
		/// </summary>
		/// <param name="decimals"></param>
		/// <returns></returns>
		PPoint ToXY(int decimals)
		{
			return PPoint(Math::Round(X, decimals), Math::Round(Y, decimals));
		}

		/// <summary>
		/// Trims values in <see cref="PPoint" /> to just X, Y and Z.
		/// </summary>
		/// <returns></returns>
		PPoint ToXYZ()
		{
			return PPoint(X, Y, Z);
		}

		/// <summary>
		/// Trims values in <see cref="PPoint" /> to just X, Y and Z. Also rounds these values to <paramref name="decimals"/> decimals.
		/// </summary>
		/// <param name="decimals"></param>
		/// <returns></returns>
		PPoint ToXYZ(int decimals)
		{
			return PPoint(Math::Round(X, decimals), Math::Round(Y, decimals), Math::Round(Z, decimals));
		}

		/// <summary>
		/// Creates a new <see cref="PPoint"/> with all values copied and <see cref="PPoint::T"/> set to <paramref name="t"/>
		/// </summary>
		/// <param name="t"></param>
		/// <returns></returns>
		PPoint WithT(double t)
		{
			PPoint nw = *this;
			nw.T = t;
			return nw;
		}

		/// <summary>
		/// Sets the T value to the decimalyear value of <paramref name="date"/>
		/// </summary>
		/// <param name="date"></param>
		/// <returns></returns>
		PPoint WithT(DateTime date)
		{
			int year = date.Year;
			double daysInYear = DateTime::IsLeapYear(year) ? 366 : 365;
			return WithT((double)year + (double)(date.DayOfYear - 1) / daysInYear);
		}

		virtual String^ ToString() override sealed
		{
			return ToString("G", System::Globalization::CultureInfo::CurrentCulture);
		}
		virtual String^ ToString(String^ format, IFormatProvider^ formatProvider) sealed;

		String^ ToString(String^ format)
		{
			return ToString(format, System::Globalization::CultureInfo::CurrentCulture);
		}
	};
}
