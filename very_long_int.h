#include <vector>
#include <string>

/** ROZLICZENIE:

 VeryLongInt x    - tworzy długą liczbę równą 0; (4)

 VeryLongInt x(y) - tworzy kopię długiej liczby y (ta instrukcja powinna być
                    również obsługiwać semantykę przenoszenia); (2), (3)

 VeryLongInt x(n) - tworzy długą liczbę na podstawie nieujemnej liczby
                    całkowitej n; (4)
 VeryLongInt x(s) - tworzy długą liczbę na podstawie jej dziesiętnego zapisu
                   w łańcuchu s; (5)

 ---

 x = y - przypisuje długą liczbę y na długą liczbę x (6) (ta instrukcja powinna
         również obsługiwać semantykę przenoszenia (7) );
 x = n - przypisuje nieujemną liczbę całkowitą n na długą liczbę x; (4), (6)

 ---

 x.numberOfBinaryDigits() - zwraca liczbę cyfr w zapisie dwójkowym x; (8)

 ---

 [OPERATORY ARYTMETYCZNO-BITOWE] - (10;26)

 ---

 W przypadku dzielenia przez 0 lub odejmowania większej liczby od mniejszej
 rozwiązanie powinno implementować nieliczbę (NaN) i zwracać ją w takich
 przypadkach. Jeśli jednym z argumentów jest nieliczba, wynik powinien być
 nieliczbą. Metoda isValid() powinna zwracać false wtedy i tylko wtedy, gdy
 długa liczba jest nieliczbą. (9)

 ---

 [OPERATORY RELACYJNE] - (30;35)

 ---

 Wszystkie operacje arytmetyczne i operatory porównania mają również działać
 z argumentami, które są nieujemnymi liczbami całkowitymi (4), ale nie powinny
 działać z napisami (błąd kompilacji). (5) (explicit)

 ---

 os << x - wypisuje długą liczbę w zapisie dziesiętnym na strumień os
          (NaN, jeśli x jest NaN). (40)

 ---

 Używanie długich liczb powinno być również możliwe w instrukcjach sterujących
 przepływem (takich jak if(x)). W takich przypadkach fałsz odpowiada NaN lub
 poprawnej długiej liczbie równej zeru. Oprócz tego przypadku, nie jest
 dopuszczalna konwersja obiektów VeryLongInt na inne typy. (41)

 ---

 Powinny również być dostępne globalne niemodyfikowalne obiekty (43) oraz
 odpowiadające im funkcje (42):

 Zero() - zwraca obiekt reprezentujący liczbę 0;
 NaN()  - zwraca obiekt reprezentujący nieliczbę.

 Funkcje Zero i NaN powinny zwracać obiekty, których nie można modyfikować
 (a próba modyfikowania powinna powodować błąd kompilacji). (zwracaja const object)

 */


typedef unsigned long long BaseType;

class VeryLongInt;

const VeryLongInt& Zero(); //(42)
const VeryLongInt& NaN();

class VeryLongInt
{
private:
    std::vector<BaseType> storage;
    bool isNaN;

    VeryLongInt& truncate();
    VeryLongInt performDivision( const VeryLongInt& dividend_in,
                                  const VeryLongInt& divisor_in,
                                  VeryLongInt* remainder_out);
    bool validateString(const char* str) const;
public:

    //konstruktor kopiujący/przenoszący
    VeryLongInt(const VeryLongInt& other) = default; //(2)
    VeryLongInt(VeryLongInt&& other) = default; //(3)

    VeryLongInt(bool) = delete;
    VeryLongInt(char) = delete;

    //konstruktor z liczbowego prymitywu
    VeryLongInt(BaseType number = 0); //(4)

    /**
     * "Jakie konstruktory mamy więc zaimplementować?"
     *
     * "Wszystkie potrzebne, aby spełnić wymagania postawione w treści zadania
     * oraz w wyjaśnieniu, które pojawiło się w wątku dotyczącym zadania 3,
     * opisującym wymagania odnośnie konstruktorów."
     *
     * P.S. Nikt dookoła nie wie jak to inaczej rozwiązać.
     * W zasadzie trzeba zdefiniowac wszystkie konstruktory jakie tylko istnieja
     * zeby spelnic warunki zadania.
     */
    VeryLongInt(unsigned short number) : VeryLongInt(static_cast<BaseType> (number)) {}
    VeryLongInt(short number) : VeryLongInt(static_cast<BaseType> (number)) {}
    VeryLongInt(unsigned int number) : VeryLongInt(static_cast<BaseType> (number)) {}
    VeryLongInt(int number) : VeryLongInt(static_cast<BaseType> (number)) {}
    VeryLongInt(unsigned long number) : VeryLongInt(static_cast<BaseType> (number)) {}
    VeryLongInt(long number) : VeryLongInt(static_cast<BaseType> (number)) {}
    //VeryLongInt(unsigned long long number) : VeryLongInt(static_cast<BaseType> (number)) {}
    VeryLongInt(long long number) : VeryLongInt(static_cast<BaseType> (number)) {}

    //konstruktor z napisu w postaci dziesiętnej
    //explicite, poniewaz nie chcemy, aby operatory dzialaly
    //dla napisow, por. w tresci zadania.
    explicit VeryLongInt(const std::string& str); //(5)
    explicit VeryLongInt(const char* str);

    //operatory przypisania, self assignment check!
    //przypisanie nieujemnej liczby odbywa sie przez
    //niejawna konwersje poprzez konstruktor z liczbowego prymitywu
    VeryLongInt& operator=(const VeryLongInt& rhs); //(6)
    VeryLongInt& operator=(VeryLongInt&& rhs); //(7)

    unsigned long long numberOfBinaryDigits() const; //(8)

    bool isValid() const; //(9)

    VeryLongInt& operator+=(const VeryLongInt& rhs); //(10)
    VeryLongInt& operator-=(const VeryLongInt& rhs); //(11)
    VeryLongInt& operator*=(const VeryLongInt& rhs); //(12)
    VeryLongInt& operator/=(const VeryLongInt& rhs); //(13)
    VeryLongInt& operator%=(const VeryLongInt& rhs); //(14)
    VeryLongInt& operator>>=(unsigned long long i); //(15)
    VeryLongInt& operator<<=(unsigned long long i); //(16)

    //VeryLongInt is so happy to have so many friends!

    friend std::ostream& operator<<(std::ostream& out, const VeryLongInt& obj); //(40)
    friend bool operator==(const VeryLongInt& lhs, const VeryLongInt& rhs); //(30)
    friend bool operator<=(const VeryLongInt& lhs,const VeryLongInt& rhs); //(32)

    explicit operator bool() const; //(41)

};

const VeryLongInt operator+(const VeryLongInt& lhs, const VeryLongInt& rhs); //(20)
const VeryLongInt operator-(const VeryLongInt& lhs, const VeryLongInt& rhs); //(21)
const VeryLongInt operator*(const VeryLongInt& lhs, const VeryLongInt& rhs); //(22)
const VeryLongInt operator/(const VeryLongInt& lhs, const VeryLongInt& rhs); //(23)
const VeryLongInt operator%(const VeryLongInt& lhs, const VeryLongInt& rhs); //(24)
const VeryLongInt operator>>(const VeryLongInt& lhs, unsigned long long i); //(25)
const VeryLongInt operator<<(const VeryLongInt& lhs, unsigned long long i); //(26)

bool operator!=(const VeryLongInt& lhs,const VeryLongInt& rhs); //(31)
bool operator>=(const VeryLongInt& lhs,const VeryLongInt& rhs); //(33)
bool operator<(const VeryLongInt& lhs,const VeryLongInt& rhs); //(34)
bool operator>(const VeryLongInt& lhs,const VeryLongInt& rhs); //(35)

