#ifndef KNIGHTS_H
#define KNIGHTS_H

#include <cstddef>
#include <list>
#include <limits>
#include <ostream>
#include <compare>

using WeaponArmourClass = size_t;
using Gold = size_t;
using NumberOfKnights = size_t;

class Knight {
    private:
        Gold gold;
        WeaponArmourClass weapon;
        WeaponArmourClass armour;
        
        constexpr Gold max_gold_that_can_be_taken() const {
            return MAX_GOLD - gold;
        }

    public:
        static constexpr size_t MAX_GOLD = std::numeric_limits<Gold>::max(); 

        constexpr Knight(Gold gold, WeaponArmourClass weaponClass, WeaponArmourClass armourClass): gold(gold), weapon(weaponClass), armour(armourClass) {}

        Knight(const Knight& other) = default;
        Knight(Knight&& other) = default;

        Knight& operator=(const Knight&) = default;
        Knight& operator=(Knight&&) = default;

        constexpr Gold get_gold() const {
            return gold;
        }

        constexpr WeaponArmourClass get_weapon_class() const {
            return weapon;
        }

        constexpr WeaponArmourClass get_armour_class() const {
            return armour;
        }

        constexpr void take_gold(Gold gold_to_be_taken) {
            if (max_gold_that_can_be_taken() <= gold_to_be_taken) {
                gold = MAX_GOLD;
            } else {
                gold += gold_to_be_taken;
            }
        }

        constexpr Gold give_gold() {
            Gold gold_knight_had = gold;
            gold = 0;
            return gold_knight_had;
        }

        constexpr void change_weapon(WeaponArmourClass new_weapon) { 
            weapon = new_weapon;
        }

        constexpr void change_armour(WeaponArmourClass new_armour) {
            armour = new_armour;
        }

        constexpr WeaponArmourClass give_up_weapon() {
            WeaponArmourClass weapon_knight_had = weapon;
            weapon = 0;
            return weapon_knight_had;
        }

        constexpr WeaponArmourClass take_off_armour() {
            WeaponArmourClass armour_knight_had = armour;
            armour = 0;
            return armour_knight_had;
        }

        constexpr std::weak_ordering operator<=>(const Knight &other) const {
            WeaponArmourClass other_weapon = other.get_weapon_class();
            WeaponArmourClass other_armour = other.get_armour_class();
            if (weapon > other_armour) {
                if (armour >= other_weapon) {
                    return std::weak_ordering::greater;
                } else if (armour > other_armour) {
                    return std::weak_ordering::greater;
                } else if (armour < other_armour) {
                    return std::weak_ordering::less;
                }
                return weapon <=> other_weapon;
            }

            if (other_weapon > armour) {
                return std::strong_ordering::less;
            }

            return std::strong_ordering::equal;
        }

        constexpr bool operator==(const Knight &other) const {
            auto cmp = *this <=> other;
            if (cmp == 0) {
                return true;
            }
            return false;
        }

        Knight& operator+=(Knight &other) {
            if (this == &other) {
                return *this;
            }
            this->take_gold(other.give_gold());
            WeaponArmourClass other_weapon = other.get_weapon_class();
            WeaponArmourClass other_armour = other.get_armour_class();
            
            if (weapon < other_weapon) {
                weapon = other.give_up_weapon();
            }
            if (armour < other_armour) {
                armour = other.take_off_armour();
            }
            return *this;
        }

        constexpr Knight operator+(const Knight &other) const {
            Gold new_gold;
            Gold other_gold = other.get_gold();
            if (max_gold_that_can_be_taken() <= other_gold) {
                new_gold = MAX_GOLD;
            } else { 
                new_gold = gold + other_gold;
            }

            WeaponArmourClass new_weapon = (weapon > other.get_weapon_class()) ? weapon : other.get_weapon_class();
            WeaponArmourClass new_armour = (armour > other.get_armour_class()) ? armour : other.get_armour_class();

            return Knight {new_gold, new_weapon, new_armour};
        }

};

inline std::ostream &operator<<(std::ostream &os, Knight const &knight) {
    return os << "(" << knight.get_gold() << " gold, " << knight.get_weapon_class() << " weapon class, " << knight.get_armour_class() << " armour class)\n";
}

constexpr Knight TRAINEE_KNIGHT {0, 1, 1};

class Tournament {
    using KnightList = std::list<Knight>;
    private:
        KnightList contenders;
        KnightList eliminated;
        NumberOfKnights contestants_number;

        void empty_eliminated() {
            contestants_number -= eliminated.size();
            eliminated.clear();
        }
    public:
        Tournament(const Tournament& other) = default;
        Tournament(Tournament&& other) = default;
        Tournament& operator=(const Tournament& other) = default;
        Tournament& operator=(Tournament&& other) = default;
        
        Tournament(KnightList contenders): contenders(contenders) {
            eliminated = KnightList();
            if (contenders.empty()) {
                this -> contenders.push_back(TRAINEE_KNIGHT);
            }
            contestants_number = this -> contenders.size();
        }
        
        Tournament& operator+=(const Knight& other) {
            empty_eliminated();
            contenders.push_back(other);
            contestants_number++;
            return *this;
        }

        Tournament& operator-=(const Knight& other) {
            Gold gold = other.get_gold();
            WeaponArmourClass weapon = other.get_weapon_class();
            WeaponArmourClass armour = other.get_armour_class();
            empty_eliminated();

            for (auto it = contenders.begin(); it != contenders.end();) {
                Knight knight = *it;
                if (knight.get_weapon_class() == weapon && knight.get_armour_class() == armour && knight.get_gold() == gold) {
                    it = contenders.erase(it);
                    contestants_number--;
                } else {
                    it++;
                }
            }
            return *this;
        }

        KnightList::const_iterator no_winner() const {
            return contenders.cend();
        }

        constexpr size_t size() const {
            return contestants_number;
        }

        KnightList::iterator play() {
            empty_eliminated();
            if (contenders.size() == 1) {
                return contenders.begin();
            }

            auto it = contenders.begin(); 
            while (contenders.size() > 1) {
                auto it2 = it;
                it2++;
                if (it2 == contenders.end())
                    break;
                if (*it > *it2) {
                   *it += *it2;
                    eliminated.push_front(*it2);
                    contenders.push_back(*it);
                } else if (*it < *it2) {
                    *it2 += *it;
                    eliminated.push_front(*it);
                    contenders.push_back(*it2);
                } else {
                    eliminated.push_front(*it2);
                    eliminated.push_front(*it);
                }

                contenders.erase(it);
                it = contenders.erase(it2);
            }
            return contenders.begin();
        }

        friend std::ostream &operator<<(std::ostream &os, Tournament const &tournament);
};

std::ostream &operator<<(std::ostream &os, Tournament const &tournament) {
    for (const auto &knight : tournament.contenders) {
        os << "+ " << knight;
    }
    
    for (const auto &knight : tournament.eliminated) {
        os << "- " << knight;
    }
    
    return os << "=\n";
}


static constexpr size_t unsigned_abs(size_t a, size_t b) {
    if (a > b) {
        return a - b;
    }

    return b - a;
}

inline consteval std::pair<WeaponArmourClass, WeaponArmourClass> max_diff_classes(const std::initializer_list<Knight>& knights) {
    auto it = knights.begin();

    std::pair<WeaponArmourClass, WeaponArmourClass> max_diff_pair 
    = std::make_pair(it->get_weapon_class(), it->get_armour_class());

    WeaponArmourClass max_diff = unsigned_abs(it->get_weapon_class(), it->get_armour_class());

    while (it != knights.end()) {
        Knight knight = *it;
        WeaponArmourClass weapon = knight.get_weapon_class();
        WeaponArmourClass armour = knight.get_armour_class();
        WeaponArmourClass diff = unsigned_abs(weapon, armour);
        if (diff > max_diff) {
            max_diff = diff;
            max_diff_pair = std::make_pair(weapon, armour);
        }
        it++;
    }
    return max_diff_pair;
}

#endif
