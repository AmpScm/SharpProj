#pragma once
namespace SharpProj {
	ref class CoordinateTransform;
	public value class ProjCoordinate : IEquatable<ProjCoordinate>
	{
	public:
		double X;
		double Y;
		double Z;
		double T;
		Byte m_axis;

	internal:
		ProjCoordinate(int axis, const double* v)
		{
			if (axis < 1 || axis > 4)
				throw gcnew ArgumentOutOfRangeException("axis");

			m_axis = (Byte)axis;
			X = v[0];
			Y = (axis > 1) ? v[1] : 0;
			Z = (axis > 2) ? v[2] : 0;
			T = (axis > 3) ? v[3] : 0;
		}

		ProjCoordinate(const PJ_COORD& coord)
		{
			ProjCoordinate r;
			r.X = coord.v[0];
			r.Y = coord.v[1];
			r.Z = coord.v[2];
			r.T = coord.v[3];

			if (r.T != 0)
				m_axis = 4;
			else if (r.Z != 0)
				m_axis = 3;
			else
				m_axis = 2;
		}
	public:
		ProjCoordinate(double x, double y)
		{
			X = x;
			Y = y;
			Z = T = 0;
			m_axis = 2;
		}

		ProjCoordinate(double x, double y, double z)
		{
			X = x;
			Y = y;
			Z = z;
			T = 0;
			m_axis = 3;
		}

		ProjCoordinate(double x, double y, double z, double t)
		{
			X = x;
			Y = y;
			Z = z;
			T = t;
		}

		ProjCoordinate(array<double>^ v)
		{
			if (!v || v->Length < 1)
				throw gcnew ArgumentException("Ordinate array needs at least one element");

			X = v[0];
			int n = v->Length;

			Y = (n > 1) ? v[1] : 0;
			Z = (n > 2) ? v[2] : 0;
			T = (n > 3) ? v[3] : 0;
			m_axis = Math::Min(v->Length, 4);
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
			default:
			{
				return gcnew array<double> {X, Y, Z, T};
			}
			}
		}

		Tuple<double, double>^ ToTupleXY()
		{
			return gcnew Tuple<double, double>(X, Y);
		}

		Tuple<double, double, double>^ ToTupleXYZ()
		{
			return gcnew Tuple<double, double, double>(X, Y, Z);
		}

		Tuple<double, double, double, double>^ ToTupleXYZT()
		{
			return gcnew Tuple<double, double, double, double>(X, Y, Z, T);
		}

		static ProjCoordinate FromArray(array<double>^ v)
		{
			return ProjCoordinate(v);
		}

	public:
		static operator array<double> ^ (ProjCoordinate v)
		{
			return v.ToArray();
		}

		static explicit operator ProjCoordinate (array<double>^ v)
		{
			return FromArray(v);
		}

		static operator ProjCoordinate (Tuple<double, double> t)
		{
			return ProjCoordinate(t.Item1, t.Item2);
		}

		static operator ProjCoordinate (Tuple<double, double, double> t)
		{
			return ProjCoordinate(t.Item1, t.Item2, t.Item3);
		}

		static operator ProjCoordinate (Tuple<double, double, double, double> t)
		{
			return ProjCoordinate(t.Item1, t.Item2, t.Item3, t.Item4);
		}

		static explicit operator Tuple<double,double>^ (ProjCoordinate p)
		{
			return p.ToTupleXY();
		}

		static explicit operator Tuple<double, double, double> ^ (ProjCoordinate p)
		{
			return p.ToTupleXYZ();
		}

		static explicit operator Tuple<double, double, double, double> ^ (ProjCoordinate p)
		{
			return p.ToTupleXYZT();
		}

		static bool operator ==(ProjCoordinate p1, ProjCoordinate p2)
		{
			return p1.X == p2.X && p1.Y == p2.Y && p1.Z == p2.Z && p1.T == p2.T;
		}

		static bool operator !=(ProjCoordinate p1, ProjCoordinate p2)
		{
			return p1.X != p2.X || p1.Y != p2.Y || p1.Z != p2.Z || p1.T != p2.T;
		}

		virtual bool Equals(Object^ other) override sealed
		{
			ProjCoordinate^ p = dynamic_cast<ProjCoordinate^>(other);

			if (p)
				return X == p->X && Y == p->Y && Z == p->Z && T == p->T;
			else
				return false;
		}

		virtual bool Equals(ProjCoordinate other) sealed
		{
			return X == other.X && Y == other.Y && Z == other.Z && T == other.T;
		}

		virtual int GetHashCode() override sealed
		{
			return X.GetHashCode() ^ Y.GetHashCode();
		}

		virtual System::String^ ToString() override
		{
			switch (Axis)
			{
			case 1:
				return String::Format("X={0}", X);
			case 2:
				return String::Format("X={0}, Y={1}", X, Y);
			case 3:
				return String::Format("X={0}, Y={1}, Z={2}", X, Y, Z);
			default:
			case 4:
				return String::Format("X={0}, Y={1}, Z={2}, T={3}", X, Y, Z, T);
			}
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
				return m_axis;
			}
		internal:
			void set(int value)
			{
				m_axis = value;
			}
		}

		ProjCoordinate Round(int decimals)
		{
			ProjCoordinate pc = ProjCoordinate(
				Math::Round(X, decimals),
				Math::Round(Y, decimals),
				Math::Round(Z, decimals),
				Math::Round(T, decimals));
			pc.Axis = Axis;
			return pc;
		}

		ProjCoordinate RoundXY(int decimals)
		{
			ProjCoordinate pc = ProjCoordinate(
				Math::Round(X, decimals),
				Math::Round(Y, decimals),
				Z,
				T);
			pc.Axis = Axis;
			return pc;
		}

		ProjCoordinate DegToRad()
		{
			ProjCoordinate pc = ProjCoordinate(
				proj_torad(X),
				proj_torad(Y),
				Z,
				T);
			pc.Axis = Axis;
			return pc;
		}
	};
}
