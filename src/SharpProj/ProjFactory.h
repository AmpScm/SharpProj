#pragma once

#include "CoordinateSystem.h"

namespace SharpProj {
    namespace Proj {
        public ref class AxisDefinition {
        private:
            AxisDefinition() {}
        };

        public enum class Cartesian2DType
        {
            EastingNorthing = PJ_CART2D_EASTING_NORTHING,
            NorthingEasting = PJ_CART2D_NORTHING_EASTING,
            NorthPoleEastingSouthNorthingSouth = PJ_CART2D_NORTH_POLE_EASTING_SOUTH_NORTHING_SOUTH,
            SouthPoleEastingNorthNorthingSouth = PJ_CART2D_SOUTH_POLE_EASTING_NORTH_NORTHING_NORTH,
            WestingSouthing = PJ_CART2D_WESTING_SOUTHING
        };

        public enum class Ellipsoidal2DType
        {
            LongitudeLatitude = PJ_ELLPS2D_LONGITUDE_LATITUDE,
            LatitudeLongitude = PJ_ELLPS2D_LATITUDE_LONGITUDE,
        };

        public enum class Ellipsoidal3DType
        {
            LongitudeLatitudeHeight = PJ_ELLPS3D_LONGITUDE_LATITUDE_HEIGHT,
            LatitudeLongitudeHeight = PJ_ELLPS3D_LATITUDE_LONGITUDE_HEIGHT,
        };

        public ref class ProjFactory sealed
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
                initonly ProjContext^ m_context;

        internal:
            ProjFactory(ProjContext^ ctx)
            {
                if (!ctx)
                    throw gcnew ArgumentNullException("ctx");

                m_context = ctx;
            }

            static operator PJ_CONTEXT* (ProjFactory^ me)
            {
                return me->m_context;
            }

        private:
            CoordinateSystem^ CreateCoordinateSystem(CoordinateSystemType type, int axisCount, array<AxisDefinition^>^ axis);
            CoordinateSystem^ CreateCartesianCoordinateSystem(Cartesian2DType type, String^ unitName, double conversionFactor);
            CoordinateSystem^ CreateEllipsoidalCoordinateSystem(Ellipsoidal2DType type, String^ unitName, double conversionFactor);
            CoordinateSystem^ CreateEllipsoidalCoordinateSystem(Ellipsoidal3DType type, String^ horizontalUnitName, double horizontalConversionFactor, String^ verticalUnitName, double verticalConversionFactor);
        };

    }
}
