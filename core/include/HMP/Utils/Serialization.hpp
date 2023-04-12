#pragma once

#include <HMP/Meshing/types.hpp>
#include <cpputils/serialization/SerializerWorker.hpp>
#include <cpputils/serialization/DeserializerWorker.hpp>
#include <cpputils/serialization/Serializer.hpp>
#include <cpputils/serialization/Deserializer.hpp>

namespace HMP::Utils::Serialization
{

	namespace Internal
	{

		class SerializerWorker final : public cpputils::serialization::SerializerWorker
		{

		public:

			using cpputils::serialization::SerializerWorker::SerializerWorker;
			using cpputils::serialization::SerializerWorker::operator<<;

			void operator<<(const Vec& _data);

		};

		class DeserializerWorker final : public cpputils::serialization::DeserializerWorker
		{

		public:

			using cpputils::serialization::DeserializerWorker::DeserializerWorker;
			using cpputils::serialization::DeserializerWorker::operator>>;

			void operator>>(Vec& _data);

		};

	}

	using Serializer = cpputils::serialization::Serializer<Internal::SerializerWorker>;
	using Deserializer = cpputils::serialization::Deserializer<Internal::DeserializerWorker>;

}

