#pragma once

#include "ProjArea.h"
namespace SharpProj {
    value class PPoint;


    namespace Proj {

        [DebuggerDisplay("{Name,nq}")]
        public ref class UsageArea : ProjArea, IProjRange
        {
        private:
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
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            double m_minX, m_minY, m_maxX, m_maxY;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            bool m_hasMinMax;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            CoordinateTransform^ m_latLonTransform;

        internal:

            UsageArea(ProjObject^ ob, double westLongitude, double southLatitude, double eastLongitude, double northLatitude, String^ name)
            {
                m_obj = ob;
                ProjArea::WestLongitude = westLongitude <= -1000 ? double::NaN : westLongitude;
                ProjArea::SouthLatitude = southLatitude <= -1000 ? double::NaN : southLatitude;
                ProjArea::EastLongitude = eastLongitude <= -1000 ? double::NaN : eastLongitude;
                ProjArea::NorthLatitude = northLatitude <= -1000 ? double::NaN : northLatitude;
                m_name = name;
            }

        private:
            CoordinateTransform^ GetLatLonConvert();
            void CalculateBounds();

        public:
            virtual String^ ToString() override
            {
                if (Name)
                    return Name;
                else
                    return Object::ToString();
            }

        public:
            property String^ Name
            {
                String^ get()
                {
                    return m_name;
                }
            }

            /// <summary>
            /// The North-West corner as calculated by Converting NorthLat, WestLon from the Geospatial CRS to the CRS itself.
            /// </summary>
            property PPoint NorthWestCorner
            {
                PPoint get();
            }

            /// <summary>
            /// The SouthEast corner as calculated by Converting SouthLat, EastLon from the Geospatial CRS to the CRS itself.
            /// </summary>
            property PPoint SouthEastCorner
            {
                PPoint get();
            }

            /// <summary>
            /// The SouthEast corner as calculated by Converting SouthLat, WestLon from the Geospatial CRS to the CRS itself.
            /// </summary>
            property PPoint SouthWestCorner
            {
                PPoint get();
            }

            /// <summary>
            /// The SouthEast corner as calculated by Converting NorthLat, EastLon from the Geospatial CRS to the CRS itself.
            /// </summary>
            property PPoint NorthEastCorner
            {
                PPoint get();
            }

            /// <summary>
            /// Gets the point located at the center of <see cref="NorthWestCorner" />, <see cref="SouthEastCorner" />, <see cref="SouthWestCorner" /> and <see cref="NorthEastCorner" />
            /// </summary>
            property PPoint Center
            {
                PPoint get();
            }

            property double MinX
            {
                virtual double get();
            }

            property double MinY
            {
                virtual double get();
            }

            property double MaxX
            {
                virtual double get();
            }

            property double MaxY
            {
                virtual double get();
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