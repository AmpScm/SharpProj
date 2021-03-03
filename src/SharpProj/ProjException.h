#pragma once
namespace SharpProj {
	[SerializableAttribute]
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


	[SerializableAttribute]
	public ref class ProjOperationException : ProjException
	{
	public:
		ProjOperationException()
		{

		}

		ProjOperationException(String^ message)
			: ProjException(message)
		{

		}

		ProjOperationException(String^ message, System::Exception^ innerException)
			: ProjException(message, innerException)
		{

		}

	protected:
		ProjOperationException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context)
			: ProjException(info, context)
		{

		}
	};

	[SerializableAttribute]
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