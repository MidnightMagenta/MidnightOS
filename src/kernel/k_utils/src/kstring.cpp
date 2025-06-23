#include "../include/kstring.hpp"

uint64_t MdOS::string::strlen(const char *str) {
	uint64_t len = 0;
	while (str[len] != 0) { len++; }
	return len;
}

char toStrBuffer[128];
const char *MdOS::string::to_string(uint64_t num) {
	uint64_t sizeTest = num;
	uint8_t length = 0;
	while (sizeTest / 10 > 0) {
		sizeTest /= 10;
		length++;
	}

	uint8_t index = 0;
	while (num / 10 > 0) {
		uint8_t remainder = uint8_t(num % 10);
		num /= 10;
		toStrBuffer[length - index] = char(remainder) + '0';
		index++;
	}
	uint8_t remainder = uint8_t(num % 10);
	toStrBuffer[length - index] = char(remainder) + '0';
	toStrBuffer[length + 1] = 0;
	return toStrBuffer;
}
const char *MdOS::string::to_hstring(uint64_t num) {
	uint64_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 8 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toStrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toStrBuffer[size - uint64_t(i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toStrBuffer[size + 1] = 0;
	return toStrBuffer;
}

const char *MdOS::string::to_hstring(uint32_t num) {
	uint32_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 4 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toStrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toStrBuffer[size - (i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toStrBuffer[size + 1] = 0;
	return toStrBuffer;
}

const char *MdOS::string::to_hstring(uint16_t num) {
	uint16_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 2 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toStrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toStrBuffer[size - (i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toStrBuffer[size + 1] = 0;
	return toStrBuffer;
}

const char *MdOS::string::to_hstring(uint8_t num) {
	uint8_t *valPtr = &num;
	uint8_t *ptr;
	uint8_t temp;
	constexpr uint64_t size = 1 * 2 - 1;
	for (uint8_t i = 0; i < size; i++) {
		ptr = ((uint8_t *) valPtr + i);
		temp = ((*ptr & 0xF0) >> 4);
		toStrBuffer[size - uint64_t(i * 2 + 1)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
		temp = ((*ptr & 0x0F));
		toStrBuffer[size - (i * 2)] = char(temp + (temp > 9 ? 'A' - 10 : '0'));
	}
	toStrBuffer[size + 1] = 0;
	return toStrBuffer;
}


const char *MdOS::string::to_string(int64_t num) {
	uint8_t negative = 0;
	if (num < 0) {
		negative = 1;
		num *= -1;
		toStrBuffer[0] = '-';
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
		toStrBuffer[negative + length - index] = char(remainder) + '0';
		index++;
	}
	uint8_t remainder = uint8_t(num % 10);
	toStrBuffer[negative + length - index] = char(remainder) + '0';
	toStrBuffer[negative + length + 1] = 0;
	return toStrBuffer;
}

const char *MdOS::string::to_string(double num, unsigned int decimal_places) {
	if (decimal_places >= 20) { decimal_places = 20; }

	char *intPtr = (char *) to_string((int64_t) num);
	char *doublePtr = toStrBuffer;

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
	*doublePtr = 0;
	return toStrBuffer;
}
const char *MdOS::string::to_string(float num, unsigned int decimal_places) {
	if (decimal_places >= 20) { decimal_places = 20; }
	char *intPtr = (char *) to_string((int64_t) num);
	char *floatPtr = toStrBuffer;

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
	*floatPtr = 0;
	return toStrBuffer;
}