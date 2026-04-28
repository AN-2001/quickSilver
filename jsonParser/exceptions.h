#pragma once
#include <exception>
#include <stdexcept>

namespace GraphToys {
    class JsonException : public std::runtime_error {
        public:
            using std::runtime_error::runtime_error;
    };

    class JsonKeyError : public JsonException {
        public:
            using JsonException::JsonException;
    };

    class JsonIndexError : public JsonException {
        public:
            using JsonException::JsonException;
    };

    class JsonParseError : public JsonException {
        public:
            using JsonException::JsonException;
    };
};
