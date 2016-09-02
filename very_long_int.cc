#include <limits>
#include <assert.h>
#include <ostream>
#include <string.h>
#include "very_long_int.h"


VeryLongInt::VeryLongInt(BaseType number)
{
    storage.push_back(number);
    isNaN = false;
}

VeryLongInt::VeryLongInt(const std::string& str) : VeryLongInt(str.c_str())
{
}

VeryLongInt::VeryLongInt(const char* str)
{
    if (!validateString(str))
        isNaN = true;
    else
    {
        isNaN = false;
        storage.push_back(static_cast<BaseType> (0));
        for (unsigned i = 0; str[i] != 0; i++)
        {
            operator*=(10);
            operator+=(static_cast<BaseType> (str[i] - '0'));
        }
    }
}

VeryLongInt& VeryLongInt::operator=(const VeryLongInt& rhs)
{
    //self-assingment check!
    if (this != &rhs)
    {
        storage = rhs.storage;
        isNaN = rhs.isNaN;
    }
    return *this;
}


VeryLongInt& VeryLongInt::operator=(VeryLongInt&& rhs)
{

    storage = std::move(rhs.storage);
    isNaN = rhs.isNaN;
    return *this;
}

unsigned long long VeryLongInt::numberOfBinaryDigits() const
{
    if (isNaN)
        return 0;
    if (operator==(*this, 0))
        return 1;
    else
    {
        auto noOfBits = std::numeric_limits<BaseType>::digits;
        auto ret = (storage.size() - 1) * noOfBits;
        BaseType last = storage.back();
        while (last != 0)
        {
            ret++;
            last >>= 1;
        }
        return ret;
    }
}

//Usuwa wiodące cyfry zero ze storage (jeśli jakieś występują)
VeryLongInt& VeryLongInt::truncate()
{
    while (storage.size() > 1 && storage.back() == 0)
        storage.pop_back();
    return *this;
}

VeryLongInt& VeryLongInt::operator+=(const VeryLongInt& other)
{
    if (isNaN || other.isNaN)
        return (operator=(NaN()));
    // a - cyfra z tej liczby, b - cyfra z drugiej liczby
    // r - cyfra będąca rezultatem, carry - cyfra do przeniesienia
    BaseType a, b, r, carry = 0;

    const std::size_t maxSize = std::max(storage.size(), other.storage.size());
    storage.resize(maxSize + 1, 0);

    for (std::size_t i = 0; i < maxSize + 1; i++)
    {
        a = storage[i];
        b = (i < other.storage.size()) ? other.storage[i] : 0;
        //Algorytm ten korzysta z arytmetyki modularnej typów unsigned, zatem jeśli
        //a + b + carry przekraczają rozmiar BaseType, nadal otrzymamy poprawny wynik
        r = a + b + carry;
        //Jeśli cyfra będąca rezultatem jest mniejsza niż którakolwiek z wejściowych
        //oznacza to, że powinno nastąpić przeniesienie.
        if (carry == 0)
            carry = (r < a || r < b) ? 1 : 0;
        else
            carry = (r <= a || r <= b) ? 1 : 0;
        storage[i] = r;
    }

    truncate();
    return *this;
}

VeryLongInt& VeryLongInt::operator-=(const VeryLongInt& other)
{
    if (isNaN || other.isNaN || operator<(*this, other))
    {
        return (operator=(NaN()));
    }
    // a - cyfra z tej liczby, b - cyfra z drugiej liczby
    // r - cyfra będąca rezultatem, borrow - cyfra do pożyczenia
    BaseType a, b, r, borrow = 0;

    const std::size_t maxSize = storage.size();
    assert(storage.size() >= other.storage.size());

    for (std::size_t i = 0; i < maxSize; i++)
    {
        a = storage[i];
        b = (i < other.storage.size()) ? other.storage[i] : 0;
        //Algorytm ten korzysta z arytmetyki modularnej typów unsigned
        //zatem jeśli a < b + borrow, to nadal dostajemy poprawny wynik
        r = a - b - borrow;
        //Jeśli cyfra od której odejmujemy jest większa niż odejmowana
        //to powinno zajść pożyczenie
        if (borrow == 0)
            borrow = (a < b) ? 1 : 0;
        else
            borrow = (a <= b) ? 1 : 0;
        storage[i] = r;
    }
    assert(borrow == 0);

    truncate();
    return *this;
}

VeryLongInt& VeryLongInt::operator*=(const VeryLongInt& other)
{
    if (isNaN || other.isNaN)
        return (operator=(NaN()));

    //Z powodu sposobu działania algorytmu, jeśli *this i other to ten sam obiekt
    //konieczne jest wykonanie kopii other
    const VeryLongInt& other_non_arg = (this == &other) ? (VeryLongInt(other)) : (other);
    VeryLongInt thisCopy = *this;
    operator=(0);
    auto noOfBits = std::numeric_limits<BaseType>::digits;
    //Aby nie wykonywać niepotrzebnie pojedyńczych przesunięć na liczbach VeryLongInt
    //zapamiętujemy liczbę przesunięć i wykonujemy je tylko, gdy potrzeba
    int toShift = 0;

    for (std::size_t i = 0; i < other_non_arg.storage.size(); i++)
    {
        //Obecnie przetwarzana cyfra drugiej liczby
        BaseType b = other_non_arg.storage[i];
        for (auto j = 0; j < noOfBits; j++)
        {
            //Sprawdzamy kolejne bity. Jeśli bit jest równy 1,
            //należy przesunąć kopię oryginalnej liczby i dodać ją
            //do wyniku
            if ((b & 1) == 1)
            {
                thisCopy <<= toShift;
                toShift = 0;
                operator+=(thisCopy);
            }
            b >>= 1;
            toShift++;
        }
    }
    truncate();
    return *this;
}

//Funkcja wykonująca dzielenie, z której korzystają operatory /= i %=
//Przyjmuje przez odniesienie dzielną i dzielnik oraz wskaźnik do zapisania reszty
//(wskaźnik ten może być NULL, jeśli reszta z dzielenia nie jest potrzebna)
VeryLongInt VeryLongInt::performDivision(const VeryLongInt& dividend_in,
                                         const VeryLongInt& divisor_in,
                                         VeryLongInt* remainder_out)
{
    auto noOfBits = std::numeric_limits<BaseType>::digits;
    VeryLongInt quotient = 0;
    if (dividend_in.isNaN || divisor_in.isNaN || divisor_in == 0)
    {
        if (remainder_out != nullptr)
            *remainder_out = NaN();
        return (quotient = NaN());
    }

    if (divisor_in == 2)
    {
        if (remainder_out != nullptr)
            *remainder_out = (dividend_in.storage[0] & (BaseType)1);
        return (dividend_in >> 1);
    }

    //Dla uproszczenia osobno traktujemy przypadek, gdy dzielnik jest większy niż dzielna
    if (divisor_in > dividend_in)
    {
        if (remainder_out != nullptr)
            *remainder_out = dividend_in;
        return VeryLongInt();
    }

    assert(dividend_in >= divisor_in);

    VeryLongInt dividend = dividend_in;
    VeryLongInt divisor = divisor_in;

    unsigned shift = 0;
    
    //Jeśli dzielnik jest znacząco większy od dzielnej, wykonujemy duże przesunięcie
    //zamiast powtarzania pojedyńczych
    if (divisor.storage.size() < dividend.storage.size() - 1){
        shift = (dividend.storage.size() - divisor.storage.size() - 1) * noOfBits;
        divisor <<= shift;
    }

    //Algorytm wykonuje binarne dzielenie pisemne liczb
    //Przesuwamy dzielnik w lewo tak daleko, aż będzie 
    //minimalnie mniejszy od dzielnej
    while (divisor < dividend)
    {
        divisor <<= 1;
        shift++;
    }

    if (divisor > dividend)
    {
        divisor >>= 1;
        shift--;
    }

    //Dla poprawy prędkości odwlekamy przesuwanie ilorazu tak długo, jak to możliwe
    unsigned quotientShift = 0;
    for (unsigned i = 0; i <= shift; i++)
    {
        quotientShift++;
        //Jeśli przesunięty dzielnik jest mniejszy lub równy dzielnej,
        //należy go odjąć, a bitem ilorazu jest 1. Jeśli nie jest, bitem ilorazu jest 0.
        //Bity 0 generowane są przez samo przesunięcie, bity 1 przez dodatkową operację bitowego OR
        if (divisor <= dividend)
        {
            dividend -= divisor;
            quotient <<= quotientShift;
            quotientShift = 0;
            quotient.storage.front() |= 1;
        }
        //W obu przypadkach przesuwamy dzielnik o jeden bit w lewo
        divisor >>= 1;
    }
    //wykonujemy "zaległe" przesunięcia
    quotient <<= quotientShift;

    if (remainder_out != nullptr)
    {
        dividend.truncate();
        *remainder_out = dividend;
    }

    quotient.truncate();
    return quotient;
}

VeryLongInt& VeryLongInt::operator/=(const VeryLongInt& other)
{
    return (operator=(performDivision(*this, other, nullptr)));
}

VeryLongInt& VeryLongInt::operator%=(const VeryLongInt& other)
{
    performDivision(*this, other, this);
    return *this;
}

VeryLongInt& VeryLongInt::operator>>=(unsigned long long i)
{
    if (isNaN || i == 0 || operator==(*this, 0))
        return *this;
    
    auto noOfBits = std::numeric_limits<BaseType>::digits;
    //O ile cyfr przechowywanych w storage należy przesunąć bity
    auto shiftMajor = i / noOfBits;
    if (shiftMajor >= storage.size())
        return (operator=(0));
    //O ile bitów wewnątrz cyfry należy przesunąć bity
    auto shiftMinor = i % noOfBits;
    if (shiftMajor > 0)
    {
        for (std::size_t i = storage.size(); i > shiftMajor; i--)
        {
            storage[i - shiftMajor - 1] = storage[i - 1];
            storage[i - 1] = 0;
        }
    }

    if (shiftMinor > 0)
    {
        //Przeniesienia bitów z poprzedniej cyfry do następnej
        BaseType carryNew, carryOld = 0;
        for (std::size_t i = storage.size(); i > 0; i--)
        {
            carryNew = storage[i - 1] << (noOfBits - shiftMinor);
            storage[i - 1] = (storage[i - 1] >> shiftMinor) | carryOld;
            carryOld = carryNew;
        }
        
    }
    truncate();
    return *this;
}

VeryLongInt& VeryLongInt::operator<<=(unsigned long long i)
{
    if (isNaN || i == 0 || operator==(*this, 0))
        return *this;
    
    auto noOfBits = std::numeric_limits<BaseType>::digits;
    //O ile cyfr przechowywanych w storage należy przesunąć bity
    auto shiftMajor = i / noOfBits;
    //O ile bitów wewnątrz cyfry należy przesunąć bity
    auto shiftMinor = i % noOfBits;
    storage.resize(storage.size() + shiftMajor + 1, 0);
    if (shiftMajor > 0)
    {
        for (std::size_t i = storage.size(); i > shiftMajor; i--)
            storage[i - 1] = storage[i - shiftMajor - 1];
        for (std::size_t i = shiftMajor; i > 0; i--)
            storage[i - 1] = 0;
    }
    if (shiftMinor > 0)
    {
        //Przeniesienia bitów z poprzedniej cyfry do następnej
        BaseType carryNew, carryOld = 0;
        for (std::size_t i = 0; i < storage.size(); i++)
        {
            carryNew = storage[i] >> (noOfBits - shiftMinor);
            storage[i] = (storage[i] << shiftMinor) | carryOld;
            carryOld = carryNew;
        }
    }
    truncate();
    return *this;
}

const VeryLongInt operator+(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    return VeryLongInt(lhs) += rhs;
}

const VeryLongInt operator-(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    return VeryLongInt(lhs) -= rhs;
}

const VeryLongInt operator*(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    return VeryLongInt(lhs) *= rhs;
}

const VeryLongInt operator/(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    return VeryLongInt(lhs) /= rhs;
}

const VeryLongInt operator%(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    return VeryLongInt(lhs) %= rhs;
}

const VeryLongInt operator>>(const VeryLongInt& lhs, unsigned long long i)
{
    return VeryLongInt(lhs) >>= i;
}

const VeryLongInt operator<<(const VeryLongInt& lhs, unsigned long long i)
{
    return VeryLongInt(lhs) <<= i;
}

bool operator==(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    if (!lhs.isValid() || !rhs.isValid())
        return false;
    if (lhs.storage.size() == rhs.storage.size())
    {
        for (size_t i = 0; i < lhs.storage.size(); i++)
            if (lhs.storage[i] != rhs.storage[i])
                return false;
        return true;
    }
    return false;
}

bool operator!=(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    if (!lhs.isValid() || !rhs.isValid())
        return false;
    return !(lhs == rhs);
}

bool operator<=(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    if (!lhs.isValid() || !rhs.isValid())
        return false;

    if (lhs.storage.size() == rhs.storage.size())
    {
        for (std::size_t i = lhs.storage.size(); i > 0; i--)
        {
            if (lhs.storage[i - 1] != rhs.storage[i - 1])
                return (lhs.storage[i - 1] < rhs.storage[i - 1]);
        }
        return true; //Liczby są równe
    }
    else
        return (lhs.storage.size() < rhs.storage.size());
}

bool operator>=(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    if (!lhs.isValid() || !rhs.isValid())
        return false;
    return !(lhs <= rhs) || lhs == rhs;
}

bool operator<(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    if (!lhs.isValid() || !rhs.isValid())
        return false;
    return lhs <= rhs && lhs != rhs;
}

bool operator>(const VeryLongInt& lhs, const VeryLongInt& rhs)
{
    if (!lhs.isValid() || !rhs.isValid())
        return false;
    return !(lhs <= rhs);
}


std::ostream& operator<<(std::ostream& out, const VeryLongInt& obj)
{
    if (obj.isNaN)
    {
        out << "NaN";
        return out;
    }

    //Dla uproszczenia zero traktujemy osobno
    if (obj == 0)
    {
        out << "0";
        return out;
    }
    /*
        Kod wykorzystuje algorytm double dabble do konwersji
        ciągu binarnego na BCD (binary coded decimal)
        http://en.wikipedia.org/wiki/Double_dabble
    */
    auto noOfBits = std::numeric_limits<BaseType>::digits;
    auto decLength = (obj.storage.size() * noOfBits + 2) / 3;
    char* dec = new char[decLength + 1]; //Tablica przechowująca cyfry dziesiętne
    for (unsigned i = 0; i < decLength + 1; i++)
        dec[i] = 0;
    unsigned firstDigit = decLength - 2;

    for (int i = obj.numberOfBinaryDigits() - 1; i >= 0; i--)
    {
        //Wczytaj nowy bit z liczby wejściowej. Rzutowanie 1 na BaseType jest bardzo ważne
        unsigned newBit = (obj.storage[i / noOfBits] & ((BaseType) 1 << (i % noOfBits))) ? 1 : 0;
        //Dodaj 3 do każdej cyfry większej lub równej niż 5
        for (unsigned k = firstDigit; k < decLength; k++)
            if (dec[k] >= 5)
                dec[k] += 3;

        //Zwiększ liczbę cyfr, jeśli potrzeba
        if (dec[firstDigit] >= 8)
            firstDigit--;

        //Cyfry poza ostatnią
        for (unsigned k = firstDigit; k < decLength - 1; k++)
        {
            dec[k] <<= 1; //Przesuń o 1 bit do przodu
            dec[k] &= 0xF; //Usuń nadmiarowe bity
            dec[k] |= (dec[k + 1] >= 8); //Dodaj na koniec ostatni bit następnej cyfry
        }
        //Ostatnia cyfra
        dec[decLength - 1] <<= 1; //Przesuń o 1 bit do przodu
        dec[decLength - 1] &= 0xF; //Usuń nadmiarowe bity
        dec[decLength - 1] |= newBit; //Dodaj na koniec bit wczytany z liczby wejściowej
    }

    //Przekonwertuj z BCD na ASCII
    for (unsigned i = 0; i < decLength; i++)
        dec[i] += '0';

    //Usuń wiodące zera
    char* dec_trim = dec;
    while (*dec_trim == '0')
    {
        dec_trim++;
    }
    //Nie tworzymy obiektu tymczasowego, ponieważ out jest przekazany przez referencję
    //i zwracana jest referencja do obiektu out
    out << dec_trim;
    delete[] dec;

    return out;
}


VeryLongInt::operator bool() const
{
    if (isNaN)
        return false;
    else
        return operator!=(*this, 0);
}


bool VeryLongInt::isValid() const
{
    return !isNaN;
}

const VeryLongInt& Zero()
{
    static const VeryLongInt zero;
    return zero;
}

const VeryLongInt& NaN()
{
    static const VeryLongInt nan("");
    return nan;
}

bool VeryLongInt::validateString(const char* str) const
{
    if(str == nullptr)
        return false;
    size_t len = strlen(str);
    if (len == 0)
            return false;

    for (size_t i = 0; str[i] != 0; i++)
        if (!isdigit(str[i]))
        {
            return false;
        }
    return true;
}
