#pragma once
#include <Windows.h>
#include <string>
#include <tdh.h>
#include <vector>
#include <iostream>
#include <map>

namespace tdhcpp {

enum class EtwProviderSchemaType {
	Manifest,
	MOF
};

struct EtwFieldInfo {
	const std::wstring name;
	const ULONGLONG value;
	const std::wstring description;

	EtwFieldInfo(std::wstring name, ULONGLONG value, std::wstring description) :
		name(std::move(name)), value(value), description(std::move(description)) {
	}

	void dump(std::wostream& output) const;

};

std::wostream& operator<<(std::wostream& outputStream, const EtwFieldInfo& fieldInfo);

using EtwFieldMap = std::map<std::string, std::vector<EtwFieldInfo>>;
using EtwFieldMapPair = std::pair<std::string, std::vector<EtwFieldInfo>>;

void DumpEtwFieldMap(const EtwFieldMap& map, std::wostream& output);

struct EtwProviderMetadata {
	const std::wstring guid;
	const std::wstring name;
	const EtwProviderSchemaType type;
	const EtwFieldMap fieldMap;

	EtwProviderMetadata(std::wstring guid, std::wstring name, EtwProviderSchemaType type, EtwFieldMap fieldMap) :
		guid(std::move(guid)), name(std::move(name)), type(type), fieldMap(std::move(fieldMap)){}

	void dump(std::wostream& output) const;
};

std::wostream& operator<<(std::wostream& outputStream, const EtwProviderMetadata& provider);

std::vector<EtwProviderMetadata> GetEtwProviders();

}
