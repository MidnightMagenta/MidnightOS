#ifndef RESULT_H
#define RESULT_H

namespace MdOS {
enum class Result {
	SUCCESS,
    NO_WORK,
	ALREADY_INITIALIZED,
    INIT_FAILURE,
    NOT_INITIALIZED,
    INVALID_PARAMETER,
    OUT_OF_MEMORY
};
}// namespace MdOS

#endif