#pragma once
namespace SharpProj {
	value class PPoint;


	namespace Proj {


		[DebuggerDisplay("{Name,nq}")]
		public ref class UsageArea
		{
		private:
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly double m_westLongitude;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly double m_southLatitude;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly double m_eastLongitude;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly double m_northLatitude;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly String^ m_name;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly ProjObject^ m_obj;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			Nullable<PPoint> m_NE;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			Nullable<PPoint> m_NW;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			Nullable<PPoint> m_SE;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			Nullable<PPoint> m_SW;
			Nullable<double> m_minX, m_minY, m_maxX, m_maxY;
			CoordinateTransform^ m_latLonTransform;

		internal:

			UsageArea(ProjObject^ ob, double westLongitude, double southLatitude, double eastLongitude, double northLatitude, String^ name)
			{
				m_obj = ob;
				m_westLongitude = westLongitude;
				m_southLatitude = southLatitude;
				m_eastLongitude = eastLongitude;
				m_northLatitude = northLatitude;
				m_name = name;
			}

		private:
			CoordinateTransform^ GetLatLonConvert();

		public:
			virtual String^ ToString() override
			{
				if (Name)
					return Name;
				else
					return __super::ToString();
			}

		public:
			property double WestLongitude
			{
				double get()
				{
					return m_westLongitude;
				}
			}
			property double SouthLatitude
			{
				double get()
				{
					return m_southLatitude;
				}
			}
			property double EastLongitude
			{
				double get()
				{
					return m_eastLongitude;
				}
			}
			property double NorthLatitude
			{
				double get()
				{
					return m_northLatitude;
				}
			}
			property String^ Name
			{
				String^ get()
				{
					return m_name;
				}
			}

			property PPoint NorthWestCorner
			{
				PPoint get();
			}

			property PPoint SouthEastCorner
			{
				PPoint get();
			}

			property PPoint SouthWestCorner
			{
				PPoint get();
			}

			property PPoint NorthEastCorner
			{
				PPoint get();
			}

			property PPoint Center
			{
				PPoint get();
			}

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

			property double CenterX
			{
				double get();
			}

			property double CenterY
			{
				double get();
			}
		};
	}
}