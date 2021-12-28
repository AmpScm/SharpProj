#pragma once
#include "ProjArea.h"
namespace SharpProj {

    public ref class CoordinateArea : ProjArea
    {
    public:
        CoordinateArea()
        {}

        CoordinateArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
            : ProjArea(westLongitude, southLatitude, eastLongitude, northLatitude)
        {
        }

    public:
        virtual property double WestLongitude
        {
            virtual double get() override
            {
                return ProjArea::WestLongitude;
            }
        public:
            void set(double value) new
            {
                ProjArea::WestLongitude = value;
            }
        }
        virtual property double SouthLatitude
        {
            virtual double get() override
            {
                return ProjArea::SouthLatitude;
            }
        public:
            void set(double value) new
            {
                ProjArea::SouthLatitude = value;
            }
        }
        virtual property double EastLongitude
        {
            virtual double get() override
            {
                return ProjArea::EastLongitude;
            }
        public:
            void set(double value) new
            {
                ProjArea::EastLongitude = value;
            }
        }
        virtual property double NorthLatitude
        {
            virtual double get() override
            {
                return ProjArea::NorthLatitude;
            }
        public:
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