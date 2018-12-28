#include <fstream>
#include <iostream>

#include <message_generated.h>

std::string generate_name()
{
	char buf[17] = {};

	size_t len = ((unsigned int)std::rand() & 15) + 1;

	buf[0] = (char)((std::rand() % 25) + 'A');
	for (size_t i = 1, end = (size_t)len; i < end; ++i)
	{
		buf[i] = (char)((std::rand() % 25) + 'a');
	}

	return buf;
}

void write()
{
	// Frustrationslevel: Niedrig.
	// Nachrichtenteile müssen von unten nach oben hergestellt werden.
	// Vergessen von Finish() schmerzt.
	// Braucht 0 Zeit beim Lesen.

	flatbuffers::FlatBufferBuilder builder(4096);

	std::vector<flatbuffers::Offset<Message::Person>> personvector;
	personvector.reserve(1000);

	for (int32_t i = 0; i < 1000; ++i)
	{
		personvector.push_back(Message::CreatePersonDirect(builder, generate_name().c_str(), i));
	}

	auto people = Message::CreateAddressbookDirect(builder, &personvector);

	builder.Finish(people);

	std::ofstream os;
	os.open("out.bin", std::ios::binary | std::ios::out);
	os.write((const char*)builder.GetBufferPointer(), builder.GetSize());
	os.close();
}

std::string read(const uint8_t *data)
{
	const Message::Addressbook *people = Message::GetAddressbook(data);

	auto flatpersonvec = people->people();

	std::string retval;
	retval.reserve(64);
	retval += flatpersonvec->begin()->name()->str() + " ";
	retval += (flatpersonvec->end() - 1)->name()->str();

	return retval;
}

void write_objapi()
{
	// Frustrationslevel: Sehr niedrig bis nicht vorhanden.
	// Struktur mit Ending "T" (per default) liefert alles, was man so braucht und man kann dann von oben nach unten herstellen. Kann dann beim Lesen auch als mutierbarer Buffer verwendet werden.
	// Ist aber langsamer. Braucht (beim Lesen) etwa halb soviel Zeit wie protobuf.

	flatbuffers::FlatBufferBuilder builder(4096);

	Message::AddressbookT people;
	people.people.reserve(1000);

	for (int32_t i = 0; i < 1000; ++i)
	{
		people.people.push_back(std::make_unique<Message::PersonT>());
		people.people[i]->id = i;
		people.people[i]->name = generate_name();
	}

	builder.Finish(Message::Addressbook::Pack(builder, &people));

	std::ofstream os;
	os.open("out.bin", std::ios::binary | std::ios::out);
	os.write((const char*)builder.GetBufferPointer(), builder.GetSize());
	os.close();
}

std::string read_objapi(const uint8_t *data)
{
	auto people = Message::UnPackAddressbook(data);

	std::string retval;
	retval.reserve(64);
	retval += people->people.begin()->get()->name + " ";
	retval += people->people.rbegin()->get()->name;
	
	return retval;

}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int main() {
	std::srand(222);
	LARGE_INTEGER freq, start, stop;

	write();

	std::ifstream in;
	in.open("out.bin", std::ios::binary | std::ios::in);

	in.seekg(0, std::ios::end);
	size_t data_len = in.tellg();
	in.seekg(0, std::ios::beg);

	uint8_t *data = new uint8_t[data_len];
	in.read((char*)data, data_len);
	in.close();

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	for (size_t i = 0; i < 1000; ++i)
	{
		if (read(data)[0] == 'a') { // just so the compiler won't optimize away
			std::cout << 'a' << std::endl;
		}
	}

	QueryPerformanceCounter(&stop);
	stop.QuadPart -= start.QuadPart;

	std::cout << "With minimal API:\n";
	std::cout << "Execution took " << ((stop.QuadPart * 1000.f) / freq.QuadPart) << "ms" << std::endl;

	/////// with --gen-object-api

	std::cout << "\n\nAnd now again using object API:\n";
	write_objapi();

	QueryPerformanceCounter(&start);

	for (size_t i = 0; i < 1000; ++i)
	{
		if (read_objapi(data)[0] == 'a') { // just so the compiler won't optimize away
			std::cout << 'a' << std::endl;
		}
	}

	QueryPerformanceCounter(&stop);
	stop.QuadPart -= start.QuadPart;

	std::cout << "Execution took " << ((stop.QuadPart * 1000.f) / freq.QuadPart) << "ms" << std::endl;

	delete[] data;

	std::cin.ignore(1024, '\n');
	return 0;
}