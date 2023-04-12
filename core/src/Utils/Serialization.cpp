#include <HMP/Utils/Serialization.hpp>

namespace HMP::Utils::Serialization
{

	namespace Internal
	{

		// SerializerWorker

		void SerializerWorker::operator<<(const Vec& _data)
		{
			cpputils::serialization::SerializerWorker::operator<<(_data.x());
			cpputils::serialization::SerializerWorker::operator<<(_data.y());
			cpputils::serialization::SerializerWorker::operator<<(_data.z());
		}

		// DeserializerWorker

		void DeserializerWorker::operator>>(Vec& _data)
		{
			cpputils::serialization::DeserializerWorker::operator>>(_data.x());
			cpputils::serialization::DeserializerWorker::operator>>(_data.y());
			cpputils::serialization::DeserializerWorker::operator>>(_data.z());
		}

	}

}