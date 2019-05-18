#include "tdhcpp.h"
#include "heap_ptr.h"
#include <sstream>
#include <stdexcept>
#include <map>

template <typename T>
T GetOffsetValue(PVOID ptr, ULONG offset) {
	return (T)(((PBYTE)ptr) + offset);
}


namespace tdhcpp {

heap_ptr<PROVIDER_ENUMERATION_INFO> EnumerateProviders() {
	heap_ptr<PROVIDER_ENUMERATION_INFO> providers;
	ULONG bufferSize = 0;
	TDHSTATUS status = TdhEnumerateProviders(NULL, &bufferSize);

	while (status == ERROR_INSUFFICIENT_BUFFER) {
		providers = heap_malloc<PROVIDER_ENUMERATION_INFO>(bufferSize);
		status = TdhEnumerateProviders(providers.get(), &bufferSize);
	}

	if (status != ERROR_SUCCESS) {
		throw std::exception("Could not get providers");
	}

	return providers;
}


std::vector<EtwFieldInfo> GetFields(LPGUID pProvider, EVENT_FIELD_TYPE fieldType) {

	heap_ptr<PROVIDER_FIELD_INFOARRAY> penum = NULL;
	DWORD bufferSize = 0;
	TDHSTATUS status = TdhEnumerateProviderFieldInformation(pProvider, fieldType, penum.get(), &bufferSize);

	if (status == ERROR_INSUFFICIENT_BUFFER) {
		penum = heap_malloc<PROVIDER_FIELD_INFOARRAY>(bufferSize);
		status = TdhEnumerateProviderFieldInformation(pProvider, fieldType, penum.get(), &bufferSize);
	}
	
	if (status != ERROR_SUCCESS) {
		return {};
	}
	
	std::vector<EtwFieldInfo> fields;

	for (DWORD i = 0; i < penum->NumberOfElements; i++) {
		PPROVIDER_FIELD_INFO fieldInfoPtr = &penum->FieldInfoArray[i];
			
		PWCHAR name = GetOffsetValue<PWCHAR>(penum.get(), fieldInfoPtr->NameOffset);

		PCWCHAR desc = (fieldInfoPtr->DescriptionOffset) ?
			GetOffsetValue<PWCHAR>(penum.get(), fieldInfoPtr->DescriptionOffset) : L"";

		EtwFieldInfo fieldInfo(name, fieldInfoPtr->Value, desc);
			
		fields.push_back(std::move(fieldInfo));
	}
	

	return fields;
}

void DumpEtwFieldMap(const EtwFieldMap& map, std::wostream& output) {

	for (const auto& pair : map) {
		if (!pair.second.empty()) {
			output << "\t- " << pair.first.c_str() << ":" << std::endl;

			for (const EtwFieldInfo& field : pair.second) {
				output << "\t\t" << field << std::endl;
			}
		}
	}
}

void EtwProviderMetadata::dump(std::wostream& output) const {
	output
		<< guid << L"\t"
		<< ((type == EtwProviderSchemaType::Manifest) ? L"Manifest" : L"MOF     ")
		<< L"\t" << name
		<< std::endl;

	DumpEtwFieldMap(fieldMap, output);
}

std::wostream& operator<<(std::wostream& outputStream, const EtwProviderMetadata& provider){
	provider.dump(outputStream);
	return outputStream;
}

void EtwFieldInfo::dump(std::wostream& output) const {
	output
		<< "0x"
		<< std::hex
		<< this->value
		<< std::dec
		<< ": "
		<< this->name;

	if (!description.empty()) {
		output << ": " << description;
	}
}

std::wostream& operator<<(std::wostream& outputStream, const EtwFieldInfo& fieldInfo) {
	fieldInfo.dump(outputStream);
	return outputStream;
}

static const std::map<EVENT_FIELD_TYPE, std::string> typeMap = {
	{EventKeywordInformation, "keyword"},
	{EventLevelInformation, "level"},
	{EventChannelInformation, "channel"},
	{EventTaskInformation, "task"},
	{EventOpcodeInformation, "opcode"},
	{EventInformationMax, "max"},
};

EtwFieldMap GenerateFieldMap(LPGUID providerGuid) {
	EtwFieldMap map;
	
	for (const auto& pair : typeMap) {
		std::vector<EtwFieldInfo> fields = GetFields(providerGuid, pair.first);
		map.insert(EtwFieldMapPair{ pair.second, std::move(fields) });	
	}

	return map;
}

std::vector<EtwProviderMetadata> GetEtwProviders() {
	
	std::vector<EtwProviderMetadata> providers;
	heap_ptr<PROVIDER_ENUMERATION_INFO> providerEnumInfo = EnumerateProviders();

	for (std::size_t i = 0; i < providerEnumInfo->NumberOfProviders; ++i) {
		PTRACE_PROVIDER_INFO provider = &providerEnumInfo->TraceProviderInfoArray[i];

		WCHAR providerGuid[40];
		StringFromGUID2(provider->ProviderGuid, providerGuid, sizeof(providerGuid)/sizeof(WCHAR));

		PWSTR providerName = GetOffsetValue<PWSTR>(providerEnumInfo.get(), provider->ProviderNameOffset);

		EtwProviderSchemaType schema = (provider->SchemaSource) ? 
			EtwProviderSchemaType::MOF : EtwProviderSchemaType::Manifest;
		
		EtwFieldMap map = GenerateFieldMap(&provider->ProviderGuid);

		EtwProviderMetadata providerMetadata(providerGuid, providerName, schema, std::move(map));
		
		providers.push_back(std::move(providerMetadata));
	}

	return providers;
}

}

