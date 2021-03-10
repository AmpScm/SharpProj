#pragma once
namespace SharpProj {
	ref class CoordinateTransform;

	/// <summary>
	/// .Net wrapper for proj coordinate, containing at most 4 ordinates conveniently called: X, Y, Z, T. (or V[0] upto V[3]).
	/// What these coordinates mean (and if they are swapped, etc.) is all defined by their usage.
	/// </summary>
	public value class PPoint : IEquatable<PPoint>
	{
	public:
		/// <summary>
		/// First ordinate of point
		/// </summary>
		double X;
		/// <summary>
		/// Second ordinate of point
		/// </summary>
		double Y;
		/// <summary>
		/// Third ordinate of point
		/// </summary>
		double Z;
		/// <summary>
		/// Time component of point
		/// </summary>
		double T;
	private:
		Byte m_axis;


	internal:
		PPoint(int axis, const double* v)
		{
			if (axis < 1 || axis > 4)
				throw gcnew ArgumentOutOfRangeException("axis");

			m_axis = (Byte)axis;
			X = v[0];
			Y = (axis > 1) ? v[1] : 0;
			Z = (axis > 2) ? v[2] : 0;
			T = (axis > 3) ? v[3] : double::PositiveInfinity;
		}

		PPoint(const PJ_COORD& coord)
		{
			PPoint r;
			r.X = coord.v[0];
			r.Y = coord.v[1];
			r.Z = coord.v[2];
			r.T = coord.v[3];

			if (r.T != 0 && r.T != double::PositiveInfinity)
				m_axis = 4;
			else if (r.Z != 0)
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
			m_axis = (t == 0.0 || t == double::PositiveInfinity) ? 3 : 4;
		}

		PPoint(array<double>^ v)
		{
			if (!v || v->Length < 1)
				throw gcnew ArgumentException("Ordinate array needs at least one element");

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
				return (T != 0.0 && T != double::PositiveInfinity);
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

		virtual System::String^ ToString() override
		{
			String^ v;
			switch (Axis)
			{
			case 1:
				v = String::Format("X={0}", X);
				break;
			case 2:
				v = String::Format("X={0}, Y={1}", X, Y);
				break;
			case 3:
			default:
				v = String::Format("X={0}, Y={1}, Z={2}", X, Y, Z);
			}

			if (HasT)
				return String::Format("{0}, T={1}", v, T);
			else
				return v;
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
				return !double::IsNaN(X) && !double::IsInfinity(X);
			}
		}
	};
}
