#pragma once
#include "ProjArea.h"
namespace SharpProj {
    using namespace SharpProj::Proj;

    public ref class CoordinateArea sealed : ProjArea
    {
    public:
        CoordinateArea()
        {}

        CoordinateArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
            : ProjArea(westLongitude, southLatitude, eastLongitude, northLatitude)
        {
        }

    public:
        property double WestLongitude
        {
        public:
            double get() new
            {
                return ProjArea::WestLongitude;
            }
            void set(double value) new
            {
                ProjArea::WestLongitude = value;
            }
        }
        property double SouthLatitude
        {
        public:
            double get() new
            {
                return ProjArea::SouthLatitude;
            }
            void set(double value) new
            {
                ProjArea::SouthLatitude = value;
            }
        }
        property double EastLongitude
        {
        public:
            double get() new
            {
                return ProjArea::EastLongitude;
            }
            void set(double value) new
            {
                ProjArea::EastLongitude = value;
            }
        }
        property double NorthLatitude
        {
        public:
            double get() new
            {
                return ProjArea::NorthLatitude;
            }
            void set(double value) new
            {
                ProjArea::NorthLatitude = value;
            }
        }

    public:
        static operator CoordinateArea ^ (IProjArea^ area)
        {
            if (!area)
                return nullptr;

            return gcnew CoordinateArea(area->WestLongitude, area->SouthLatitude, area->EastLongitude, area->NorthLatitude);
        }
    };

    public enum class IntermediateCrsUsage
    {
        Auto,
        Always,
        Never
    };

    public ref class CoordinateTransformOptions
    {
    public:
        property CoordinateArea^ Area;
        property String^ Authority;
        property Nullable<double> Accuracy;
        property bool NoBallparkConversions;
        property bool NoDiscardIfMissing;
        property bool UsePrimaryGridNames;
        property bool UseSuperseded;
        property bool StrictContains;
        property IntermediateCrsUsage IntermediateCrsUsage;
    };
}