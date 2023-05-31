#include "Vector3f.h"

#include <limits>

const Vector3f Vector3f::Zero(0, 0, 0);
const Vector3f Vector3f::One(1, 1, 1);
const Vector3f Vector3f::Half(0.5f, 0.5f, 0.5f);
const Vector3f Vector3f::MinusOne(-1, -1, -1);
const Vector3f Vector3f::PlusInf(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
const Vector3f Vector3f::MinusInf(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
