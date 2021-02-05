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

}