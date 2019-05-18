#include "tdhcpp.h"
#include <iostream>

int main(int argc, const char* argv) {

	std::cout << "[+] Collecting Information.." << std::endl;

	std::vector<tdhcpp::EtwProviderMetadata> metadata = tdhcpp::GetEtwProviders();

	for (const tdhcpp::EtwProviderMetadata& provider : metadata) {
		std::wcout << provider << std::endl;
	}
}


