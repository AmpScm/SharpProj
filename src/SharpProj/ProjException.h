#pragma once
namespace SharpProj {
    [Serializable]
    public ref class ProjException : System::Exception
    {
    public:
        ProjException()
        {

        }

        ProjException(String^ message)
            : Exception(message)
        {

        }

        ProjException(String^ message, System::Exception^ innerException)
            : Exception(message, innerException)
        {

        }

    protected:
        ProjException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context)
            : Exception(info, context)
        {

        }
    };


    [Serializable]
    public ref class ProjOperationException : ProjException
    {
    public:
        ProjOperationException()
        {

        }

        ProjOperationException(String^ message)
            : ProjException(StripMessage(message))
        {

        }

        ProjOperationException(String^ message, System::Exception^ innerException)
            : ProjException(StripMessage(message), innerException)
        {

        }

    protected:
        ProjOperationException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context)
            : ProjException(info, context)
        {

        }

    private:
        static String^ StripMessage(String^ s)
        {
            if (s)
            {
                s = s->Replace("Error 1027 (Invalid value for an argument): ", "")
                    ->Replace("Error 1026 (Missing argument): ", "");
            }
            return s;
        }
    };

    [Serializable]
    public ref class ProjOperationArgumentException : ProjOperationException
    {
    public:
        ProjOperationArgumentException()
        {

        }

        ProjOperationArgumentException(String^ message)
            : ProjOperationException(message)
        {

        }

        ProjOperationArgumentException(String^ message, System::Exception^ innerException)
            : ProjOperationException(message, innerException)
        {

        }

    protected:
        ProjOperationArgumentException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context)
            : ProjOperationException(info, context)
        {

        }
    };

    [Serializable]
    public ref class ProjTransformException : ProjException
    {
    public:
        ProjTransformException()
        {

        }

        ProjTransformException(String^ message)
            : ProjException(message)
        {

        }

        ProjTransformException(String^ message, System::Exception^ innerException)
            : ProjException(message, innerException)
        {

        }

    protected:
        ProjTransformException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context)
            : ProjException(info, context)
        {

        }
    };
}