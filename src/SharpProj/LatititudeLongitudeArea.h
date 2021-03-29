#pragma once
namespace SharpProj {
	value class PPoint;


	namespace Proj {

		public interface class ILatitudeLongitudeArea
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

		[DebuggerDisplay("W-E: {WestLongitude} - {EastLongitude}, N-S: {NorthLatitude} - {SouthLatitude}")]
		public ref class LatitudeLongitudeArea : ILatitudeLongitudeArea
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
			LatitudeLongitudeArea()
			{

			}
		public:
			LatitudeLongitudeArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
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
		};
	}
}