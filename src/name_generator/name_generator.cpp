/*
* AIM name_generator
* Copyright (C) 2015 lzwdgc
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <random>
#include <string>

class NameGenerator
{
    static const int letmin = 2;
    static const int letmax = 3;

public:
    NameGenerator()
        : rng(std::random_device()())
    {
    }

    std::string generate(int sylmin = 2, int sylmax = 3)
    {
        static const std::string consonants = "bcdfghjklmnpqrstvwxz";
        static const std::string vowels = "aeiouy";
        using dist = std::uniform_int_distribution<>;
        dist c_distr(0, consonants.size() - 1);
        dist v_distr(0, vowels.size() - 1);
        dist cv_distr(0, 1);
        dist let_distr(letmin, letmax);
        auto n_syllables = dist(sylmin, sylmax)(rng);
        std::string name;
        for (int s = 0; s < n_syllables; s++)
        {
            auto n_letters = let_distr(rng);
            int n_consonants = 0, n_vowels = 0;
            std::string syllable;
            for (int l = 0; l < n_letters; l++)
            {
                auto vowel = cv_distr(rng);
                syllable += vowel ? vowels[v_distr(rng)] : consonants[c_distr(rng)];
                n_consonants += !vowel;
                n_vowels += vowel;
            }
            if (n_consonants == n_letters)
            {
                auto pos = dist(0, n_letters - 1)(rng);
                syllable[pos] = vowels[v_distr(rng)];
            }
            else if (n_vowels == n_letters)
            {
                auto pos = dist(0, n_letters - 1)(rng);
                syllable[pos] = consonants[c_distr(rng)];
            }
            name += syllable;
        }
        return check_name(name) ? name : generate(sylmin, sylmax);
    }

private:
    std::mt19937 rng;

    bool check_name(const std::string &name) const
    {
        auto banned = { "hu", "piz", "eb", "bl", "mu" };
        for (auto &b : banned)
            if (name.find(b) != std::string::npos)
                return false;
        return true;
    }
};

int main(int argc, char *argv[])
{
    int n = argc == 1 ? 1 : std::stoi(argv[1]);
    NameGenerator ng;
    for (int i = 0; i < n; i++)
        std::cout << ng.generate() << (i != n-1 ? "\n" : "");
    return 0;
}
