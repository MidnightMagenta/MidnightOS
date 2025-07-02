#include <k_utils/kstring.hpp>
#include <k_utils/memory.h>

uint64_t MdOS::string::strlen(const char *str) {
	uint64_t len = 0;
	while (str[len] != 0) { len++; }
	return len;
}

char toU64StrBuffer[128];
const char *MdOS::string::to_string(uint64_t num) {
	uint64_t sizeTest = num;
	uint8_t length = 0;
	while (sizeTest / 10 > 0) {
		sizeTest /= 10;
		length++;
	}

	uint8_t index = 0;
	do {
		uint8_t remainder = uint8_t(num % 10);
		num /= 10;
		toU64StrBuffer[length - index] = char(remainder) + '0';
		index++;
	} while (num / 10 > 0);
	uint8_t remainder = uint8_t(num % 10);
	toU64StrBuffer[length - index] = char(remainder) + '0';
	toU64StrBuffer[length + 1] = '\0';
	return toU64StrBuffer;
}

char toH64StrBuffer[128];
const char *MdOS::string::to_hstring(uint64_t num) {
	uint64_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 8 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toH64StrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toH64StrBuffer[size - uint64_t(i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toH64StrBuffer[size + 1] = '\0';
	return toH64StrBuffer;
}

char toH32StrBuffer[128];
const char *MdOS::string::to_hstring(uint32_t num) {
	uint32_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 4 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toH32StrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toH32StrBuffer[size - (i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toH32StrBuffer[size + 1] = '\0';
	return toH32StrBuffer;
}

char toH16StrBuffer[128];
const char *MdOS::string::to_hstring(uint16_t num) {
	uint16_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 2 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toH16StrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toH16StrBuffer[size - (i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toH16StrBuffer[size + 1] = '\0';
	return toH16StrBuffer;
}

char toH8StrBuffer[128];
const char *MdOS::string::to_hstring(uint8_t num) {
	uint8_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 1 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toH8StrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toH8StrBuffer[size - (i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toH8StrBuffer[size + 1] = '\0';
	return toH8StrBuffer;
}

char toI64StrBuffer[128];
const char *MdOS::string::to_string(int64_t num) {
	uint8_t negative = 0;
	if (num < 0) {
		negative = 1;
		num *= -1;
		toI64StrBuffer[0] = '-';
	}
	uint64_t sizeTest = uint64_t(num);
	uint8_t length = 0;
	while (sizeTest / 10 > 0) {
		sizeTest /= 10;
		length++;
	}

	uint8_t index = 0;
	while (num / 10 > 0) {
		uint8_t remainder = uint8_t(num % 10);
		num /= 10;
		toI64StrBuffer[negative + length - index] = char(remainder) + '0';
		index++;
	}
	uint8_t remainder = uint8_t(num % 10);
	toI64StrBuffer[negative + length - index] = char(remainder) + '0';
	toI64StrBuffer[negative + length + 1] = '\0';
	return toI64StrBuffer;
}

char toF64StrBuffer[128];
const char *MdOS::string::to_string(double num, unsigned int decimal_places) {
	if (decimal_places >= 20) { decimal_places = 20; }

	char *intPtr = (char *) to_string((int64_t) num);
	char *doublePtr = toF64StrBuffer;

	if (num < 0) { num *= -1; }
	while (*intPtr != 0) {
		*doublePtr = *intPtr;
		intPtr++;
		doublePtr++;
	}

	*doublePtr = '.';
	doublePtr++;

	double newNum = num - int(num);
	for (uint8_t i = 0; i < decimal_places; i++) {
		newNum *= 10;
		*doublePtr = char(int(newNum)) + '0';
		newNum -= int(newNum);
		doublePtr++;
	}
	*doublePtr = '\0';
	return toF64StrBuffer;
}

char toF32StrBuffer[128];
const char *MdOS::string::to_string(float num, unsigned int decimal_places) {
	if (decimal_places >= 20) { decimal_places = 20; }
	char *intPtr = (char *) to_string((int64_t) num);
	char *floatPtr = toF32StrBuffer;

	if (num < 0) { num *= -1; }
	while (*intPtr != 0) {
		*floatPtr = *intPtr;
		intPtr++;
		floatPtr++;
	}

	*floatPtr = '.';
	floatPtr++;

	float newNum = num - int(num);
	for (uint8_t i = 0; i < decimal_places; i++) {
		newNum *= 10;
		*floatPtr = char(int(newNum)) + '0';
		newNum -= int(newNum);
		floatPtr++;
	}
	*floatPtr = '\0';
	return toF32StrBuffer;
}