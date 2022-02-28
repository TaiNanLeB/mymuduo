#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    Timestamp(long int microSecondsSinceEpoch_);
    static Timestamp now();
    std::string toString() const;
private:
    long int microSecondsSinceEpoch_;

};