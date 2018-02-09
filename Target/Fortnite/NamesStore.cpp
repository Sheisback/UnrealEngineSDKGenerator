#include <windows.h>

#include "PatternFinder.hpp"
#include "NamesStore.hpp"

#include "EngineClasses.hpp"

class FNameEntry
{
public:
	__int32 Index;
	char pad_0x0004[0x4];
	FNameEntry* HashNext;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};

	std::string GetName() const
	{
		int seed = 0x9C5C5836; // 4D 8D 42 10 BA ? ? ? ? 41 87 02
		char buf[1024] = {};

		buf[0] = AnsiName[0] ^ 0x36; // seed modulo 100
		if (buf[0])
		{
			auto index = 0;
			char c;
			do
			{
				seed += ++index + 3;
				c = seed ^ AnsiName[index];
				buf[index] = c;
			} while (c);
		}

		return buf;
	}
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead
{
public:
	int32_t Num() const
	{
		return NumElements;
	}

	bool IsValidIndex(int32_t index) const
	{
		return index >= 0 && index < Num() && GetById(index) != nullptr;
	}

	ElementType const* const& GetById(int32_t index) const
	{
		return *GetItemPtr(index);
	}

public:
	ElementType const* const* GetItemPtr(int32_t Index) const
	{
		int32_t ChunkIndex = Index / ElementsPerChunk;
		int32_t WithinChunkIndex = Index % ElementsPerChunk;
		ElementType** Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	enum
	{
		ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
	};

	char UnknownData[0x1B8]; // 49 8B 84 C1 ?? ?? ?? ?? 48 8B 04 C8
	ElementType** Chunks[ChunkTableSize];
	char UnknownData2[0x58];
	__int32 NumElements;
	__int32 NumChunks;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16384>;

TNameEntryArray* GlobalNames = nullptr;

bool NamesStore::Initialize()
{
	const auto address = FindPattern(GetModuleHandleW(L"FortniteClient-Win64-Shipping.exe"), reinterpret_cast<const unsigned char*>("\x48\x8B\x3D\x00\x00\x00\x00\x48\x89\x9C\x24\x00\x00\x00\x00"), "xxx????xxxx????");

	if (address == -1)
	{
		return false;
	}

	const auto offset = *reinterpret_cast<uint32_t*>(address + 3);

	GlobalNames = reinterpret_cast<decltype(GlobalNames)>(*reinterpret_cast<uintptr_t*>(address + 7 + offset));

	#ifdef _DEBUG
		printf("GlobalNames Found at Address %p \n", GlobalNames);
		printf("GlobalNames num %p \n", GlobalNames->Num());
		printf("Offset %p \n", offsetof(TNameEntryArray, NumElements));
		printf("Offset %s \n", GlobalNames->GetById(0)->GetName().c_str());
	#endif

	return true;
}

void* NamesStore::GetAddress()
{
	return GlobalNames;
}

size_t NamesStore::GetNamesNum() const
{
	return GlobalNames->Num();
}

bool NamesStore::IsValid(size_t id) const
{
	return GlobalNames->IsValidIndex(static_cast<int32_t>(id));
}

std::string NamesStore::GetById(size_t id) const
{
	return GlobalNames->GetById(static_cast<int32_t>(id))->GetName();
}
